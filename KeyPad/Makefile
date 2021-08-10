PROG		:= keypad 
SOURCES		= keypad.cc 
LIB		:=
LIB_SOURCES	=
INSTALLDIR	:= ../
LIBS            =
CPPFLAGS_COMMON	=
CFLAGS_COMMON	=

#########################################################################################################################################
#################### PLEASE DO NOT TOUCH THINGS BELOW !!! ANY SINGLE SPACE CHARACTER WILL BRAKE THE WHOLE THING !!!  ####################
#########################################################################################################################################



X86_COMPILER	:=	
X86_SYSROOT	:=	/
X86_LINKER	=	g++
X86_CFLAGS	= 	-g -fPIC -pthread -std=c99 -Wall -Wno-misleading-indentation -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -I$(SYSROOT)/include
X86_CPPFLAGS	= 	-g -fPIC -pthread -std=c++14 -Wall -Wno-misleading-indentation -Wno-int-to-pointer-cast -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/include/arm-linux-gnueabihf
X86_LDFLAGS	:= 	-lstdc++ -lsupc++ 

#ARM_COMPILER	:=	/opt/A13/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
#ARM_SYSROOT	:=	/opt/A13/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf
ARM_COMPILER	:=	/opt/linaro/gcc-linaro-6.3.1-2017.02-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ARM_SYSROOT	:=	/opt/linaro/gcc-linaro-6.3.1-2017.02-i686_arm-linux-gnueabihf
ARM_LINKER	=	$(CROSS_COMPILE)g++ -L$(SYSROOT)/lib -L$(SYSROOT)/usr/lib -L$(SYSROOT)/arm-linux-gnueabihf -L$(SYSROOT)/arm-linux-gnueabihf/lib -L$(SYSROOT)/usr/lib/arm-linux-gnueabihf
ARM_CFLAGS	= 	-g -fPIC -pthread -std=c99 -mfpu=neon -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -I$(SYSROOT)/include -I$(SYSROOT)/usr/include/arm-linux-gnueabihf
ARM_CPPFLAGS	= 	-g -fPIC -pthread -std=c++14 -mfpu=neon -Wall -Wno-int-to-pointer-cast -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/include/arm-linux-gnueabihf
ARM_LDFLAGS	:= 	-lstdc++ -lsupc++ 

CPP		=	$(CROSS_COMPILE)g++
CC		=	$(CROSS_COMPILE)gcc
AR		=	$(CROSS_COMPILE)ar
RANLIB		=	$(CROSS_COMPILE)ranlib
CP		:=	/bin/cp
BUILDDIR_ARM	=	build_arm
BUILDDIR_X86	=	build_x86


_arm: CROSS_COMPILE = $(ARM_COMPILER)
_arm: SYSROOT 	:= $(ARM_SYSROOT)
_arm: LINKER 	:= $(ARM_LINKER)
_arm: ARCH 	:= 'ARM'
_arm: CFLAGS 	:= $(ARM_CFLAGS) $(CFLAGS_COMMON)
_arm: CPPFLAGS 	:= $(ARM_CPPFLAGS) $(CPPFLAGS_COMMON) 
_arm: LDFLAGS 	:= $(ARM_LDFLAGS) 

_x86: CROSS_COMPILE = $(X86_COMPILER)
_x86: SYSROOT 	:= $(X86_SYSROOT)
_x86: LINKER 	:= $(X86_LINKER)
_x86: ARCH 	:= 'X86'
_x86: CFLAGS 	:= $(X86_CFLAGS) $(CFLAGS_COMMON) 
_x86: CPPFLAGS 	:= $(X86_CPPFLAGS) $(CPPFLAGS_COMMON)
_x86: LDFLAGS 	:= $(X86_LDFLAGS) 


PROGBIN 	:= $(addprefix $(BUILDDIR)/,$(PROG))
LIBBIN 		:= $(addprefix $(BUILDDIR)/,$(LIB))

