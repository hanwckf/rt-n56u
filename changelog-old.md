##2020.02.23##
* 增加定时重启功能,入口在系统管理页面
* 更改ssp绕过大陆模式使用pdnsd进行解析,加载cdn域名走国内dns(此功能和smartdns不能同时工作)
* 去除adbyby plus adb规则，改成加载hosts规则
* 同步最新smartdns源码
* 更改了一下koolproxy脚本，看还会不会出现重启无法启动的问题，如果还不行就重写脚本吧。
* 更改开机插件启动统一由脚本控制，检测是否联网再进行启动
* 加入AdguardHome(从github上下载运行文件)
* web界面一些微调

##2020.02.09##
* 更改KP完整规则的下载地址
* 同步hanwckf的最新源码
* 调整固件的版本信息由自动生成日期改成固定日期

##2020.01.23##
* 应群友建议,增加无线弱信号剔除功能
* 继续修复SSP自动切换
* 调整修改ssp web页面的一些小问题
* 修正ssp和smartdns页面增加服务器页面跳转的问题
* webdav增加自定义用户名和密码
* 升级smartdns源码
* 调整了固件版本命名:改成日期

##2020.01.20##
* 应群友建议,增加caddy filebrowser和WEBDAV(文件来自ntgeralt）
* SSP增加trojan
* 修复京东云NTFS和MMC自动挂载
* 修复在源码更改了路由默认IP后，编译出来的固件恢复出厂IP跳转错误的问题

##2020.01.16##
* 增修复ssp+故障切换问题
* 增加京东云路由适配(来自Lintel)

##2020.01.11##
* 增加koolproxy
* 修复ssp+故障切换问题
* 修复frp重启无效问题，加了守护
* 修复用mtd_write -r unlock mtd1重启方式,脚本无法保存的问题。
* 修改控制台reboot命令逻辑，选择mtd_write -r unlock mtd1重启方式后，输入reboot也是用这种方式重启,方便定时重启的人使用

##2020.01.09##
* ssp+界面增加路由自身走代理开关，页面布局进行微调。
* smartdns增加黑白名单开关，上游服务器增加黑白名单过滤选项，页面布局进行重新分布
* frp增加web界面
* 增加mtd_write -r unlock mtd1重启方式，32M的路由可以前往系统管理-->设置里选择.(THX HIBOY)
* 删除YK-L1配置文件5G相关。

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

##2019.12.31##
* 修复路由重启后aliddns不运行的问题
* SSP+增加链接导入功能，修复web界面一些显示问题
* 适配B70(感谢Untitled提供荒野无灯的适配文件)
* 适配JCG-AC856M(感谢群里的旅途中的我适配和测试,gpio值还未完全适配，但不影响使用)
* 适配JCG-AC836M(感谢群里的碧霄客修改和测试)
* 适配YK-L1(L1、L1C、L1W通刷)

##2019.12.26##
* 按照目前收到的反馈，继续修复、修改SSP+
* 新增了SSP+内网控制
* 此次更新建议重置下(/etc/storage)
* PS:因为此固件只有我自己修改和测试，所以难免会有一些问题没有遇到，请多多反馈给我。

##2019.12.23##
* 1.同步最新源码
* 2.适配PSG712
* 3.修复adbyby自定义规则无法下载的问题