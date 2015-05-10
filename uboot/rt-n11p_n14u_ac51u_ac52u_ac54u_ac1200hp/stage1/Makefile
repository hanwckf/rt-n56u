sinclude $(TOPDIR)/config.mk
-include $(TOPDIR)/.config

ifdef RT2880_FPGA_BOARD
FADDR = 0xBC400000
endif
ifdef RT2880_ASIC_BOARD 
FADDR = 0xBC400000
endif
ifdef RT2883_FPGA_BOARD
FADDR = 0xBC000000
endif
ifdef RT2883_ASIC_BOARD 
FADDR = 0xBC000000
endif
ifdef RT3052_ASIC_BOARD 
FADDR = 0xBFC00000
endif
ifdef RT3052_FPGA_BOARD
FADDR = 0xBFC00000
endif

SRC := ./start.S ./boot.c ./nandc.c ./gdma.c
OBJS = $(notdir $(addsuffix .o, $(basename $(SRC))))
CFLAGS += -DCONFIG_STAGE2_OFFSET=0x4000

all: uboot.img

stage2.bin:
	../tools/mkimage -A $(ARCH) -T standalone -C none -a $(TEXT_BASE) \
	-e $(shell readelf -h ../u-boot | grep "Entry" | awk '{print $$4}') \
	-n "$(shell echo $(CFG_ENV_IS) | sed -e 's/IN_//') Flash Image" -d \
	../uboot.bin $@

stage2.o: stage2.bin
	echo 'char *stage1_ww="ww:' `date` `whoami` '";' | $(CC) $(CFLAGS) -c -o $@ -xc -
	$(OBJCOPY) --add-section .stage2=$< $@
	$(OBJCOPY) --set-section-flags .stage2=code,contents,alloc,readonly $@

stage1n2.elf: $(OBJS) stage2.o
	$(LD) -T $(@:.elf=.lds) -Ttext $(FADDR) $^ -Map $(@:.elf=.map) -o $@

uboot.img: stage1n2.elf
	$(OBJCOPY) $(OBJCFLAGS) -O binary -v $< $@
	@ls -al $@
	dd if=$@ bs=4096 count=1 of=$@ seek=1 conv=notrunc
	dd if=$@ bs=4096 count=2 of=$@ seek=2 conv=notrunc
	@ls -al $@
	@mv -f $@ ../$@

.PHONY: clean
clean:
	rm -f $(OBJS) stage2.bin stage2.o stage1n2.elf stage1n2.map


#########################################################################

.depend: Makefile $(SRC) 
	$(CC) -M $(CFLAGS) $(SRC) > $@

sinclude .depend
#########################################################################

