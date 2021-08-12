/*
Код вдохновлен либой tslib, можно сказать стандартом работы с тач скрином. Рекомендую использовать ее
https://github.com/libts/tslib
*/

#include <signal.h>//for signal, to prevent leaking the layer on termination
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "framebuffer.h"
#include "touch_input.h"
#include "touch_calibrate.h"

#define CROSS_BOUND_DIST	80

static struct FRAME_BUFFER   *FrameBuffer = NULL;
static struct TOUCH_INPUT    *touch = NULL;

// время прошедшее с старта старта программы в ms
static unsigned GetTimeMs()
{
	struct timeval curr;
	gettimeofday(&curr, NULL);
	return curr.tv_sec*1000 + curr.tv_usec/1000;
}


static void interrupt_handler(int signum)
{
	ReleaseInput(touch);
	FBRelease(FrameBuffer);
	fprintf(stderr, "\n !!! signal=%d(%s) !!!\n", signum, strsignal(signum));
	exit(1);
}

void DrawCross(int32_t x, int32_t y, uint32_t col)
{
	FBHLine(FrameBuffer, x - 10, y, 8, col);
	FBHLine(FrameBuffer, x + 2, y, 8, col);
	FBVLine(FrameBuffer, x, y - 10, 8, col);
	FBVLine(FrameBuffer, x, y + 2, 8, col);

}

static int Init(const char *config_file)
{
	signal(SIGSEGV, interrupt_handler);
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);

	FrameBuffer = FBInit("/dev/fb0");
	if (!FrameBuffer)
		return 5;

	touch = InitInput("ft5x_ts", FrameBuffer->screenX, FrameBuffer->screenY);
	if (!touch)
		touch = InitInput("sun4i-ts", FrameBuffer->screenX, FrameBuffer->screenY);
	if (!touch) {
		FBRelease(FrameBuffer);
		FrameBuffer = NULL;
		fprintf(stderr, "Can't open touch device\n");
		return 4;//goto end1;
	}

	FILE *calibrate_file = fopen(config_file, "r");
 	if (calibrate_file) {
 		int rc = fscanf(calibrate_file, "%i %i %i %i %i %i %i", &touch->linearMatrix[0], &touch->linearMatrix[1], &touch->linearMatrix[2], &touch->linearMatrix[3], &touch->linearMatrix[4], &touch->linearMatrix[5], &touch->linearMatrix[6]);
 		if (rc!=7)
 			fprintf(stderr, "Can't scanf 7 parameters from '%s' file", config_file);
		else
	 		printf("TouchCalibrateMatrix: %i, %i, %i, %i, %i, %i, %i\n", touch->linearMatrix[0], touch->linearMatrix[1], touch->linearMatrix[2], touch->linearMatrix[3], touch->linearMatrix[4], touch->linearMatrix[5], touch->linearMatrix[6]);
		fclose(calibrate_file);
	} else {
		fprintf(stderr, "Can't open '%s' file", config_file);
	}

	return  0;
}

int TouchTest(const char *config_file)
{
	int press_count = 0, release_count=0;
	int x,y, raw_x,raw_y, color = 0;
	int rc = Init(config_file);
	if (rc)
		goto end;

	while (1) {
		InputTouch(touch);
		FBClearScreen(FrameBuffer, 0);

		raw_x = (touch->rawPosX * FrameBuffer->screenX)/touch->scaleX;//ScreenX - 
		raw_y = (touch->rawPosY * FrameBuffer->screenY)/touch->scaleY;
		if (touch->touchBtn) {
			x = touch->posX;
			y = touch->posY;
			fprintf(stderr, "InputTouch return: ABS_X=%i(%i)->%i, ABS_Y=%i(%i)->%i\n", raw_x, touch->rawPosX, x, raw_y, touch->rawPosY, y);
			color = 0xffffffff;
			press_count++;
		}
		if (touch->untouchBtn) {
			x = touch->posX;
			y = touch->posY;
			touch->untouchBtn = 1;
			fprintf(stderr, "InputUnTouch return: ABS_X=%i(%i)->%i, ABS_Y=%i(%i)->%i\n", raw_x, touch->rawPosX, x, raw_y, touch->rawPosY, y);
			color = 0xff00ff00;
			release_count++;
		}

		DrawCross(raw_x,raw_y,0xffff0000);
		DrawCross(touch->touchX, touch->touchY, 0xff0000ff);
		if (color)
			DrawCross(x,y,color);
	}

end:
	FBRelease(FrameBuffer);
	ReleaseInput(touch);
	return rc;
}


typedef struct {
	int x[5], xfb[5];
	int y[5], yfb[5];
	int a[7];
} calibration;

