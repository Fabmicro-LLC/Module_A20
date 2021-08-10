/*
	Simple input device example.
*/


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <linux/input.h> 


const char* input_device = "/dev/input/event0";
int input_fd = -1;

int main(int argc, char *argv[])
{
	int len;
	fd_set rd_set;
	struct timeval timeout;

	fprintf(stderr, "%s - a simple input device example.\nCopyright (C) 2021, Fabmicro, LLC., Tyumen, Russia.\n\n", argv[0]);
	fflush(stderr);

	if((input_fd = open(input_device, O_RDONLY)) < 0) {
		fprintf(stderr, "Failed to open device %s. %s\n\n", input_device, strerror(errno));
                return 2;
        }


	while(1) {

		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		FD_ZERO (&rd_set);

		if (input_fd > 0)
			FD_SET (input_fd, &rd_set);

		if((len = select(FD_SETSIZE, &rd_set, NULL, NULL, &timeout)) < 0) {

			if(errno == EAGAIN)
				continue;

			perror("select");
			break;
		}


		if(input_fd > 0 && FD_ISSET (input_fd, &rd_set)) {

			struct input_event ev;

			if((len = read(input_fd, &ev, sizeof(struct input_event))) <= 0) {
				perror("read event");
				break;
			}

			printf("Event: type = %d, code = %d\n", ev.type, ev.code);


		}

	}

	fprintf(stderr, "%s terminated!\n\n", argv[0]);

	return 0;
}
