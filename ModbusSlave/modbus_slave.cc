/*
	Simple Modbus/RTU slave (responder)

	Copyright (C) 2021, Fabmicro, LLC., Tyumen, Russia.

*/


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <string>

#include "modbus_common.h"

#define	REG_SYS_COUNTER		30		// System counter

#define	MSG_BUF_SIZE		1024	


int baudrate = 115200;
int modbus_address = 1;
char serial_device[128];
int fd = -1;
uint32_t reg_sys_counter = 0;

MODBUS_REQUEST request;
timeval request_last_rx_time;
 
unsigned short modbus_crc16(void *in_buf, int len)
{
	uint16_t crc = 0xFFFF;
	unsigned char *buf = (unsigned char*) in_buf;

	for (int pos = 0; pos < len; pos++) {
		crc ^= (uint16_t)buf[pos];    // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;		    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			} else			    // Else LSB is not set
				crc >>= 1;		    // Just shift right
		}
	}
	return crc;
}


void modbus_add_crc(void *in_buf, int len)
{
	unsigned char *buf = (unsigned char*) in_buf;
	uint16_t crc = modbus_crc16(buf, len);
	buf[len] = (crc & 0xff);
	buf[len+1] = (crc >> 8) & 0xff;
}


uint32_t modbus_error(uint8_t f, uint8_t err_code, void *txbuf)
{
	fprintf(stderr, "Request error, func: %d, err: %d\r\n", f, err_code);

	uint8_t* _txbuf = (uint8_t*) txbuf;
	_txbuf[0] = modbus_address;
	_txbuf[1] = f | 0x80;
	_txbuf[2] = err_code;
	modbus_add_crc(_txbuf, 3);
	return 3+2;
}


int modbus_transmit(MODBUS_REQUEST *req)
{
	int written;


	// Add CRC to TX buffer

	modbus_add_crc((unsigned char*)req->txbuf, req->txlen);
	req->txlen += 2;

	// Write response from TX buffer

	fprintf(stderr, "Modbus TX: sending %d bytes\n", req->txlen);

	if((written = write(fd, req->txbuf, req->txlen)) != req->txlen) {
		fprintf(stderr, "Modbus TX I/O error, attempted to write: %d, written: %d, error: %s\n", req->txlen, written, strerror(errno));
		fflush(stderr); 
		return -3;
	}


	memset(&request, 0, sizeof(request));
	request.timeout = MODBUS_RX_TIMEOUT * 1000;

	return 0;

}

void memcpy_rev(void *dst, void *src, uint32_t count) {
	uint8_t* s = (uint8_t*) src;
	uint8_t* d = (uint8_t*) dst;
	while(count) {
		*d++ = *s--;
		count--;
	}
}


int baudrate_to_speed(int baud) {

	switch(baud) {
		case 300: return B300;
		case 1200: return B1200;
		case 2400: return B2400;
		case 9600: return B9600;
		case 19200: return B19200;
		case 38400: return B38400;
		case 57600: return B57600;
		case 115200: return B115200;
		case 230400: return B230400;
	}

	return 0;
}

int init_serial_device(int speed)
{

	struct termios tty;

	if (tcgetattr (fd, &tty) != 0) {
		fprintf(stderr, "tcgetattr failure: %s\n\n", strerror(errno));
		return -1;
	}

	cfsetospeed (&tty, baudrate_to_speed(speed));
	cfsetispeed (&tty, baudrate_to_speed(speed));

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;	 // disable break processing
	tty.c_lflag = 0;		// no signaling chars, no echo,
					// no canonical processing
	tty.c_oflag = 0;		// no remapping, no delays
	tty.c_cc[VMIN]  = 1;	    // read blocks
	tty.c_cc[VTIME] = 2;	    // 0.2 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
					// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= 0;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		fprintf(stderr, "tcsetattr failure: %s\n\n", strerror(errno));
		return -2;
	}

	return 0;
}



