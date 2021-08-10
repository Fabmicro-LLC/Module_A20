/*
	A set of tools to work this Modbus/RTU packets

	$Author: rz $

	$Log: modbus_common.h,v $
	Revision 1.1  2021/06/29 18:55:11  rz
	Simple Modbus/RTP responder

	Revision 1.1  2021/06/25 16:15:15  rz
	New module: RS485/Modbus Slave

	Revision 1.1.1.1  2020/08/07 17:28:07  rz
	Initial import of consolidated project

	Revision 1.5  2020/05/12 23:42:13  rz
	Bug in Modbus write hold 32 bit reg has been fixed

	Revision 1.4  2020/05/04 21:20:40  rz
	Some comments added

	Revision 1.3  2020/05/04 21:17:13  rz
	Support for writing 32 bit hold regs added (method:hold32)

*/

#ifndef __MODBUS_COMMON_H__
#define __MODBUS_COMMON_H__

#define	MODBUS_TX_BUF_SIZE	256
#define	MODBUS_RX_BUF_SIZE	256
#define	MODBUS_REQ_TIMEOUT	100			// Wait for response this amount of MS 
#define	MODBUS_RX_TIMEOUT	20			// WAit input data not more than this amout of MS


#define FUNC_READ_COILS                 0x01            // Read one or more coils' status
#define FUNC_READ_DISC_INPUT            0x02            // Read one or more discrete inputes
#define FUNC_READ_HOLD_REGS             0x03            // Read one or more holding registers (current PWM)
#define FUNC_READ_INPUT_REGS            0x04            // Read one or more input registers (inpud ADC)
#define FUNC_WRITE_ONE_COIL_REG         0x05            // Write one coil (ON/OFF)
#define FUNC_WRITE_ONE_HOLD_REG         0x06            // Write one holding register (PWM)
#define FUNC_WRITE_MANY_COIL_REG        0x0F            // Write many coils (ON/OFF)
#define FUNC_WRITE_MANY_HOLD_REGS	0x10            // Write many holding registers (PWM)
#define	FUNC_WRITE_HOLD_REGS		FUNC_WRITE_MANY_HOLD_REGS
#define FUNC_READ_ID                    0x11            // Read device ID

#define ERR_FUNC_OK			0x00            // Error code: no error
#define ERR_FUNC_NOT_IMPLEMENTED        0x01            // Error code: func not implemented
#define ERR_ADDR_NOT_AVAILABLE          0x02            // Error code: register address not available
#define ERR_WRONG_ARGS                  0x03            // Error code: invalid arguments

const char* MODBUS_FUNCTION_NAMES[] = { "unsupported_0x00", 
				"read_coils", 
				"read_discrete_input", 
				"read_hold_regs", 
				"read_input_regs", 
				"write_one_coil",
				"write_one_hold_reg",
				"unsupported_0x07",
				"unsupported_0x08",
				"unsupported_0x09",
				"unsupported_0x0a",
				"unsupported_0x0b",
				"unsupported_0x0c",
				"unsupported_0x0d",
				"unsupported_0x0e",
				"write_many_coils",
				"write_many_hold_regs",
				"read_id"
};

typedef struct _MODBUS_REQUEST {
	int msg_req_complete;
	int msg_req_timeout;
	char txbuf[MODBUS_TX_BUF_SIZE];
	char rxbuf[MODBUS_RX_BUF_SIZE];
	int timeout;
	int txlen;
	int rxlen;
	void *params;
} MODBUS_REQUEST;


#define	MODBUS_READ_ID(modbus_req, addr, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_READ_ID; \
		(modbus_req)->txbuf[2] = 0x0; \
		(modbus_req)->txbuf[3] = 0x0; \
		(modbus_req)->txlen = 4; \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
	}\
	

#define	MODBUS_READ_COIL_REGS(modbus_req, addr, reg, qty, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_READ_COILS; \
		(modbus_req)->txbuf[2] = ((reg) >> 8) & 0xff; \
		(modbus_req)->txbuf[3] = (reg) & 0xff; \
		(modbus_req)->txbuf[4] = ((qty) >> 8) & 0xff; \
		(modbus_req)->txbuf[5] = (qty) & 0xff; \
		(modbus_req)->txlen = 6; \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
	}\


