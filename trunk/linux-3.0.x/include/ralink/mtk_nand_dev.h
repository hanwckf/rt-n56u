#ifndef __MTK_NAND_DEV_H__
#define __MTK_NAND_DEV_H__

#define MTK_NAND_DRV_NAME		"MT7621-NAND"

#define NFI_CS_NUM			(1)
#define NFI_DEFAULT_CS			(0)

/* NAND driver params */
struct mtk_nand_host_hw {
	unsigned int nfi_bus_width;	/* NFI_BUS_WIDTH */
	unsigned int nfi_cs_num;	/* NFI_CS_NUM */
	unsigned int nfi_cs_id;		/* NFI_CS_ID */
};

#endif

