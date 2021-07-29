#BOARD?=obu
#ARCH ?= arm
#ARCHITECTURE ?= arm-linux-gnueabihf
#CROSS_COMPILE ?= $(ARCHITECTURE)-
#BUILD_PATH ?= ../../../../bsp/kernel/linux-imx
BUILD_PATH ?= ../../bsp/k4.11-hostconfig/
#BUILD_PATH:=/home/duser/obu-wl18xx/bsp/kernel/linux-imx/
INSTALLDIR ?= ../../../../bsp/image/rootfs/
PWD:= $(shell pwd)

#export BOARD
#export ARCH
#export CROSS_COMPILE
#export BUILD_PATH

#INCL:= -I/home/duser/obu-wl18xx/bsp/wl18xx/build-utilites/src/compat_wireless/backport-include/net/

#KBUILD_EXTRA_SYMBOLS:=/home/duser/obu-wl18xx-new-v3/bsp/wl18xx/build-utilites/src/compat_wireless/Module.symvers  /home/duser/obu-wl18xx-new-v3/intech/kernel/drivers/intech/llc/Module.symvers
KBUILD_EXTRA_SYMBOLS:=/home/duser/obu-wl18xx-new-v3/intech/kernel/drivers/intech/llc/Module.symvers
EXTRA_CFLAGS += -DDEBUG -save-temps 

#ccflags-y := -I/home/duser/obu-wl18xx-new-v3/bsp/wl18xx/build-utilites/src/compat_wireless/include/net  \
#	     -I/home/duser/obu-wl18xx-new-v3/bsp/wl18xx/build-utilites/src/compat_wireless/include \
#	     -I/home/duser/obu-wl18xx-new-v3/bsp/wl18xx/build-utilites/src/compat_wireless/backport-include \
#	     -I/home/duser/obu-wl18xx-new-v3/intech/kernel/include

ccflags-y += -g

obj-m += wch11p.o
wch11p-y = netdriver.o mac80211-ops.o wch11p-ops.o debug_ieee80211.o debug.o

all:
	make -C $(BUILD_PATH) M=$(PWD) $(INCL) modules

install:
	install -d $(INSTALLDIR)/opt/atag/drivers
	install -c wch11p.ko $(INSTALLDIR)/opt/atag/drivers

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.inc
	rm -f Module.markers Module.symvers modules.order
