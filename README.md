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
- [curl](https://github.com/curl/curl.git)更新到7.58.0,可选编译可执行文件```CONFIG_FIRMWARE_INCLUDE_CURL```
- aria2源码更新到1.17.1,预编译二进制文件为1.33.1 ```CONFIG_FIRMWARE_INCLUDE_ARIA2_NEW_PREBUILD_BIN```
- aria2前端更换为[aria-ng](https://github.com/mayswind/AriaNg.git) (0.4.0)
- 集成以下软件
>- [scutclient](https://github.com/hanwckf/scutclient.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_SCUT_MOD```
>- [ttyd](https://github.com/tsl0922/ttyd.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_TTYD```
>- [vlmcsd](https://github.com/hanwckf/vlmcsd.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_VLMCSD```
>- [napt66](https://github.com/mzweilin/napt66.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_NAPT66```
>- [dns-forwarder](https://github.com/aa65535/hev-dns-forwarder.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_DNSFORWARDER```
>- [ChinaDNS](https://github.com/aa65535/ChinaDNS.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_CHINADNS```
>- [dnsmasq-china-list](https://github.com/felixonmars/dnsmasq-china-list.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_DNSMASQ_CHINA_CONF```
>- [ssr](https://github.com/shadowsocksr-backup/shadowsocksr-libev.git)(含webui) ```CONFIG_FIRMWARE_INCLUDE_SHADOWSOCKS```
>- [lrzsz](https://ohse.de/uwe/software/lrzsz.html) ```CONFIG_FIRMWARE_INCLUDE_LRZSZ```
>- [htop](https://hisham.hm/htop/releases/) ```CONFIG_FIRMWARE_INCLUDE_HTOP```
>- [nano](https://www.nano-editor.org/dist/) ```CONFIG_FIRMWARE_INCLUDE_NANO```
>- [gdut-drcom](https://github.com/chenhaowen01/gdut-drcom.git) ```CONFIG_FIRMWARE_INCLUDE_GDUT_DRCOM```
>- [dogcom](https://github.com/mchome/dogcom.git) ```CONFIG_FIRMWARE_INCLUDE_DOGCOM```
>- [minieap](https://github.com/hanwckf/minieap.git) ```CONFIG_FIRMWARE_INCLUDE_MINIEAP```
>- [njit-client](https://github.com/hanwckf/njit8021xclient.git) ```CONFIG_FIRMWARE_INCLUDE_NJIT_CLIENT```
- 已适配除官方适配外的以下机型
>- K2 / PSG1218 (64M)
>- OYE-001 (128M,USB)
>- MI-MINI (128M,USB)
>- 5K-W20 (64M,USB)

***

### 编译说明 ###

* 安装依赖包
```shell
sudo apt-get update
sudo apt-get install unzip libtool curl cmake gperf gawk flex bison nano \
git python-docutils gettext automake autopoint texinfo build-essential \
pkg-config zlib1g-dev libgmp3-dev libmpc-dev libmpfr-dev libncurses5-dev
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
- https://www.jianshu.com/p/52282cd07284
