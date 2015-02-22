utils_dir_y =
utils_dir_  =

# ralink specific applications
utils_dir_y			+= hw_nat

# other utils
utils_dir_y			+= cpu_usage

# ESW
ifdef CONFIG_RAETH_ESW
utils_dir_y			+= switch
else
ifdef CONFIG_MT7530_GSW
utils_dir_y			+= switch
endif
endif

# hub-ctrl
ifdef CONFIG_USB_SUPPORT
utils_dir_y			+= hub-ctrl
endif

all romfs:
	for i in $(utils_dir_y) ; do make -C $$i $@ || exit $?; done

clean:
	for i in `ls -d *` ; do \
		[ ! -d $$i ] || \
		$(MAKE) -C $$i clean ; \
	done
