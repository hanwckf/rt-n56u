##2020.01.09##
* ssp+界面增加路由自身走代理开关，页面布局进行微调。
* smartdns增加黑白名单开关，上游服务器增加黑白名单过滤选项，页面布局进行重新分布
* frp增加web界面
* 增加mtd_write -r unlock mtd1重启方式，32M的路由可以前往系统管理-->设置里选择.(THX HIBOY)
* 删除YK-L1配置文件5G相关。
* 可能还会继续有更多.........请看源码的commit

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