#pragma once

struct TOUCH_INPUT {
	int  inputFuzzTime;
	int  fd;
	int  scaleX, scaleY;
	int  minX, maxX, resX, minY, maxY, resY;
	int  rawPosX, rawPosY;
	int  posX, posY;// current position after calibrate, rotate, antifuzz, resize to screen size
	int  touchX, touchY;// first touch. After calibrate, rotate, antifuzz, resize to screen size
	int  touchBtn;// при нажатии пальцем
	int  untouchBtn;// при отпускании пальца
	unsigned untouchTime;// при отпускании пальца
	// calibration matrix
	int  linearMatrix[7];// первые 6 - две строки матричного преобразования, 7-ой элемент - делитель
	// rotate: 0-none, 1-90cw, 2-180cw, 3-90ccw
	unsigned rotate;
	unsigned screenX, screenY;// screen resolution
	char inputName[32];
};

struct TOUCH_INPUT *InitInput(const char *input_name, unsigned screen_x, unsigned screen_y);
void ReleaseInput(struct TOUCH_INPUT *dev);
// return 0, если не было ввода
int  InputTouch(struct TOUCH_INPUT *dev);
// return 0, если не было ввода, select уже выполнен прдварительно
int  InputTouchWithoutSelect(struct TOUCH_INPUT *dev);

