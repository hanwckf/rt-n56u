EXTRA_CFLAGS += -Idrivers/char/pcm/proslic_api/inc -Idrivers/char/pcm/proslic_api/custom
EXTRA_CFLAGS += -Idrivers/char/pcm/proslic_api/example_drivers/ralink/proslic_ppt_mb
EXTRA_CFLAGS += -Idrivers/char/pcm/proslic_api/example_drivers/ralink/timer

SI3220_DIR = ./si3220_rev1.5
SI3220_OBJ = $(SI3220_DIR)/slic.o $(SI3220_DIR)/isr.o $(SI3220_DIR)/dual_i2o.o $(SI3220_DIR)/data.o \
			$(SI3220_DIR)/slic_data.o $(SI3220_DIR)/countries.o

SI321X_DIR = ./si321x
SI321X_OBJ = $(SI321X_DIR)/proslic_api/ralink/timer/proslic_timer_intf_linux.o \
			$(SI321X_DIR)/proslic_api/ralink/proslic_ppt_mb/si321x/proslic_spi_api.o \
			$(SI321X_DIR)/proslic_api/proslic.o	\
			$(SI321X_DIR)/proslic_api/si321x_intf.o	\
			$(SI321X_DIR)/pbx.o	\
			$(SI321X_DIR)/si321x_constants.o
#EXTRA_CFLAGS   +=-DSI321X 
			
CONFIG_SLIC_SI322X := y
ifeq ($(CONFIG_SLIC_SI322X),y)
EXTRA_CFLAGS += -DSI322X
endif


CUSTOM_DIR = ./proslic_api/custom
CUSTOM_OBJ = \
			$(CUSTOM_DIR)/vdaa_constants.o

ifeq ($(CONFIG_SLIC_SI322X),y)
CUSTOM_OBJ += $(CUSTOM_DIR)/si3226_constants.o
CUSTOM_OBJ += $(CUSTOM_DIR)/si3226_patch_C_FLBK_2008NOV20.o
#CUSTOM_OBJ += $(CUSTOM_DIR)/si3226_patch_D_FLBK_2008NOV20.o
#CUSTOM_OBJ += $(CUSTOM_DIR)/si3226_patch_E_FLBK_2010JAN06.o
#CUSTOM_OBJ += $(CUSTOM_DIR)/si3226_patch_C_QCUK_2008NOV20.o
CUSTOM_OBJ += $(CUSTOM_DIR)/si3226_patch_D_QCUK_2008NOV20.o
CUSTOM_OBJ += $(CUSTOM_DIR)/si3226_patch_E_FLBK_2010JAN06.o
endif

ifeq ($(CONFIG_SLIC_SI321X),y)
CUSTOM_OBJ += $(CUSTOM_DIR)/si321x_constants.o 
endif

ifeq ($(CONFIG_SLIC_SI324X),y)
CUSTOM_OBJ += $(CUSTOM_DIR)/si324x_constants.o 
CUSTOM_OBJ += $(CUSTOM_DIR)/si324x_patch_D_2008JUN23.o 
endif
			
SRC_DIR = ./proslic_api/src
SRC_OBJ = $(SRC_DIR)/si_voice_version.o $(SRC_DIR)/vdaa.o \
			$(SRC_DIR)/proslic.o $(SRC_DIR)/si_voice.o

ifeq ($(CONFIG_SLIC_SI322X),y)
SRC_OBJ += $(SRC_DIR)/si3226_intf.o 
endif

ifeq ($(CONFIG_SLIC_SI324X),y)
SRC_OBJ += $(SRC_DIR)/si324x_intf.o 
endif

ifeq ($(CONFIG_SLIC_SI321X),y)
SRC_OBJ += $(SRC_DIR)/si321x_intf.o 
endif

ifeq ($(CONFIG_SLIC_SI322X),y)			
RALINK_DIR = ./proslic_api/example_drivers/ralink/proslic_ppt_mb/si3226
endif

ifeq ($(CONFIG_SLIC_SI324X),y)			
RALINK_DIR = ./proslic_api/example_drivers/ralink/proslic_ppt_mb/si324x
endif

ifeq ($(CONFIG_SLIC_SI321X),y)			
RALINK_DIR = ./proslic_api/example_drivers/ralink/proslic_ppt_mb/si321x
endif

RALINK_OBJ = $(RALINK_DIR)/proslic_spi_api.o
TIMER_DIR = ./proslic_api/example_drivers/ralink/timer
RALINK_OBJ += $(TIMER_DIR)/proslic_timer_intf_linux.o ./proslic_api/slic_init.o
			
obj-$(CONFIG_RALINK_PCM)  += ralink_pcm.o

ralink_pcm-y       		:= pcm_ctrl.o ./codec_lib/codec_api.o $(SI3220_OBJ)
#$(SI321X_OBJ)

ifeq ($(CONFIG_SLIC_SI322X),y)
ralink_pcm-y += $(SRC_OBJ) $(CUSTOM_OBJ) $(RALINK_OBJ)
endif

						#$(perf_obj)
