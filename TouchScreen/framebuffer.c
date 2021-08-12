#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "framebuffer.h"

struct FRAME_BUFFER *FBInit(const char *fb_name)
{
	struct FRAME_BUFFER *fb = calloc(1, sizeof(struct FRAME_BUFFER));
	if (!fb) {
		fprintf(stderr, "Not enough memory\n");
		return NULL;
	}

	fb->fbFd = open(fb_name, O_RDWR);//"/dev/graphics/fb0"
	if (fb->fbFd < 0) {
		fprintf(stderr, "Unable to open framebuffer '%s', try '/dev/graphics/fb0'\n", fb_name);
		fb->fbFd = open("/dev/graphics/fb0", O_RDWR);
	}
	if (fb->fbFd < 0) {
		fprintf(stderr, "Unable to open framebuffer dev/graphics/fb0\n");
		return NULL;
	}

	// Выполняем Ioctl. Получаем изменяемую информацию экрана.
	struct fb_var_screeninfo var_info;// изменяемая информация экрана
	if (ioctl(fb->fbFd, FBIOGET_VSCREENINFO, &var_info) < 0) {
	    fprintf(stderr, "error FBIOGET_VSCREENINFO: %s\n", strerror(errno));
	    FBRelease(fb);
		return NULL;
	}
	fprintf(stderr, "Prev Screen: (%dx%dx%d), virtual screen: (%dx%d) size: (%dx%d)\n", var_info.xres, var_info.yres, var_info.bits_per_pixel, var_info.xres_virtual, var_info.yres_virtual, var_info.width, var_info.height);
	fprintf(stderr, "red(%d<<%d), green(%d<<%d), blue(%d<<%d), alpha(%d<<%d)\n", var_info.red.length, var_info.red.offset
		, var_info.green.length, var_info.green.offset, var_info.blue.length, var_info.blue.offset, var_info.transp.length, var_info.transp.offset);
	fb->screenX     = var_info.xres;
	fb->offsetX     = var_info.xoffset;
	fb->screenY     = var_info.yres;
	fb->offsetY     = var_info.yoffset;
	fb->bitsPerPixel= var_info.bits_per_pixel;
	fb->screenSize  = fb->screenX * fb->screenY * fb->bitsPerPixel / 8;
	var_info.xres_virtual = var_info.xres;
	var_info.yres_virtual = var_info.yres*2;
	if (ioctl(fb->fbFd, FBIOPUT_VSCREENINFO, &var_info) < 0) {
		fprintf(stderr, "error FBIOPUT_VSCREENINFO: %s\n", strerror(errno));
	}
	// Выполняем Ioctl. Запрашиваем неизменяемую информацию об экране. 
	struct fb_fix_screeninfo fix_info;// неизменяемая информация об экране
	if (ioctl(fb->fbFd, FBIOGET_FSCREENINFO, &fix_info) < 0) {
		fprintf(stderr, "error FBIOGET_FSCREENINFO: %s\n", strerror(errno));
	    FBRelease(fb);
		return NULL;
	} else {
		fb->physMem  = fix_info.smem_start;
		fb->physSize = fix_info.smem_len;
		fb->pitch    = fix_info.line_length;
	}

	fb->ptrMem = mmap(NULL, fb->physSize, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fbFd, 0);
	if (fb->ptrMem == MAP_FAILED) {
		fprintf(stderr, "mmap failed:\n");
	    FBRelease(fb);
		return NULL;
	}

	memset(fb->ptrMem, 0, fb->physSize);

	fprintf(stderr, "FrameBufferInit() addr=%p phys=0x%x size=%d, pitch=%d %ix%ix%i\n"
		, fb->ptrMem, fb->physMem, fb->physSize, fb->pitch, fb->screenX, fb->screenY, fb->bitsPerPixel);

	return fb;
}

void FBRelease(struct FRAME_BUFFER *fb)
{
	if (!fb)
		return;
	fprintf(stderr, "ReleaseFrameBuffer addr=%p, phys=0x%x size=%d ...\n", fb->ptrMem, fb->physMem, fb->physSize);
	if (fb->ptrMem) {
		munmap(fb->ptrMem, fb->physSize);
	}
	close(fb->fbFd);
	free(fb);
}

// draw to current fb
void FBHLine(struct FRAME_BUFFER *fb, int pos_x, int pos_y, int length, uint32_t color)
{
	if (!fb || !fb->ptrMem)
		return;
	if (pos_x+length <= 0 || pos_y < 0 || pos_x >= fb->screenX || pos_y >= fb->screenY)
		return;
	
	if (pos_x < 0) {
		length += pos_x;
		pos_x = 0;
	}
	if (pos_x+length > fb->screenX) {
		length = fb->screenX - pos_x;
	}

    switch (fb->bitsPerPixel) {
    case 8:
    	{
    	uint8_t *buf = (uint8_t *)fb->ptrMem + fb->pitch * pos_y + pos_x;
    	for(int i =0; i < length; i++, buf++)
    		*buf = (uint8_t)color;
   		}
    	break;
    case 16:
    	{
    	uint16_t *buf = (uint16_t *)fb->ptrMem + fb->pitch/2 * pos_y + pos_x;
    	for(int i =0; i < length; i++, buf++)
    		*buf = (uint16_t)color;
   		}
    	break;
    case 32:
    	{
    	uint32_t *buf = (uint32_t *)fb->ptrMem + fb->pitch/4 * pos_y + pos_x;
    	for(int i =0; i < length; i++, buf++)
    		*buf = color;
   		}
    	break;
    default: fprintf(stderr, "Unknown pixel depth\n");
    }
}

// draw to current fb
void FBVLine(struct FRAME_BUFFER *fb, int pos_x, int pos_y, int length, uint32_t color)
{
	if (!fb || !fb->ptrMem)
		return;
	if (pos_x < 0 || pos_y+length <= 0 || pos_x >= fb->screenX || pos_y >= fb->screenY)
		return;
	
	if (pos_y < 0) {
		length += pos_y;
		pos_y = 0;
	}
	if (pos_y+length > fb->screenY) {
		length = fb->screenY - pos_y;
	}

    switch (fb->bitsPerPixel) {
    case 8:
    	{
    	uint8_t *buf = (uint8_t *)fb->ptrMem + fb->pitch * pos_y + pos_x;
    	for(int i = 0; i < length; i++, buf+=fb->pitch)
    		*buf = (uint8_t)color;
   		}
    	break;
    case 16:
    	{
    	uint16_t *buf = (uint16_t *)fb->ptrMem + fb->pitch/2 * pos_y + pos_x;
    	for(int i = 0; i < length; i++, buf+=fb->pitch/2)
    		*buf = (uint16_t)color;
   		}
    	break;
    case 32:
    	{
    	uint32_t *buf = (uint32_t *)fb->ptrMem + fb->pitch/4 * pos_y + pos_x;
    	for(int i = 0; i < length; i++, buf+=fb->pitch/4)
    		*buf = color;
   		}
    	break;
    default: fprintf(stderr, "Unknown pixel depth\n");
    }
}

void FBClearScreen(struct FRAME_BUFFER *fb, uint32_t color)
{
	if (!fb || !fb->ptrMem)
		return;
	if (color == 0)	{
		memset(fb->ptrMem, 0, fb->screenSize);//todo
	} else {
		uint32_t *ptr = (uint32_t *)fb->ptrMem;
		uint32_t *end = ptr + fb->screenSize/4;
		while (ptr < end)
			*ptr++ = color;
	}
}
