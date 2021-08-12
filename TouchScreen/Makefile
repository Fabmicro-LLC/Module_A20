INCLUDES := -I $(SYSROOT)/usr/include -I $(SYSROOT)/include/arm-linux-gnueabihf -I $(SYSROOT)/arm-linux-gnueabihf/libc/usr/include
LIBS := -L $(SYSROOT)/usr/lib -L $(SYSROOT)/lib/arm-linux-gnueabihf -L $(SYSROOT)/arm-linux-gnueabihf/libc/usr/lib -L $(SYSROOT)/usr/lib/arm-linux-gnueabihf
CFLAGS := -O2 -march=armv7-a -mfpu=neon

all: ts_test ts_calibrate

ts_test: ts_test.c
	$(CROSS_COMPILE)gcc $(CFLAGS) $(INCLUDES) $(LIBS) -o ts_test ts_test.c touch_calibrate.c touch_input.c framebuffer.c -lm

ts_calibrate: ts_calibrate.c
	$(CROSS_COMPILE)gcc $(CFLAGS) $(INCLUDES) $(LIBS) -o ts_calibrate ts_calibrate.c touch_calibrate.c touch_input.c framebuffer.c -lm

clean:
	@rm -vf ts_test ts_calibrate *.o *~


