# 广告过滤 访问控制功能

# 内网(LAN)访问控制的默认代理转发设置，
#    0  默认值, 常规, 未在以下设定的 内网IP 根据 AD配置工作模式 走 AD
#    1         全局, 未在以下设定的 内网IP 使用全局代理 走 AD
#    2         绕过, 未在以下设定的 内网IP 不使用 AD
AD_LAN_AC_IP=0
nvram set AD_LAN_AC_IP=$AD_LAN_AC_IP
# =========================================================
# 内网(LAN)IP设定行为设置, 格式如 b,192.168.1.23, 多个值使用空格隔开
#   使用 b/g/n 前缀定义主机行为模式, 使用英文逗号与主机 IP 分隔
#   b: 绕过, 此前缀的主机IP 不使用 AD
#   g: 全局, 此前缀的主机IP 忽略 AD配置工作模式 使用全局代理 走 AD
#   n: 常规, 此前缀的主机IP 使用 AD配置工作模式 走 AD
#   s: https, 此前缀的主机IP 使用 AD配置工作模式 https走 AD
#   优先级: 绕过 > 全局 > 常规
# （如多个设置则每一个ip一行,可选项：删除前面的#可生效）
cat > "/tmp/ad_spec_lan_DOMAIN.txt" <<-\EOF
#b,192.168.123.115
#g,192.168.123.116
#n,192.168.123.117
#s,192.168.123.118
#b,099B9A909FD9
#s,099B9A909FD9
#g,A9:CB:3A:5F:1F:C7



EOF
# =========================================================

# adbyby加载第三方adblock规则 0关闭；1启动（可选项：删除前面的#可生效）
#【不建议启用第三方规则,有可能破坏规则导致过滤失效】
nvram set adbyby_adblocks=1
adblocks=`nvram get adbyby_adblocks`
cat > "/tmp/rule_DOMAIN.txt" <<-\EOF
# 【可选多项，会占用内存：删除前面的#可生效，前面添加#停用规则】
# https://easylist-downloads.adblockplus.org/easylistchina.txt


EOF

