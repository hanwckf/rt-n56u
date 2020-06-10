# dns2tcp
一个 DNS 实用工具，用于将 DNS 查询从 UDP 模式转换为 TCP 模式。当然 pdnsd、dnsforwarder 也支持该功能，但是它们通常都有着较为繁杂的配置，而很多时候我们只是需要使用它们的 udp2tcp 功能而已，因此有了 `dns2tcp`。`dns2tcp` 设计的非常简洁以及易用，它不需要任何配置文件，直接在命令行参数中指定一个 **本地 UDP 监听地址** 以及一个 **远程 DNS 服务器地址**（该 DNS 服务器支持 TCP 查询）即可，没有任何多余的功能。

## 如何编译
```bash
git clone https://github.com/zfl9/dns2tcp
cd dns2tcp
make && sudo make install
```
dns2tcp 默认安装到 `/usr/local/bin/dns2tcp`，可安装到其它目录，如 `make install DESTDIR=/opt/local/bin`。<br>
交叉编译时只需指定 CC 变量，如 `make CC=aarch64-linux-gnu-gcc`（若报错，请先执行 `make clean`，然后再试）。

## 如何运行
```bash
dns2tcp -L"127.0.0.1#5353" -R"8.8.8.8#53"
```
- `-L` 选项指定本地监听地址，该监听地址接受 UDP 形式的 DNS 查询。
- `-R` 选项指定远程 DNS 服务器地址，该 DNS 服务器应支持 TCP 查询。
- 该例子中，dns2tcp 会将从 `127.0.0.1#5353` 地址收到 dns query 转换为 tcp 形式的 dns query，然后与 `8.8.8.8#53` 服务器建立 TCP 连接，连接建立后，会将此 dns query 发送给 `8.8.8.8#53`，然后等待 `8.8.8.8#53` 的 dns reply，收到完整 packet 后，将其转换为 udp 形式的 dns reply，最后将其发送给与之关联的请求客户端，并释放 TCP 连接及相关数据。

## 其它参数
```bash
usage: dns2tcp <-L listen> <-R remote> [-s syncnt] [-6rafvVh]
 -L <ip#port>            udp listen address, this is required
 -R <ip#port>            tcp remote address, this is required
 -s <syncnt>             set TCP_SYNCNT(max) for remote socket
 -6                      enable IPV6_V6ONLY for listen socket
 -r                      enable SO_REUSEPORT for listen socket
 -a                      enable TCP_QUICKACK for remote socket
 -f                      enable TCP_FASTOPEN for remote socket
 -v                      print verbose log, default: <disabled>
 -V                      print version number of dns2tcp and exit
 -h                      print help information of dns2tcp and exit
bug report: https://github.com/zfl9/dns2tcp. email: zfl9.com@gmail.com
```
`-s`：对`TCP`套接字启用`TCP_SYNCNT`选项，其值将影响`TCP`连接超时时间，但其并不等于超时时间，具体请谷歌。<br>
`-6`：对`UDP`套接字启用`IPV6_V6ONLY`选项，此选项与`IPv4-mapped IPv6 address`有关系，请谷歌以了解详细情况。<br>
`-r`：对`UDP`套接字启用`SO_REUSEPORT`选项，此选项在`Linux 3.9+`才有(打过相应内核补丁除外)，用于多进程负载均衡。<br>
`-a`：对`TCP`套接字启用`TCP_QUICKACK`选项，此选项在`Linux 2.4.4+`才有，和`TCP_NODELAY`(默认启用)类似但不完全相同。<br>
`-f`：对`TCP`套接字启用`TCP_FASTOPEN`选项，此特性在`Linux 3.7+`才有，若要`TFO`生效，需配置`net.ipv4.tcp_fastopen`。

Enjoy it!
