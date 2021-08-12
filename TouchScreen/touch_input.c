#include "touch_input.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/input.h>
#include <dirent.h>
#include <features.h>


void PrintProp(const char *name, const int *var)
{
	static const char * absval[6] = { "Value", "Min", "Max", "Fuzz", "Flat", "Resolution" };
	printf("%s", name);
	for (int i = 0; i < 6; i++) {
		printf(" %s=%i,",absval[i], var[i]);
	}
	printf("\n");
}

// время прошедшее с старта старта программы в ms
static unsigned GetTimeMs()
{
	struct timeval curr;
	gettimeofday(&curr, NULL);
	return curr.tv_sec*1000 + curr.tv_usec/1000;
}

#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME "event"
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE           8
#define BITS_PER_LONG           (sizeof(long) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
static int is_event_device(const struct dirent *dir)
{
	return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}

static char *scan_devices(const char *name)
{
	struct dirent **namelist;
	int i, ndev;
	char *filename = NULL;
	long propbit[BITS_TO_LONGS(INPUT_PROP_MAX)] = {0};

	ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, alphasort);
	if (ndev <= 0)
		return NULL;

	for (i = 0; i < ndev; i++) {
		char fname[512];
		int fd = -1;

		snprintf(fname, sizeof(fname), DEV_INPUT_EVENT "/%s", namelist[i]->d_name);
		fd = open(fname, O_RDONLY);
		if (fd < 0)
			continue;

		char dev_name[256] = "Unknown";
		ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name);
		printf("dev_name =%s\n", dev_name);
		if (strcmp(dev_name, name) == 0) {
			close(fd);
			printf("event device found '%s'\n", namelist[i]->d_name);
			filename = malloc(strlen(DEV_INPUT_EVENT) + strlen(EVENT_DEV_NAME) + 12);
			if (!filename)
				break;

			sprintf(filename, "%s/%s%d", DEV_INPUT_EVENT, EVENT_DEV_NAME, i);
			break;
		}

		if ((ioctl(fd, EVIOCGPROP(sizeof(propbit)), propbit) < 0) ||
			!(propbit[BIT_WORD(INPUT_PROP_DIRECT)] & BIT_MASK(INPUT_PROP_DIRECT))) {
			close(fd);
			continue;
		} else {
			close(fd);
			printf("event device found '%s'\n", namelist[i]->d_name);
			filename = malloc(strlen(DEV_INPUT_EVENT) + strlen(EVENT_DEV_NAME) + 12);
			if (!filename)
				break;

			sprintf(filename, "%s/%s%d", DEV_INPUT_EVENT, EVENT_DEV_NAME, i);
			break;
		}
	}

	for (i = 0; i < ndev; ++i)
		free(namelist[i]);

	free(namelist);

	return filename;
}
 
void LinearTransform(struct TOUCH_INPUT *dev)
{
	if (!dev)
		return;
}

struct TOUCH_INPUT *InitInput(const char *input_name, uint screen_x, uint screen_y)
{
	printf("Initializing Input '%s' ...\n", input_name);

	struct TOUCH_INPUT *dev = malloc(sizeof(struct TOUCH_INPUT));
	strncpy(dev->inputName, input_name, sizeof(dev->inputName)); dev->inputName[sizeof(dev->inputName) - 1] = 0;
	dev->screenX = screen_x;
	dev->screenY = screen_y;
	dev->scaleX = dev->scaleY = 65535;
	dev->posX = dev->posY = 0;
	dev->posX = dev->posY = 0;
	dev->rawPosX = dev->rawPosY = 0;
	dev->inputFuzzTime = 100;
	dev->touchBtn = 0;
	dev->untouchBtn = 0;
	dev->linearMatrix[0] = 1;
	dev->linearMatrix[1] = 0;
	dev->linearMatrix[2] = 0;
	dev->linearMatrix[3] = 0;
	dev->linearMatrix[4] = 1;
	dev->linearMatrix[5] = 0;
	dev->linearMatrix[6] = 1;