static void GetSample(calibration *cal, int index, int x, int y, const char *name, short redo)
{
	DrawCross(x, y, 0xffffffff);
	while (1) {
		InputTouch(touch);
		if (touch->touchBtn) {
			cal->x[index] = touch->touchX;
			cal->y[index] = touch->touchY;
			cal->x[index] = (touch->rawPosX * FrameBuffer->screenX)/touch->scaleX;//ScreenX - 
			cal->y[index] = (touch->rawPosY * FrameBuffer->screenY)/touch->scaleY;
//			cal->x[index] = cal->x[index] - (cal->x[index]-400)/20;// тестовые искажения
//			cal->y[index] = cal->y[index] - (cal->y[index]-240)/20;// тестовые искажения
			DrawCross(cal->x[index], cal->y[index], 0xffff0000);
			break;
		}
	}

	cal->xfb[index] = x;
	cal->yfb[index] = y;

	fprintf(stderr, "%s: (%d, %d) <- (%d, %d)\n", name, x,y, cal->x[index], cal->y[index]);
}

int PerformCalibration(calibration *cal)
{
	int j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float scaling = 65536.0;

	/* Get sums for matrix */
	n = x = y = x2 = y2 = xy = 0;
	for (j = 0; j < 5; j++) {
		n += 1.0;
		x += (float)cal->x[j];
		y += (float)cal->y[j];
		x2 += (float)(cal->x[j]*cal->x[j]);
		y2 += (float)(cal->y[j]*cal->y[j]);
		xy += (float)(cal->x[j]*cal->y[j]);
	}

	/* Get determinant of matrix -- check if determinant is too small */
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if (det < 0.1 && det > -0.1) {
		fprintf(stderr, "calibrate: determinant is too small -- %f\n", det);
		return 0;
	}

	/* Get elements of inverse matrix */
	a = (x2*y2 - xy*xy)/det;
	b = (xy*y - x*y2)/det;
	c = (x*xy - y*x2)/det;
	e = (n*y2 - y*y)/det;
	f = (x*y - n*xy)/det;
	i = (n*x2 - x*x)/det;

	/* Get sums for x calibration */
	z = zx = zy = 0;
	for (j = 0; j < 5; j++) {
		z += (float)cal->xfb[j];
		zx += (float)(cal->xfb[j]*cal->x[j]);
		zy += (float)(cal->xfb[j]*cal->y[j]);
	}

	/* Now multiply out to get the calibration for framebuffer x coord */
	cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
	cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
	cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling));

	printf("coeff(0,1,2): %f %f %f\n", (a*z + b*zx + c*zy), (b*z + e*zx + f*zy), (c*z + f*zx + i*zy));

	/* Get sums for y calibration */
	z = zx = zy = 0;
	for (j = 0; j < 5; j++) {
		z += (float)cal->yfb[j];
		zx += (float)(cal->yfb[j]*cal->x[j]);
		zy += (float)(cal->yfb[j]*cal->y[j]);
	}

	/* Now multiply out to get the calibration for framebuffer y coord */
	cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
	cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
	cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling));

	printf("coeff(3,4,5): %f %f %f\n", (a*z + b*zx + c*zy), (b*z + e*zx + f*zy), (c*z + f*zx + i*zy));

	/* If we got here, we're OK, so assign scaling to a[6] and return */
	cal->a[6] = (int)scaling;
	printf("scaling: %f\n", scaling);

	return 1;
}

