# ipt2socks
类似 [redsocks](https://github.com/darkk/redsocks)、[redsocks2](https://github.com/semigodking/redsocks) 的实用工具，用于将 iptables(REDIRECT/TPROXY) 流量转换为 socks5(tcp/udp) 流量。除此之外，ipt2socks 不提供任何非必要的功能（即：KISS 原则，`keep it simple, stupid`，保持简单和愚蠢）。ipt2socks 可以为仅支持 socks5 传入协议的“本地代理”提供 **iptables 透明代理** 传入协议的支持，比如 ss/ssr 的 ss-local/ssr-local、v2ray 的 socks5 传入协议、trojan 的 socks5 客户端等等。

**TODO: libuv-based 版本实现质量不佳，bug 过多，无力维护，已打算使用 libev 进行完全重构。**

## 简要说明
- IPv4 和 IPv6 双栈支持，支持 **纯 TPROXY** 透明代理模式，专为 [ss-tproxy](https://github.com/zfl9/ss-tproxy) 而写。
- TCP 透明代理提供 REDIRECT、TPROXY 两种方式，UDP 透明代理为 TPROXY 方式。
- UDP 透明代理支持 Full Cone NAT，前提是后端的 socks5 服务器支持 Full Cone NAT。
- 多线程 + SO_REUSEPORT 端口重用，每个线程运行各自独立的事件循环，性能提升显著。

## 如何编译
**动态链接 libuv**：适用于本地编译，使用包管理器安装 [libuv](https://github.com/libuv/libuv) 依赖库即可（如 `yum install libuv-devel`）：
```bash
git clone https://github.com/zfl9/ipt2socks
cd ipt2socks
make && sudo make install
```
ipt2socks 默认安装到 `/usr/local/bin/ipt2socks`，可安装到其它目录，如 `make install DESTDIR=/opt/local/bin`。

**静态链接 libuv**：适用于交叉编译，此方式编译出来的 `ipt2socks` 不依赖任何第三方库，可直接拷贝到目标系统运行：
```bash
# 进入某个目录
cd /opt

# 获取 libuv 源码包
libuv_version="1.32.0" # 定义 libuv 版本号
wget https://github.com/libuv/libuv/archive/v$libuv_version.tar.gz -Olibuv-$libuv_version.tar.gz
tar xvf libuv-$libuv_version.tar.gz

# 进入源码目录，编译
cd libuv-$libuv_version
./autogen.sh
./configure --prefix=/opt/libuv --enable-shared=no --enable-static=yes CC="gcc -O3"
make && sudo make install
cd ..

# 获取 ipt2socks 源码
git clone https://github.com/zfl9/ipt2socks

# 进入源码目录，编译
cd ipt2socks
make INCLUDES="-I/opt/libuv/include" LDFLAGS="-L/opt/libuv/lib" && sudo make install
```

## 如何运行
```bash
# -s 指定 socks5 服务器 ip
# -p 指定 socks5 服务器端口
ipt2socks -s 127.0.0.1 -p 1080
```
> ipt2socks 启动后，配置相应的 iptables 规则即可。这里就不详细介绍了，有兴趣的请戳 [ss-tproxy](https://github.com/zfl9/ss-tproxy)。

**全部参数**
```bash
$ ipt2socks --help
usage: ipt2socks <options...>. the existing options are as follows:
 -s, --server-addr <addr>           socks5 server ip address, <required>
 -p, --server-port <port>           socks5 server port number, <required>
 -a, --auth-username <user>         username for socks5 authentication
 -k, --auth-password <passwd>       password for socks5 authentication
 -b, --listen-addr4 <addr>          listen ipv4 address, default: 127.0.0.1
 -B, --listen-addr6 <addr>          listen ipv6 address, default: ::1
 -l, --listen-port <port>           listen port number, default: 60080
 -j, --thread-nums <num>            number of worker threads, default: 1
 -n, --nofile-limit <num>           set nofile limit, maybe need root priv
 -o, --udp-timeout <sec>            udp socket idle timeout, default: 300
 -c, --cache-size <size>            max size of udp lrucache, default: 256
 -f, --buffer-size <size>           buffer size of tcp socket, default: 8192
 -u, --run-user <user>              run the ipt2socks with the specified user
 -G, --graceful                     gracefully close the tcp connection pair
 -R, --redirect                     use redirect instead of tproxy (for tcp)
 -T, --tcp-only                     listen tcp only, aka: disable udp proxy
 -U, --udp-only                     listen udp only, aka: disable tcp proxy
 -4, --ipv4-only                    listen ipv4 only, aka: disable ipv6 proxy
 -6, --ipv6-only                    listen ipv6 only, aka: disable ipv4 proxy
 -v, --verbose                      print verbose log, default: <disabled>
 -V, --version                      print ipt2socks version number and exit
 -h, --help                         print ipt2socks help information and exit
```
- -s 选项指定 socks5 服务器的监听地址。
- -p 选项指定 socks5 服务器的监听端口。
- -a 选项指定 socks5 服务器的认证用户。
- -k 选项指定 socks5 服务器的认证密码。
- -b 选项指定 ipt2socks 的 IPv4 监听地址。
- -B 选项指定 ipt2socks 的 IPv6 监听地址。
- -l 选项指定 ipt2socks 的透明代理监听端口。
- -j 选项指定 ipt2socks 的线程数，默认为 1。
- -n 选项设置 ipt2socks 进程的 nofile 限制值。
- -o 选项设置 ipt2socks 的 UDP 空闲超时(秒)。
- -c 选项设置 ipt2socks 的 UDP 缓存最大大小。
- -f 选项设置 ipt2socks 的 TCP 接收缓冲区大小。
- -u 选项设置 ipt2socks 的用户ID，`run_as_user`。
- -G 选项指示 ipt2socks 优雅地关闭 TCP 代理连接对。
- -R 选项指示 ipt2socks 使用 REDIRECT 而非 TPROXY。
- -T 选项指示 ipt2socks 仅启用 TCP 透明代理监听端口。
- -U 选项指示 ipt2socks 仅启用 UDP 透明代理监听端口。
- -4 选项指示 ipt2socks 仅启用 IPv4 协议栈的透明代理。
- -6 选项指示 ipt2socks 仅启用 IPv6 协议栈的透明代理。
- -v 选项指示 ipt2socks 在运行期间打印详细的日志信息。
- -V 选项打印 ipt2socks 的版本号，然后退出 ipt2socks 进程。
- -h 选项打印 ipt2socks 的帮助信息，然后退出 ipt2socks 进程。

**以普通用户运行 ipt2socks**
- `sudo setcap cap_net_bind_service,cap_net_admin+ep /usr/local/bin/ipt2socks`
- 如果以 root 用户启动 ipt2socks，也可以指定 `-u nobody` 选项切换至 `nobody` 用户

Enjoy it!