	if (strchr(input_name, '/') == NULL || strchr(input_name, '?') != NULL) {
		input_name = scan_devices(input_name);
	}

	dev->fd = open(input_name, O_RDONLY|O_NONBLOCK);
    if (dev->fd == -1) {
        fprintf(stderr, "'%s' is not a vaild device\n", input_name);
        free(dev);
        return NULL;
    }

	char name[256] = "Unknown";
	ioctl(dev->fd, EVIOCGNAME(sizeof(name)), name);
	printf("Reading from: '%s', device name='%s'\n", input_name, name);

	int data[6] = {};
	if (ioctl(dev->fd, EVIOCGABS(ABS_MT_POSITION_X), data)>=0) {
		dev->rawPosX = data[0];
		dev->scaleX = data[2];
		PrintProp("ABS_MT_POSITION_X: ", data);
	}
	if (ioctl(dev->fd, EVIOCGABS(ABS_MT_POSITION_Y), data)>=0) {
		dev->rawPosY = data[0];
		dev->scaleY = data[2];
		PrintProp("ABS_MT_POSITION_Y: ", data);
	}
	if (ioctl(dev->fd, EVIOCGABS(ABS_X), data)>=0) {
		if (data[2]) {
			dev->rawPosX= data[0];
			dev->scaleX = data[2];
			dev->minX   = data[1];
			dev->maxX   = data[2];
			dev->resX   = data[5];
		}
		PrintProp("ABS_X: ", data);
	}
	if (ioctl(dev->fd, EVIOCGABS(ABS_Y), data)>=0) {
		if (data[2]) {
			dev->rawPosY= data[0];
			dev->scaleY = data[2];
			dev->minY   = data[1];
			dev->maxY   = data[2];
			dev->resY   = data[5];
		}
		PrintProp("ABS_Y: ", data);
	}

    int grab_flag = 0;// 0 если достаточно неэкслюзвного grab
	int rc = ioctl(dev->fd, EVIOCGRAB, (void*)1);
	if (rc == 0 && !grab_flag)
		ioctl(dev->fd, EVIOCGRAB, (void*)0);
	if (rc) {
		fprintf(stderr, "  Device '%s' is grabbed by another process. Run to see enemy process 'fuser -v %s'\n", input_name, input_name);
	}

    return dev;
}


void ReleaseInput(struct TOUCH_INPUT *dev)
{
	if (!dev)
		return;
	if (dev->fd >= 0) {
		printf("Release '/dev/input/event'...\n");
		ioctl(dev->fd, EVIOCGRAB, (void*)0);
		close(dev->fd);
		dev->fd = -1;
	}
	free(dev);
}

// return 0, если не было ввода
int  InputTouch(struct TOUCH_INPUT *dev)
{
	if (!dev || dev->fd < 0)
		return 0;
	dev->touchBtn = 0;
	dev->untouchBtn = 0;

	struct timeval time_delay = {0, 20000};
	fd_set rdfs;
	FD_ZERO(&rdfs);
	FD_SET(dev->fd, &rdfs);
	if (select(dev->fd + 1, &rdfs, NULL, NULL, &time_delay) <= 0)
		return 0;

	return InputTouchWithoutSelect(dev);
}

