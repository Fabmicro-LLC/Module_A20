#pragma once

struct CALIBRATE {
/*	int	swap_xy;

	// Linear scaling and offset parameters for pressure
	int	p_offset;
	int	p_mult;
	int	p_div;*/

	// Linear scaling and offset parameters for x,y (can include rotation)
	int	a[7];

	// Screen resolution at the time when calibration was performed
	unsigned int cal_res_x;
	unsigned int cal_res_y;

	// rotate: 0-none, 1-90cw, 2-180cw, 3-90ccw
	unsigned int rotate;
};


// config_file - json config file for touch
int TouchCalibrate(const char *config_file);

// config_file - json config file for touch
int TouchTest(const char *config_file);

void LinearCalibrate(struct CALIBRATE *lin, int *x, int *y);
