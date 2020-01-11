##2020.01.11##
* 增加koolproxy
* 修复ssp+故障切换问题
* 修复frp重启无效问题，加了守护
* 修复用mtd_write -r unlock mtd1重启方式,脚本无法保存的问题。
* 修改控制台reboot命令逻辑，选择mtd_write -r unlock mtd1重启方式后，输入reboot也是用这种方式重启,方便定时重启的人使用
* 可能还会继续有更多.........请看源码的commit

##往期更新日志##
*https://github.com/chongshengB/rt-n56u/blob/master/changelog-old.md