// return 0, если не было ввода, select уже выполнен прдварительно
int  InputTouchWithoutSelect(struct TOUCH_INPUT *dev)
{
	if (!dev || dev->fd < 0)
		return 0;
	struct input_event ev[64];
	int coor_change = 0;

	dev->touchBtn = 0;
	dev->untouchBtn = 0;

	int rd_size = read(dev->fd, &ev, sizeof(ev));
	if (rd_size < (int) sizeof(struct input_event) || rd_size % sizeof(struct input_event) != 0) {
		fprintf(stderr, "expected %d bytes, got %d\n", (int) sizeof(struct input_event), rd_size);
		return 0;
	}
	//todo может быть abs_y еще не положен, а abs_x уже положен
	for (int i = 0; i < rd_size / sizeof(struct input_event); i++) {
		uint type = ev[i].type;
		uint code = ev[i].code;
		
	    if (type == EV_ABS && code == ABS_X) {
	    	coor_change = 1;
	    	dev->rawPosX = ev[i].value;
	    	printf("[%i] ABS_X=%i\n", i, dev->rawPosX);
		} else if (type == EV_ABS && code == ABS_Y) {
	    	coor_change = 1;
	    	dev->rawPosY = ev[i].value;
	    	printf("[%i] ABS_Y=%i\n", i, dev->rawPosY);
		} else if (type == EV_REL && code == REL_X) {
	    	coor_change = 1;
	    	dev->rawPosX += ev[i].value;
	    	printf("[%i] REL_X=%i\n", i, dev->rawPosX);
		} else if (type == EV_REL && code == REL_Y) {
	    	coor_change = 1;
	    	dev->rawPosY += ev[i].value;
	    	printf("[%i] REL_Y=%i\n", i, dev->rawPosY);
		} else if (type == EV_KEY && code == BTN_TOUCH) {
	    	if (dev->untouchTime && (GetTimeMs() - dev->untouchTime) < dev->inputFuzzTime) {
				printf("[%i] BTN_TOUCH=%i \033[40;1;31m Delete Fuzz\033[0m\n", i, ev[i].value);
	    	} else if (ev[i].value) {// button press
				printf("[%i] BTN_TOUCH=%i Press\n", i, ev[i].value);
				dev->touchBtn = ev[i].value;
			} else if (!ev[i].value) {// button release
		    	printf("[%i] BTN_TOUCH=%i Release\n", i, ev[i].value);
		   		dev->untouchBtn = 1;
		   		dev->untouchTime = GetTimeMs();
			}
		} else if (type == EV_KEY && code == BTN_LEFT) {
	    	printf("[%i] BTN_LEFT=%i\n", i, ev[i].value);
	    	if (ev[i].value) {// button press
	    		dev->touchBtn = ev[i].value;
			} else if (!ev[i].value) {// button release
	    		dev->untouchBtn = 1;
	    	}
		} else {
	    	printf("type=%i code=%i value=%i\n", type, code, ev[i].value);
		}
	}

	if (coor_change) {
		if (dev->rawPosX < 0)
			dev->rawPosX = 0;
		if (dev->rawPosY < 0)
			dev->rawPosY = 0;
		if (dev->rawPosX >=dev->scaleX)
			dev->rawPosX = dev->scaleX-1;
		if (dev->rawPosY >= dev->scaleY)
			dev->rawPosY = dev->scaleY-1;

		int scr_x = (dev->rawPosX * dev->screenX)/dev->scaleX;//ScreenX - 
		int scr_y = (dev->rawPosY * dev->screenY)/dev->scaleY;
		dev->posX = (dev->linearMatrix[2] + dev->linearMatrix[0]*scr_x + dev->linearMatrix[1]*scr_y) / dev->linearMatrix[6];
		dev->posY = (dev->linearMatrix[5] + dev->linearMatrix[3]*scr_x + dev->linearMatrix[4]*scr_y) / dev->linearMatrix[6];
		if (dev->touchBtn) {
			dev->touchX = dev->posX;
			dev->touchY = dev->posY;
		}

		switch (dev->rotate) {
		int rot_tmp;
		case 1:
			rot_tmp = dev->posX;
			dev->posX = dev->posY;
			dev->posY = dev->screenX - rot_tmp - 1;
			break;
		case 2:
			dev->posX = dev->screenX - dev->posX - 1;
			dev->posY = dev->screenY - dev->posY - 1;
			break;
		case 3:
			rot_tmp = dev->posX;
			dev->posX = dev->screenY - dev->posY - 1;
			dev->posY = rot_tmp;
			break;
		}
	}

	return dev->touchBtn;
}

