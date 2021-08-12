/*
        Simple Touch Screen test utility 

        Copyright (C) 2021, Fabmicro, LLC., Tyumen, Russia.

*/


#include "touch_calibrate.h"


int main(int argc, char **argv)
{
	TouchTest(argc>1? argv[1]: "touch.txt");

	return 0;
}

