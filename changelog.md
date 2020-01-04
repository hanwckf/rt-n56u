##2020.01.03##
* 增加jq源码，如jq编译出现错误，请执行sudo apt-get install gcc-multilib
* 加入V2RAY单文件版,自动编译FLASH32M以上和16M无USB的固件会集成。THX:https://github.com/felix-fly/v2ray-openwrt
* 自己编译可修改build_firmware_modify中的CONFIG_FIRMWARE_INCLUDE_V2RAY=n/改成y。
* 自动编译增加D1和歌华链
* 适配NEWIFI-D1
* 适配歌华链(感谢群里Heaven适配与测试）

##2020.01.02##
* 增加SmartDNS源码，现在SmartDNS默认是编译进固件的(可选关闭),不再占用tmp空间。
* PSG1218,PSG1208,PSG712默认不编译进固件,由网上下载
* 修复B70 2.4G无线无法使用.(内核文件取自恩山 痴呆症的小白）

##往期更新日志##
*https://github.com/chongshengB/rt-n56u/blob/master/changelog-old.md