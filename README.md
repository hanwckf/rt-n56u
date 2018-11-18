[![Build Status](https://travis-ci.org/hanwckf/rt-n56u.svg?branch=master)](https://travis-ci.org/hanwckf/rt-n56u)

# README #

Welcome to the rt-n56u project

This project aims to improve the rt-n56u and other supported devices on the software part, allowing power user to take full control over their hardware.
This project was created in hope to be useful, but comes without warranty or support. Installing it will probably void your warranty. 
Contributors of this project are not responsible for what happens next.

### How do I get set up? ###

* [Get the tools to build the system](https://bitbucket.org/padavan/rt-n56u/wiki/EN/HowToMakeFirmware) or [Download pre-built system image](https://bitbucket.org/padavan/rt-n56u/downloads)
* Feed the device with the system image file (Follow instructions of updating your current system)
* Perform factory reset
* Open web browser on http://my.router to configure the services

### Contribution guidelines ###

* To be completed

***

### 特别说明 ###
* 汉化字典来自 https://github.com/gorden5566/padavan

***

### 固件特点 ###
- 使用[gorden5566](https://github.com/gorden5566/padavan)的汉化字典
- [aria2](https://github.com/aria2/aria2)源码更新到1.17.1,预编译二进制文件为1.33.1 ```CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN```
- aria2前端更换为[AriaNg](https://github.com/mayswind/AriaNg)
- [curl](https://github.com/curl/curl)可选编译可执行程序```CONFIG_FIRMWARE_INCLUDE_CURL```
- 使用了[PROMETHEUS](http://pm.freize.net/index.html)提供的部分补丁，包括新版本的类库、软件包和WIFI驱动补丁```CONFIG_APPLY_PROMETHEUS_WIFI_DRIVER_PATCH```
- 使用了[Linaro1985/padavan-ng](https://github.com/Linaro1985/padavan-ng)的部分软件包
- 集成以下软件
>- [scutclient](https://github.com/hanwckf/scutclient) ```CONFIG_FIRMWARE_INCLUDE_SCUTCLIENT```
>- [gdut-drcom](https://github.com/chenhaowen01/gdut-drcom) ```CONFIG_FIRMWARE_INCLUDE_GDUT_DRCOM```
>- [dogcom](https://github.com/hanwckf/dogcom) ```CONFIG_FIRMWARE_INCLUDE_DOGCOM```
>- [minieap](https://github.com/hanwckf/minieap) ```CONFIG_FIRMWARE_INCLUDE_MINIEAP```
>- [njit-client](https://github.com/hanwckf/njit8021xclient) ```CONFIG_FIRMWARE_INCLUDE_NJIT_CLIENT```
>- [napt66](https://github.com/mzweilin/napt66) ```CONFIG_FIRMWARE_INCLUDE_NAPT66```
>- [ssr](https://github.com/shadowsocksr-backup/shadowsocksr-libev)/[ss](https://github.com/shadowsocks/shadowsocks-libev) ```CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS```
>- [ss-server](https://github.com/shadowsocks/shadowsocks-libev) ```CONFIG_FIRMWARE_INCLUDE_SSSERVER```
>- [softether-vpnclient](https://github.com/SoftEtherVPN/SoftEtherVPN_Stable) ```CONFIG_FIRMWARE_INCLUDE_SOFTETHERVPN_CLIENT```
>- [softether-vpncmd](https://github.com/SoftEtherVPN/SoftEtherVPN_Stable) ```CONFIG_FIRMWARE_INCLUDE_SOFTETHERVPN_CMD```
>- [dnsmasq-china-list](https://github.com/felixonmars/dnsmasq-china-list) ```CONFIG_FIRMWARE_INCLUDE_DNSMASQ_CHINA_CONF```
>- [dns-forwarder](https://github.com/aa65535/hev-dns-forwarder) ```CONFIG_FIRMWARE_INCLUDE_DNSFORWARDER```
>- [ChinaDNS](https://github.com/aa65535/ChinaDNS) ```CONFIG_FIRMWARE_INCLUDE_CHINADNS```
>- [vlmcsd](https://github.com/hanwckf/vlmcsd) ```CONFIG_FIRMWARE_INCLUDE_VLMCSD```
>- [ttyd](https://github.com/tsl0922/ttyd) ```CONFIG_FIRMWARE_INCLUDE_TTYD```
>- [lrzsz](https://ohse.de/uwe/software/lrzsz.html) ```CONFIG_FIRMWARE_INCLUDE_LRZSZ```
>- [htop](https://hisham.hm/htop/releases/) ```CONFIG_FIRMWARE_INCLUDE_HTOP```
>- [nano](https://www.nano-editor.org/dist/) ```CONFIG_FIRMWARE_INCLUDE_NANO```
>- [iperf](https://sourceforge.net/projects/iperf2) ```CONFIG_FIRMWARE_INCLUDE_IPERF```
>- [iperf3](https://iperf.fr/) ```CONFIG_FIRMWARE_INCLUDE_IPERF3```
>- [dump1090](https://github.com/hanwckf/dump1090) ```CONFIG_FIRMWARE_INCLUDE_DUMP1090```
>- [rtl-sdr](https://github.com/osmocom/rtl-sdr) ```CONFIG_FIRMWARE_INCLUDE_RTL_SDR```
>- [samba3.6](https://github.com/Linaro1985/padavan-ng/tree/master/trunk/user/samba36) ```CONFIG_FIRMWARE_INCLUDE_SMBD36```
>- [mtr](https://github.com/traviscross/mtr) ```CONFIG_FIRMWARE_INCLUDE_MTR```
- 已适配除官方适配外的以下机型
>- WR1200JS (128M,USB)
>- NEWIFI3(D2) (512M,USB)
>- K2 / PSG1218 (64M)
>- K2P (128M,杂交固件)
>- K2P_DRV/K2P_nano (128M)
>- MZ-R13 (64M)
>- MZ-R13P (未测试,64M)
>- OYE-001 (128M,USB)
>- MI-MINI (未测试,128M,USB)
>- MI-3 (未测试,128M,USB)
>- 5K-W20 (未测试,64M,USB)

***

### 编译说明 ###

* 安装依赖包
```shell
sudo apt-get update
sudo apt-get install unzip libtool curl cmake gperf gawk flex bison nano \
git python-docutils gettext automake autopoint texinfo build-essential \
pkg-config zlib1g-dev libgmp3-dev libmpc-dev libmpfr-dev libncurses5-dev libltdl-dev
```
* 克隆源码
```shell
git clone --depth=1 https://gitee.com/hanwckf/rt-n56u.git /opt/rt-n56u
#git clone --depth=1 https://github.com/hanwckf/rt-n56u.git /opt/rt-n56u
```
* 编译工具链
```shell
cd /opt/rt-n56u/toolchain-mipsel
./clean_sources
./build_toolchain_3.4.x
```
* (可选)修改机型配置文件
```shell
nano /opt/rt-n56u/trunk/configs/templates/PSG1218.config
```
* 清理代码树并开始编译
```shell
cd /opt/rt-n56u/trunk
sudo ./clear_tree
sudo ./build_firmware_modify PSG1218
#脚本第一个参数为路由型号，在trunk/configs/templates/中
#编译好的固件在trunk/images里
```

***

### 请参阅 ###
- https://www.jianshu.com/p/cb51fb0fb2ac
- https://www.jianshu.com/p/d76a63a12eae
- https://www.jianshu.com/p/52282cd07284
- https://www.jianshu.com/p/6b8403cdea46