#define	MODBUS_READ_HOLDING_REGS(modbus_req, addr, reg, qty, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_READ_HOLD_REGS; \
		(modbus_req)->txbuf[2] = ((reg) >> 8) & 0xff; \
		(modbus_req)->txbuf[3] = (reg) & 0xff; \
		(modbus_req)->txbuf[4] = ((qty) >> 8) & 0xff; \
		(modbus_req)->txbuf[5] = (qty) & 0xff; \
		(modbus_req)->txlen = 6; \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
	}\

#define	MODBUS_READ_INPUT_REGS(modbus_req, addr, reg, qty, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_READ_INPUT_REGS; \
		(modbus_req)->txbuf[2] = ((reg) >> 8) & 0xff; \
		(modbus_req)->txbuf[3] = (reg) & 0xff; \
		(modbus_req)->txbuf[4] = ((qty) >> 8) & 0xff; \
		(modbus_req)->txbuf[5] = (qty) & 0xff; \
		(modbus_req)->txlen = 6; \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
	}\

#define	MODBUS_WRITE_MANY_HOLD_REGS(modbus_req, addr, reg, qty, data, data_len, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_WRITE_MANY_HOLD_REGS; \
		(modbus_req)->txbuf[2] = ((reg) >> 8) & 0xff; \
		(modbus_req)->txbuf[3] = (reg) & 0xff; \
		(modbus_req)->txbuf[4] = ((qty) >> 8) & 0xff; \
		(modbus_req)->txbuf[5] = (qty) & 0xff; \
		(modbus_req)->txbuf[6] = (data_len); \
		memcpy((modbus_req)->txbuf+7, (data), (data_len)); \
		(modbus_req)->txlen = 7+(data_len); \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
		svc_debug_print(str, sprintf(str, "MODBUS: out of memory (addr=%d, func=%d)\r\n", addr, FUNC_WRITE_MANY_HOLD_REGS)); \
	}\


#define	MODBUS_WRITE_ONE_HOLD_REG(modbus_req, addr, reg, data, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_WRITE_ONE_HOLD_REG; \
		(modbus_req)->txbuf[2] = ((reg) >> 8) & 0xff; \
		(modbus_req)->txbuf[3] = (reg) & 0xff; \
		(modbus_req)->txbuf[4] = ((data) >> 8) & 0xff; \
		(modbus_req)->txbuf[5] = (data) & 0xff; \
		(modbus_req)->txlen = 6; \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
	}\

#define	MODBUS_WRITE_ONE_HOLD32_REG(modbus_req, addr, reg, data, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_WRITE_ONE_HOLD_REG; \
		(modbus_req)->txbuf[2] = ((reg) >> 8) & 0xff; \
		(modbus_req)->txbuf[3] = (reg) & 0xff; \
		(modbus_req)->txbuf[4] = ((data) >> 24) & 0xff; \
		(modbus_req)->txbuf[5] = ((data) >> 16) & 0xff; \
		(modbus_req)->txbuf[6] = ((data) >> 8) & 0xff; \
		(modbus_req)->txbuf[7] = (data) & 0xff; \
		(modbus_req)->txlen = 8; \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
	}\

#define	MODBUS_WRITE_ONE_COIL_REG(modbus_req, addr, reg, data, msg_complete, msg_timeout) \
	if(modbus_req) {\
		(modbus_req)->txbuf[0] = (addr); \
		(modbus_req)->txbuf[1] = FUNC_WRITE_ONE_COIL_REG; \
		(modbus_req)->txbuf[2] = ((reg) >> 8) & 0xff; \
		(modbus_req)->txbuf[3] = (reg) & 0xff; \
		(modbus_req)->txbuf[4] = ((data) >> 8) & 0xff; \
		(modbus_req)->txbuf[5] = (data) & 0xff; \
		(modbus_req)->txlen = 6; \
		(modbus_req)->rxlen = 0; \
		(modbus_req)->timeout = MODBUS_REQ_TIMEOUT; \
		(modbus_req)->msg_req_complete = msg_complete; \
		(modbus_req)->msg_req_timeout = msg_timeout; \
	} else {\
	}\


typedef struct _MODBUS_RESPONSE {
        int msg_response;
        char txbuf[MODBUS_TX_BUF_SIZE];
        char rxbuf[MODBUS_RX_BUF_SIZE];
        int txlen;
        int rxlen;
        int registered_func;
        int registered_reg_start;
        int registered_reg_end;
} MODBUS_RESPONSE;


