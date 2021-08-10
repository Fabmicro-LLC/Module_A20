#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static int screenX, screenY, bitsPerPixel, pitch;
static uint32_t *ptrMem = NULL;
static uint32_t memSize = 0;

int main(int argc, const char **argv)
{
	if (argc < 2) {
		puts("ShowImage is a simple tool to display PNG images on the screen (/dev/fbX)\n");
		puts("Copyright (C) 2021, Fabmicro, LLC., Tyumen, Russia.\n\n");
		puts("show_image [-c] filename [offset_x offset_y]\n");
		return 0;
	}
	int offset_x = 0, offset_y = 0, clear = 0;
	const char *filename;
	if (strcmp(argv[1], "-c") == 0) {
		clear = 1;
		filename = argv[2];
		if (argc >= 5) {
			offset_x = atoi(argv[3]);
			offset_y = atoi(argv[4]);
		}
	} else {
		filename = argv[1];
		if (argc >= 4) {
			offset_x = atoi(argv[2]);
			offset_y = atoi(argv[3]);
		}
	}
	
	// load picture
	int pict_width, pict_height, channels;
	uint8_t *pict = stbi_load(filename, &pict_width, &pict_height, &channels, 4);
	if (!pict) {
		printf("Can't load picture '%s'\n", filename);
		return 1;
	}
	// swap r, b
	uint8_t *pixels = pict;
	for (int i = 0; i < pict_width * pict_height; ++i) {
		uint8_t temp = pixels[0];
		pixels[0] = pixels[2];
		pixels[2] = temp;
		pixels += 4;
	}

	// init framebuffer
	int fb_fd = open("/dev/fb0", O_RDWR);//"/dev/graphics/fb0"
	if (fb_fd < 0) {
		puts("Unable to open framebuffer '/dev/fb0', try '/dev/graphics/fb0'");
		fb_fd = open("/dev/graphics/fb0", O_RDWR);
	}
	if (fb_fd < 0) {
		puts("Unable to open framebuffer dev/graphics/fb0");
		return 2;
	}

	// Выполняем Ioctl. Получаем изменяемую информацию экрана.
	struct fb_var_screeninfo var_info;// изменяемая информация экрана
	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_info) < 0) {
		printf("error FBIOGET_VSCREENINFO: %s\n", strerror(errno));
		goto end;
	}
	printf("Prev Screen: (%dx%dx%d), virtual screen: (%dx%d), rotate:%d size: (%dx%d)\n", var_info.xres, var_info.yres, var_info.bits_per_pixel, var_info.xres_virtual, var_info.yres_virtual, var_info.rotate, var_info.width, var_info.height);
	printf("red(%d<<%d), green(%d<<%d), blue(%d<<%d), alpha(%d<<%d)\n", var_info.red.length, var_info.red.offset
		, var_info.green.length, var_info.green.offset, var_info.blue.length, var_info.blue.offset, var_info.transp.length, var_info.transp.offset);
	screenX     = var_info.xres;
	screenY     = var_info.yres;
	bitsPerPixel= var_info.bits_per_pixel;
	// Выполняем Ioctl. Запрашиваем неизменяемую информацию об экране. 
	struct fb_fix_screeninfo fix_info;// неизменяемая информация об экране
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix_info) < 0) {
		printf("error FBIOGET_FSCREENINFO: %s\n", strerror(errno));
		goto end;
	} else {
		memSize = fix_info.smem_len;
		pitch   = fix_info.line_length;
	}

	ptrMem = mmap(NULL, memSize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (ptrMem == MAP_FAILED) {
		printf("mmap failed:\n");
		goto end;
	}
	printf("FrameBufferInit() addr=%p size=%d, pitch=%d %ix%ix%i\n", ptrMem, memSize, pitch, screenX, screenY, bitsPerPixel);
	if (clear) {
		memset(ptrMem, 0, memSize);
	}

	// draw picture to framebuffer
	int pos_x = offset_x, pos_y = offset_y;
	int rect_width=pict_width, rect_height=pict_height, stride=pict_width;
	if (bitsPerPixel==32) {
		uint8_t  *scr = (uint8_t *)ptrMem;
		uint32_t *buf = (uint32_t *)pict;
		if (pos_x + rect_width <= 0 || pos_x >= screenX || pos_y + rect_height <= 0 || pos_y >= screenY) {
			goto end;
		}
		if (pos_x < 0) {
			rect_width += pos_x;
			buf = buf - pos_x;
			pos_x = 0;
		}
		if (pos_x + rect_width > screenX) {
			rect_width = screenX - pos_x;
		}
		if (pos_y < 0) {
			rect_height += pos_y;
			buf = buf - pos_y*stride;
			pos_y = 0;
		}
		if (pos_y + rect_height > screenY) {
			rect_height = screenY - pos_y;
		}

		scr += pos_x*4 + pitch * pos_y;
		for (int y = 0; y < rect_height; y++) {
			for (int x = 0; x < rect_width; x++) {
				((uint32_t*)scr)[x] = buf[x];
			}
			buf = buf + stride;
			scr += pitch;
		}
	}

	// free resources
end:
	stbi_image_free(pict);
	if (ptrMem) {
		munmap(ptrMem, memSize);
	}
	if (fb_fd >= 0)
		close(fb_fd);
}