int main(int argc, char *argv[])
{
	fd_set rd_set;
	struct timeval timeout;


	fprintf(stderr, "%s - a simple Modbus/RTU responder. Copyright (C) 2021, Fabmicro, LLC., Tyumen, Russia.\n\n", argv[0]);
	fflush(stdout);


	request.timeout = MODBUS_RX_TIMEOUT * 1000;

	while(1) {
		int fd_num;

		timeout.tv_sec = 0;
		timeout.tv_usec = request.timeout;

		FD_ZERO (&rd_set);
		//FD_ZERO (&err_set);

		FD_SET (fd, &rd_set);
		//FD_SET (fd, &err_set);

		reg_sys_counter ++;

		if((fd_num = select(FD_SETSIZE, &rd_set, NULL, NULL, &timeout)) < 0) {

			if(errno == EAGAIN)
				continue;

			perror("select");
			break;
		}


		if(fd_num && FD_ISSET(fd, &rd_set)) {

			int fd_len;

			if((fd_len = read(fd, request.rxbuf + request.rxlen, MODBUS_RX_BUF_SIZE - request.rxlen)) <= 0) {
				perror("read serial");
				break;
			}

			request.rxlen += fd_len;

			gettimeofday(&request_last_rx_time, NULL);

			fprintf(stderr, "Modbus RX: bytes RX = %d, rxbuf_len = %d\n", fd_len, request.rxlen);
		}


		if(fd_num == 0 || !FD_ISSET(0, &rd_set)) {

			timeval cur_time;
			gettimeofday(&cur_time, NULL);


			if(request.rxlen == MODBUS_RX_BUF_SIZE || 
			   (request.rxlen > 0 && (cur_time.tv_sec * 1000000 + cur_time.tv_usec) - 
			   ( request_last_rx_time.tv_sec * 1000000 + request_last_rx_time.tv_usec) >= request.timeout)) {

				uint16_t my_crc16;
				uint16_t his_crc16;
				uint8_t func;
				uint16_t reg_addr;
				uint16_t qty;


				fprintf(stderr, "Modbus RX: parsing request buf = %d\n", request.rxlen);
				fprintf(stderr, "Modbus RX: Data: ");

				for(int i = 0; i < request.rxlen; i++)
					fprintf(stderr, "0x%02x ", request.rxbuf[i]);
				fprintf(stderr, "\n");

				if(request.rxlen < 5) {
					fprintf(stderr, "Modbus RX: bogus request, rxlen = %d is too short!\n", request.rxlen);
					goto modbus_parsing_done; 
				}

				my_crc16 = modbus_crc16(request.rxbuf, request.rxlen - 2);
				his_crc16 = (request.rxbuf[request.rxlen - 1] << 8) | (request.rxbuf[request.rxlen - 2]);

				if(my_crc16 != his_crc16) {
					fprintf(stderr, "Modbus RX: bogus request, my_crc16 (0x%04X) != his_crc16 (0x%04X)\n", my_crc16, his_crc16);
					goto modbus_parsing_done; 
				}

				func = request.rxbuf[1];

				if(request.rxbuf[0] != modbus_address && (request.rxbuf[0] != 0x00 && func != FUNC_READ_ID)) {
					fprintf(stderr, "Modbus RX: request is not for us: my address = %d, his = %d\n", modbus_address, request.rxbuf[0]);
					goto modbus_parsing_done; 
				}

				reg_addr = (request.rxbuf[2] << 8) | request.rxbuf[3];
				qty = (request.rxbuf[4] << 8) | request.rxbuf[5];

				memset(request.txbuf, 0, MODBUS_TX_BUF_SIZE);

				request.txbuf[0] = modbus_address;
				request.txbuf[1] = func;

				printf("Modbus RX: func = %d, reg = %d, qty = %d\r\n", func, reg_addr, qty);

				switch(func) {
					case FUNC_READ_ID:   {
						request.txlen = sprintf(request.txbuf+3, "Modbus/RTU:, date: %s",__DATE__);
						request.txlen += 3;
					} break;

					case FUNC_READ_COILS:
					case FUNC_READ_DISC_INPUT:
					case FUNC_READ_HOLD_REGS: {

						if(qty != 1) {
							request.txlen = modbus_error(func, ERR_WRONG_ARGS, request.txbuf);
							break;
						}


						const char* func_name = MODBUS_FUNCTION_NAMES[func];

						fprintf(stdout, "{request:\"%s\", reg:\"%d\", qty:\"%d\"}\n", func_name, reg_addr, qty);


						// Auto-respond to below regs:

						switch(reg_addr) {
							case REG_SYS_COUNTER: {
								uint8_t* p = (uint8_t*)&reg_sys_counter;
								memcpy_rev(&(request.txbuf[3]), &(p[3]), 4);
								request.txbuf[2] = 4;
								request.txlen = 3+4;
                                                	} break;
						}

					} break;
				}

				modbus_parsing_done:

				request.rxlen = 0; // mark request as already processed

			}



			if(request.txlen > 0) {
				modbus_transmit(&request);
			}
		}

	}

	fprintf(stderr, "%s terminated!\n\n", argv[0]);

	return 0;
}
