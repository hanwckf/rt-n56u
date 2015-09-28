                Ralink/MediaTek U-Boot for MIPS SoC
          RT3052/RT3352/RT3883/RT5350/MT7620/MT7621/MT7628
                  Based on MediaTek SDK 5.0.0.0

                          LIST OF CHANGES

5.0.0.1:
----------------------------------------------------------
- Original MTK SDK 5.0.0.0 codebase.
- Added GPIO control module (Buttons/LEDs).
- Added TFTP server from ASUS (support TFTP client and ASUS Firmware Restoration).
- Added Realtek RTL8367 GSW driver.
- Fixed FE/ESW/GSW initialization.
- Fixed brick bug on update U-Boot via TFTP and TFTP transfer break.
- Fixed erase bug in MT7621 NAND code (infinity loop and erase all chip).
- Improved MT7621 NAND I/O performance (up to 3x).
- Shrinked MT7621 NAND U-Boot image size to enough one NAND block (131072).
- Support MT7621 NAND partitions configuration via profile.
- Support SPI clock and SPI DOR mode configuration via profile.
- Support Buttons and LEDs configuration via profile.
- Support EPHY ports on/off configuration via profile.


                            BUILD TOOLS

For MT7621 U-Boot:
- extract 'tools/mips-2012.03.tar.bz2' to /opt

For RT3XXX/MT7620/MT7628 U-Boot:
- extract 'tools/buildroot-gcc342.tar.bz2' to /opt

Both toolchains require x86 (32-bit) Linux environment.


                         BUILD INSTRUCTIONS

- Copy appropriate '.config' file (e.g. profiles/asus_rt-n11p/.config)
  to 'uboot-5.x.x.x' dir.
- Goto 'uboot-5.x.x.x' dir.
- Run 'make menuconfig', choose [Exit] and confirm [Save]. This is important step!
- Run 'make'.
- Use image file uboot.bin (ROM mode) for NOR and SPI-flash boards.
- Use image file uboot.img (RAM mode) for NAND-flash (or RT3XXX SPI-flash) boards 

To clean U-Boot tree:
- Run 'make clean'.
- Run 'make unconfig'.

NOTE:
1. U-Boot images for most ASUS devices is already builded.
2. U-Boot is configured for UART baud rate 115200.
3. All profiles has disabled option "Enable all Ethernet PHY" to prevent LAN-WAN
   spoofing (EPHY will be enabled later in FW logic). To force enable EPHY (e.g. for
   use OpenWRT/PandoraBox), select option "Enable all Ethernet PHY".


                         FLASH INSTRUCTIONS

- Upload appropriate U-Boot image file to router's /tmp dir (e.g. via WinSCP).
- Check U-Boot image checksum and compare with uboot.md5:

md5sum /tmp/uboot.bin

- Flash checked U-Boot via SSH or Telnet console (flash duration ~3 sec):

mtd_write write /tmp/uboot.bin Bootloader

- Reboot router.

                             WARNING

- Do not remove power supply during flash U-Boot!!!
- Device may be bricked due to your incorrect actions!!!


                             FEATURES

1. Press and hold RESET button on PowerUp: switch to recovery mode. Use TFTP
   client or ASUS Firmware Restoration (device IP-address is 192.168.1.1).
2. Press and hold WPS button on PowerUp: erase 'Config' partition (U-Boot Env and NVRAM)
   and self-reboot.

NOTE:
- U-Boot perform auto-switch to recovery mode when flash image integrity fail.
- Alert LED(s) is blinked in recovery mode and on TFTP operations.





-
Padavan
09/29/2015