default:
		@$(MAKE) x86
		@$(MAKE) arm

x86:
		@$(MAKE) _x86 BUILDDIR=$(BUILDDIR_X86)

arm:
		@$(MAKE) _arm BUILDDIR=$(BUILDDIR_ARM)


_x86:		check_build_dir build_lib build_prog done 

_arm:		check_build_dir build_lib build_prog done


install:
		@$(MAKE) install_x86
		@$(MAKE) install_arm

install_x86: INSTALLTO = $(INSTALLDIR)/$(BUILDDIR_X86)
install_x86: BUILDDIR = $(BUILDDIR_X86)
install_x86: ARCH = 'X86'
install_x86: check_install_dir install_all done
		
install_arm: INSTALLTO = $(INSTALLDIR)/$(BUILDDIR_ARM)
install_arm: BUILDDIR = $(BUILDDIR_ARM)
install_arm: ARCH = 'ARM'
install_arm: check_install_dir install_all done
		

check_install_dir:	
		@echo
		@echo '=== INSTALLING FOR:' $(ARCH), 'TO:' $(INSTALLTO) 
		@if [ ! -d $(INSTALLTO) ]; then mkdir $(INSTALLTO); fi

install_all:	
		- $(foreach file, $(PROGBIN), $(CP) $(BUILDDIR)/$(file) $(INSTALLTO)/$(file);)





check_build_dir: 
		@echo
		@echo '=== BUILDING FOR:' $(ARCH)
		@echo '	BUILD DIR:' $(BUILDDIR)
		@echo '	SYSROOT DIR:' $(SYSROOT)
		@if [ ! -d $(BUILDDIR) ]; then mkdir $(BUILDDIR); fi
		@echo '	SOURCES:' $(SOURCES)
		@echo '	PROGOBJS:' $(PROGOBJS)
		@echo '	PROGBIN:' $(PROGBIN)
		@echo '	LIBBIN:' $(LIBBIN)
		@echo

done:
		@echo '=== DONE FOR:' $(ARCH)
		@echo


define make_one

${1} : ${2}

ifeq ($(suffix ${2}),.cc)
		@echo '	COMPILNG C++:' $$< '->' $$@ 
		$$(CPP) -c $$(CPPFLAGS) $$< -o $$@ 
		@echo
endif

ifeq ($(suffix ${2}),.c)
		@echo '	COMPILNG C:' $$< '->' $$@ 
		$$(CC) -c $$(CFLAGS) $$< -o $$@ 
		@echo
endif

ifeq ($(suffix ${2}),.S)
		@echo '	COMPILNG ASM:' $$< '->' $$@ 
		$$(CC) -c $$(CFLAGS) $$< -o $$@
		@echo
endif

PROGOBJS += ${1}
LIBOBJS += ${1}
endef


build_lib:		$(foreach in,${LIB_SOURCES},$(eval $(call make_one,$(BUILDDIR)/$(notdir $(basename ${in})).o,${in}))) 
build_lib:		$(LIBBIN)


build_prog:		$(foreach in,${SOURCES},$(eval $(call make_one,$(BUILDDIR)/$(notdir $(basename ${in})).o,${in}))) 
build_prog:		$(PROGBIN)


$(PROGBIN):	$(PROGOBJS) $(LIBBIN)
		@echo '	LINKING PROG:' $(PROGOBJS) '->' $@
		$(LINKER) $(LDFLAGS) $(PROGOBJS) -o $@ $(LIBS)
		@echo


$(LIBBIN):	$(LIBOBJS)
		@echo '	CREATING LIB:' $(LIBOBJS) '->' $@
		$(AR) rvs $@ $(LIBOBJS) 
		$(RANLIB) $@
		@echo

clean:
		rm -rf ./$(BUILDDIR_X86)
		rm -rf ./$(BUILDDIR_ARM)
		@echo



