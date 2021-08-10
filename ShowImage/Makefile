INCLUDES := -I $(SYSROOT)/usr/include -I $(SYSROOT)/include/arm-linux-gnueabihf -I $(SYSROOT)/arm-linux-gnueabihf/libc/usr/include
LIBS := -L $(SYSROOT)/usr/lib -L $(SYSROOT)/lib/arm-linux-gnueabihf -L $(SYSROOT)/arm-linux-gnueabihf/libc/usr/lib -L $(SYSROOT)/usr/lib/arm-linux-gnueabihf
CFLAGS := -O2 -march=armv7-a -mfpu=neon

all: show_image

show_image: show_image.c
	$(CROSS_COMPILE)gcc $(CFLAGS) $(INCLUDES) $(LIBS) -o show_image show_image.c -lm

clean:
	@rm -vf show_image *.o *~