#define	MODBUS_CREATE_RESPONDER(modbus_resp, msg, func, reg_start, reg_end) \
	memset(modbus_resp, 0, sizeof(MODBUS_RESPONSE)); \
	(modbus_resp)->msg_response = msg; \
	(modbus_resp)->registered_func = func; \
	(modbus_resp)->registered_reg_start = reg_start; \
	(modbus_resp)->registered_reg_end = reg_end; \

#define	MODBUS_GET_REQ_FUNC(modbus_resp) (unsigned int)((modbus_resp)->rxbuf[1])
#define	MODBUS_GET_REQ_QTY(modbus_resp) (unsigned int)(((modbus_resp)->rxbuf[4] << 8) | modbus_resp->rxbuf[5])
#define	MODBUS_GET_REQ_REG(modbus_resp) (unsigned int)(((modbus_resp)->rxbuf[2] << 8) | modbus_resp->rxbuf[3])
#define	MODBUS_GET_REQ_REG_BUF(modbus_resp, offset) (char*)(((modbus_resp)->rxbuf+7+(offset)))
#define	MODBUS_GET_REQ_REG_BUF_SIZE(modbus_resp) (uint8_t)(((modbus_resp)->rxbuf[6]))
#define	MODBUS_GET_REQ_REG_VAL(modbus_resp, index) (unsigned int)(((modbus_resp)->rxbuf[index*2+7] << 8) | modbus_resp->rxbuf[index*2+8])

#define	MODBUS_GET_RESP_BUF(modbus_resp) (char*)(((modbus_resp)->txbuf+7))

#define MODBUS_RESPONSE_SET_BUF_SIZE(modbus_resp, size) \
	(modbus_resp)->txbuf[2] = size; \
	(modbus_resp)->txlen += size; \

#define MODBUS_RESPONSE_ERROR(modbus_resp, func, error) \
	(modbus_resp)->txbuf[1] = func | 0x80; \
	(modbus_resp)->txbuf[2] = error; \
	(modbus_resp)->txlen = 3; \

#define MODBUS_RESPONSE_OK(modbus_resp, func) \
	(modbus_resp)->txbuf[1] = func | 0x80; \
	(modbus_resp)->txbuf[2] = ERR_FUNC_OK; \
	(modbus_resp)->txlen = 3; \

 
#define MODBUS_RESPONSE_START(modbus_resp, func) \
	(modbus_resp)->txbuf[0] = 0; \
	(modbus_resp)->txbuf[1] = func; \
	(modbus_resp)->txbuf[2] = 0; \
	(modbus_resp)->txlen = 3;

#define MODBUS_RESPONSE_ADD_BYTE(modbus_resp, val) \
	(modbus_resp)->txbuf[2] += 1; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 1; \

#define MODBUS_RESPONSE_ADD_WORD(modbus_resp, val) \
	(modbus_resp)->txbuf[2] += 2; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 2; \

#define MODBUS_RESPONSE_ADD_DWORD(modbus_resp, val) \
	(modbus_resp)->txbuf[2] += 4; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 24) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = (((uint32_t)val) >> 16) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+2] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+3] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 4; \



#define MODBUS_WRITE_RESPONSE_START(modbus_resp, func) \
	(modbus_resp)->txbuf[0] = 0; \
	(modbus_resp)->txbuf[1] = func; \
	(modbus_resp)->txlen = 2;

#define MODBUS_WRITE_RESPONSE_ADD_BYTE(modbus_resp, val) \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 1; \

#define MODBUS_WRITE_RESPONSE_ADD_WORD(modbus_resp, val) \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 2; \

#define MODBUS_WRITE_RESPONSE_ADD_DWORD(modbus_resp, val) \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 24) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = (((uint32_t)val) >> 16) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+2] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+3] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 4; \

#define	MODBUS_WRITE_RESPONSE_BUF(modbus_resp) (char*)(((modbus_resp)->txbuf+8))
#define	MODBUS_WRITE_RESPONSE_BUF_SIZE(modbus_resp, size) (modbus_resp)->txlen = (size+8)

#endif //__MODBUS_COMMON_H__