int TouchCalibrate(const char *config_file)
{
	int rc = Init(config_file);

	calibration cal = {0};
	struct CALIBRATE lin[1]={0};
	unsigned int tick = 0;
	unsigned int min_interval = 0;
	int rotation = 0;
	int xres = FrameBuffer->screenX;
	int yres = FrameBuffer->screenY;
	/* ignore rotation for calibration. only save it.*/
	int rotation_temp = rotation;
	int xres_temp = xres;
	int yres_temp = yres;
	rotation = 0;
	xres = FrameBuffer->screenX;
	yres = FrameBuffer->screenY;

	short redo = 0;

	if (rc)
		goto end;

redocalibration:
	tick = GetTimeMs();
	GetSample(&cal, 0, CROSS_BOUND_DIST,        CROSS_BOUND_DIST,        "Top left ", redo);
	redo = 0;
	if (GetTimeMs() - tick < min_interval) {
		redo = 1;
	#ifdef DEBUG
		printf("ts_calibrate: time before touch press < %dms. restarting.\n", min_interval);
	#endif
		goto redocalibration;
	}

	tick = GetTimeMs();
	GetSample(&cal, 1, xres - CROSS_BOUND_DIST, CROSS_BOUND_DIST,        "Top right", redo);
	if (GetTimeMs() - tick < min_interval) {
		redo = 1;
	#ifdef DEBUG
		printf("ts_calibrate: time before touch press < %dms. restarting.\n", min_interval);
	#endif
		goto redocalibration;
	}

	tick = GetTimeMs();
	GetSample(&cal, 2, xres - CROSS_BOUND_DIST, yres - CROSS_BOUND_DIST, "Bot right", redo);
	if (GetTimeMs() - tick < min_interval) {
		redo = 1;
	#ifdef DEBUG
		printf("ts_calibrate: time before touch press < %dms. restarting.\n", min_interval);
	#endif
		goto redocalibration;
	}

	tick = GetTimeMs();
	GetSample(&cal, 3, CROSS_BOUND_DIST,        yres - CROSS_BOUND_DIST, "Bot left ", redo);
	if (GetTimeMs() - tick < min_interval) {
		redo = 1;
	#ifdef DEBUG
		printf("ts_calibrate: time before touch press < %dms. restarting.\n", min_interval);
	#endif
		goto redocalibration;
	}

	tick = GetTimeMs();
	GetSample(&cal, 4, FrameBuffer->screenX / 2,  FrameBuffer->screenY / 2,  "Center   ", redo);
	if (GetTimeMs() - tick < min_interval) {
		redo = 1;
	#ifdef DEBUG
		printf("ts_calibrate: time before touch press < %dms. restarting.\n", min_interval);
	#endif
		goto redocalibration;
	}

	rotation = rotation_temp;
	xres = xres_temp;
	yres = yres_temp;

	lin->a[0] = 1;
	lin->a[1] = 0;
	lin->a[2] = 0;
	lin->a[3] = 0;
	lin->a[4] = 1;
	lin->a[5] = 0;
	lin->a[6] = 1;
/*	lin->p_offset = 0;
	lin->p_mult   = 1;
	lin->p_div    = 1;
	lin->swap_xy  = 0;*/
	lin->rotate = 0;
	lin->cal_res_x = FrameBuffer->screenX;
	lin->cal_res_y = FrameBuffer->screenY;

	if (PerformCalibration (&cal)) {
		printf("Calibration constants: ");
		for (int i = 0; i < 7; i++)
			printf("%d ", cal.a[i]);
		printf("\n");

		FILE *file = fopen(config_file, "w");
		if (file) {
	 		fprintf(file, "%i %i %i %i %i %i %i", cal.a[1], cal.a[2], cal.a[0], cal.a[4], cal.a[5], cal.a[3], cal.a[6]);//xres_orig, yres_orig, rotation);
		} else {
			fprintf(stderr, "Can't open '%s' file for write\n", config_file);
		}

		lin->a[0] = cal.a[1];//1;
		lin->a[1] = cal.a[2];//0;
		lin->a[2] = cal.a[0];//0;
		lin->a[3] = cal.a[4];//0;
		lin->a[4] = cal.a[5];//1;
		lin->a[5] = cal.a[3];//0;
		lin->a[6] = cal.a[6];//1;
		printf("Calibration result:\n");
		int x[5], y[5];
		for (int j = 0; j < 5; j++) {
			x[j] = cal.x[j];
			y[j] = cal.y[j];
			LinearCalibrate(lin, &x[j], &y[j]);
			DrawCross(x[j], y[j], 0xff00ff00);
//			LOGI("need(%4i,%4i)  touch(%4i,%4i)  calibrated(%4i,%4i)", cal.xfb[j], cal.yfb[j], cal.x[j], cal.y[j], x[j], y[j]);
		}
		printf("need:       (%4i,%4i) (%4i,%4i) (%4i,%4i) (%4i,%4i) (%4i,%4i)\n", cal.xfb[0], cal.yfb[0], cal.xfb[1], cal.yfb[1], cal.xfb[2], cal.yfb[2], cal.xfb[3], cal.yfb[3], cal.xfb[4], cal.yfb[4]);
		printf("touch:      (%4i,%4i) (%4i,%4i) (%4i,%4i) (%4i,%4i) (%4i,%4i)\n", cal.x[0], cal.y[0], cal.x[1], cal.y[1], cal.x[2], cal.y[2], cal.x[3], cal.y[3], cal.x[4], cal.y[4]);
		printf("calibrated: (%4i,%4i) (%4i,%4i) (%4i,%4i) (%4i,%4i) (%4i,%4i)\n", x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3], x[4], y[4]);

	} else {
		fprintf(stderr, "Calibration failed.\n");
	}



end:
	ReleaseInput(touch);
	FBRelease(FrameBuffer);
	return rc;
}

void LinearCalibrate(struct CALIBRATE *lin, int *x, int *y)
{
	int xtemp = *x;
	int ytemp = *y;
	*x = (lin->a[2] + lin->a[0]*xtemp + lin->a[1]*ytemp) / lin->a[6];
	*y = (lin->a[5] + lin->a[3]*xtemp + lin->a[4]*ytemp) / lin->a[6];
/*	if (info->dev->res_x && lin->cal_res_x)
		*x = *x * info->dev->res_x / lin->cal_res_x;
	if (info->dev->res_y && lin->cal_res_y)
		*y = *y * info->dev->res_y / lin->cal_res_y;*/

/*	*pressure = ((*pressure + lin->p_offset) * lin->p_mult) / lin->p_div;
	if (lin->swap_xy) {
		int tmp = *x;

		*x = *y;
		*y = tmp;
	}*/

	switch (lin->rotate) {
	int rot_tmp;
	case 0:
		break;
	case 1:
		rot_tmp = *x;
		*x = *y;
		*y = lin->cal_res_x - rot_tmp - 1;
		break;
	case 2:
		*x = lin->cal_res_x - *x - 1;
		*y = lin->cal_res_y - *y - 1;
		break;
	case 3:
		rot_tmp = *x;
		*x = lin->cal_res_y - *y - 1;
		*y = rot_tmp ;
		break;
	default:
		break;
	}
}



