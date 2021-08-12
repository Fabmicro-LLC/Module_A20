#pragma once
#include <stdint.h>

struct FRAME_BUFFER {
	int      screenSize;
	int      screenX, screenY, offsetX, offsetY, pitch, bitsPerPixel;
	void    *ptrMem;// ссылка на маппинг экранной пам€ти
	uint32_t physMem;// физическа€ пам€ть
	int      physSize;// размер физической пам€ти fb дл€ одной страницы
	int      fbFd;
};

struct FRAME_BUFFER *FBInit(const char *fb_name);
void FBRelease(struct FRAME_BUFFER *fb);
// draw to current fb
void FBHLine(struct FRAME_BUFFER *fb, int pos_x, int pos_y, int length, uint32_t color);
// draw to current fb
void FBVLine(struct FRAME_BUFFER *fb, int pos_x, int pos_y, int length, uint32_t color);
void FBClearScreen(struct FRAME_BUFFER *fb, uint32_t color);
