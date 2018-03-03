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

### 编译说明 ###
* 安装依赖包
```shell
sudo apt-get update
sudo apt-get install libtool curl cmake gperf gawk flex bison nano \
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
vi /opt/rt-n56u/trunk/configs/templates/PSG1218.config
```
* 清理代码树并开始编译
```shell
cd /opt/rt-n56u/trunk
sudo ./clear_tree
sudo ./build_firmware_modify PSG1218
#编译好的固件在trunk/images里
```

### 特别说明 ###
* 汉化字典来自 https://github.com/gorden5566/padavan
