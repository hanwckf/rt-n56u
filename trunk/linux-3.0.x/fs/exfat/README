HOW TO BUILD YOUR KERNEL WITH THE exFAT FILESYSTEM DRIVER

- get Kernel code of your Samsung Product in OSRC website. (http://opensource.samsung.com/)

- make "exfat" directory into {Kernel}/drivers/

- copy exFAT source code into {Kernel}/drivers/exfat/

- edit default kernel configuration file of your device.
$ vi {Kernel}/arch/arm/configs/{your_device_defconfig}
#NLS Setting
CONFIG_NLS_UTF8=y

- edit a Makefile in Kernel/drivers
$ vi {Kernel}/drivers/Makefile
obj-y += exfat/

- execute command to make for your Kernel. (see README_kernel.txt in Kernel)
	ex) make arch=arm defconfig
	make

