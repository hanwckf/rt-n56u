DriverVersion="V4.4.2.0"
ModulePrefix="MT7615_LinuxAP"
ChipName="mt7615"
WiFiMode=AP
package_name=$ModulePrefix\_$DriverVersion

if [ -d wifi_driver ]; then

if [ -d release ]; then
	rm release -Rf
fi

if [ -d mt_wifi ]; then
	rm mt_wifi -Rf
fi

if [ -d mt_wifi_ap ]; then
	rm mt_wifi_ap -Rf
fi

	## clean the garbage files
	cd wifi_driver/embedded
	####### Remove unwanted files that do not processed by strip tool ####### // TODO
	rm os/linux/Makefile.libautoprovision.6 common/rt2860.bin common/rt2870_sw_ch_offload.bin common/RT85592.bin
	rm tools/mt7662e_ap.sh tools/mt7662e_sta.sh tools/i.sh tools/trace.sh
	rm ../eeprom/MT7601*.bin ../eeprom/MT7603*.bin ../eeprom/MT7628*.bin ../eeprom/MT7637*.bin ../eeprom/MT7662*.bin
	rm ../eeprom/MT7636*.bin
	rm ../mcu/bin/*_FPGA*.bin ../mcu/bin/*_plain*.bin ../mcu/bin/*_test*.bin
	rm ../mcu/bin/*7601* ../mcu/bin/*7603* ../mcu/bin/*7612* ../mcu/bin/*7636* ../mcu/bin/*7637*
	rm ../mcu/bin/*7610* ../mcu/bin/*7650* ../mcu/bin/*7662* ../mcu/bin/*7628*
	rm ../os/linux/Kconfig.ap* ../os/linux/Kconfig.sta* ../os/linux/Kconfig.rlt* ../os/linux/Kconfig.wifi
	rm ../os/linux/Makefile.ap* ../os/linux/Makefile.sta* ../os/linux/Makefile.2880* ../os/linux/Makefile.rlt*
	#########################################################################
	
	make build_tools CHIPSET=$ChipName
	## Regenerate SKU tables ##
	make build_sku_tables CHIPSET=$ChipName
	make release WIFI_MODE=$WiFiMode CHIPSET=$ChipName

	mv DPA ../../mt_wifi
	
	cd ../../
	mkdir -p mt_wifi_ap
    #mkdir -p mt_wifi_sta
	
    cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_ap ./mt_wifi_ap/Kconfig
    cp -a  wifi_driver/os/linux/Makefile.mt_wifi_ap ./mt_wifi_ap/Makefile
    #cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_sta ./mt_wifi_sta/Kconfig
    #cp -a  wifi_driver/os/linux/Makefile.mt_wifi_sta ./mt_wifi_sta/Makefile
    cp -a wifi_driver/os/linux/Kconfig.mt_wifi wifi_driver/embedded/Kconfig

	rm mt_wifi/embedded/*auto_build*	

	mkdir -p release/
	tar -jcvf  release/$package_name\.tar.bz2 mt_wifi mt_wifi_ap
	
else
    exit 1
fi
