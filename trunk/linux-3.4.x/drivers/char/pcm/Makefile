EXTRA_CFLAGS   +=-DSI321X 


SI321X_DIR = ./si321x
SI321X_OBJ = $(SI321X_DIR)/proslic_api/ralink/timer/proslic_timer_intf_linux.o \
			$(SI321X_DIR)/proslic_api/ralink/proslic_ppt_mb/si321x/proslic_spi_api.o \
			$(SI321X_DIR)/proslic_api/proslic.o	\
			$(SI321X_DIR)/proslic_api/si321x_intf.o	\
			$(SI321X_DIR)/pbx.o	\
			$(SI321X_DIR)/si321x_constants.o

SI3220_DIR = ./si3220_rev1.5
SI3220_OBJ = $(SI3220_DIR)/slic.o $(SI3220_DIR)/isr.o $(SI3220_DIR)/dual_i2o.o $(SI3220_DIR)/data.o \
			$(SI3220_DIR)/slic_data.o $(SI3220_DIR)/countries.o

		
obj-$(CONFIG_RALINK_PCM)  += ralink_pcm.o
ralink_pcm-objs       		:= pcm_ctrl.o ./codec_lib/codec_api.o $(SI3220_OBJ) $(SI321X_OBJ)

