                          ASUS RT-N65U Uboot

                              WARNING

- Please use this Uboot only for ASUS RT-N65U board!!!
- ASUS RT-N65U used "RAM mode" Uboot, flash uboot.trx file only (not uboot.bin)!!!
- Do not remove power supply during flash Uboot!!!
- Your ASUS RT-N65U may be bricked due to your incorrect actions!!!


                          FLASH INSTRUCTIONS

- Upload uboot_n65u.trx file to router's /tmp dir (e.g. via WinSCP).
- Check Uboot image checksum and compare with uboot_n65u.md5:
----------------------------------------------
md5sum /tmp/uboot_n65u.trx
----------------------------------------------
- Flash checked Uboot via SSH or Telnet console (flash duration ~5 sec):
----------------------------------------------
mtd_write write /tmp/uboot_n65u.trx Bootloader
----------------------------------------------
- Reboot router.


                            LIST OF CHANGES


1.0.0.9:
----------------------------------------------------------
- SPI clock 40MHz instead of 20MHz - firmware boot faster.
- Fixed GMAC1 initialization.
- Fixed RTL8367RB initialization.
- Disabled RTL8367RB noisy debug out.





-
Padavan
04/15/2014
