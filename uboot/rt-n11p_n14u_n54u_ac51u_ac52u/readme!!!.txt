                 ASUS RT-N11P/N14U/N54U/AC51U/AC52U Uboot

                              WARNING

- Please use this Uboot only for ASUS RT-N11P/N14U/N54U/AC51U/AC52U boards!!!
- ASUS RT-N11P/N14U/N54U/AC51U boards used "ROM mode" Uboot, flash uboot.bin
  file only (not uboot.trx)!!!
- ASUS RT-AC52U board used "RAM mode" Uboot, flash uboot_stage2.trx file only (not uboot.bin)!!!
- Do not remove power supply during flash Uboot!!!
- Your device may be bricked due to your incorrect actions!!!


                          BUILD INSTRUCTIONS

NOTE: uboot_n11p.bin and uboot_n14u.bin is already builded and tested for RT-N11P/N14U.

- Copy appropriate file (e.g. config_n11p) to .config.
- Run make menuconfig, choose [Exit] and confirm Save.
- Run make.
- Use image file uboot.bin for RT-N11P/N14U/N54U/AC51U boards.
- Use image file uboot_stage2.trx for RT-AC52U board.


                          FLASH INSTRUCTIONS

- Upload appropriate file (e.g. uboot_n11p.bin) to router's /tmp dir (e.g. via WinSCP).
- Check Uboot image checksum and compare with uboot_XXXX.md5:
----------------------------------------------
md5sum /tmp/uboot_n11p.bin
----------------------------------------------
- Flash checked Uboot via SSH or Telnet console (flash duration ~5 sec):
----------------------------------------------
mtd_write write /tmp/uboot_n11p.bin Bootloader
----------------------------------------------
- Reboot router.


                            LIST OF CHANGES


1.0.0.1:
----------------------------------------------------------
- Early disabled PHY ports link (prevent spoofing).
- Fixed RT-N11P switch VLAN isolation (prevent spoofing).
- Fixed RT-N14U GPIO #2 initialization.
- Configured RT-N11P CPU Clock to 600MHz (native for SDR).
- Enabled SPI DOR mode (opcode 0x3B) - firmware load & boot faster.
- Disabled SPI debug out.
- Decrease boot delay to 1 sec.




-
Padavan
12/06/2014
