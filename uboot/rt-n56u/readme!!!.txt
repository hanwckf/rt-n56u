                          ASUS RT-N56U Uboot

                              WARNING

- Please use this Uboot only for ASUS RT-N56U board!!!
- ASUS RT-N56U used "ROM mode" Uboot, flash uboot.bin file only (not uboot.trx)!!!
- Do not remove power supply during flash Uboot!!!
- Your ASUS RT-N56U may be bricked due to your incorrect actions!!!


                         FLASH INSTRUCTIONS

- Upload uboot_n56u.bin file to router's /tmp dir (e.g. via WinSCP).
- Flash Uboot via SSH or Telnet console (flash duration ~5 sec):
----------------------------------------------
mtd_write write /tmp/uboot_n56u.bin Bootloader
----------------------------------------------
- Reboot router.


                          LIST OF CHANGES

1.0.0.8:
----------------------------------------------------------
- Added NVRAM erase feature by hold WPS button at power-up.
  Usage:
  Power-up RT-N56U with pressed and holded WPS button, until
  LED Power is blinking. After release WPS button, NVRAM will be
  erased and RT-N56U is restarted.
- Improved RTL8367M initialization (prevent WAN<->LAN spoofing).
- Allowed firmware restoration from WAN port (in Emergency mode).
- Disabled RTL8367M noisy debug out.




-
Padavan
04/15/2014
