* ИНСТРУКЦИЯ ПО СБОРКЕ КРОСС-TOOLCHAIN *

Кросс-toolchain собирается под CPU архитектуры MIPS32_R2 LE:
- Ralink RT3883/RT3662 (MIPS 74Kc)
- MediaTek MT7620 (MIPS 24KEc)
- MediaTek MT7621 (MIPS 1004Kc)

Для сборки кросс-toolchain требуется Linux окружение 32 или 64 бита. Сборка
кросс-toolchain протестирована на Linux дистрибутивах Debian 'wheezy' 7.8.0 и
Debian 'jessie' 8.0.0.

Для сборки кросс-toolchain необходимо выполнить скрипт "build_toolchain"
и дождаться окончания процедуры сборки. Сборка занимает от 10 минут до
нескольких часов, в зависимости от типа CPU хоста.

Скрипт "build_toolchain" выполнит сборку кросс-toolchain для ядра Linux 3.4.x.
Собранный кросс-toolchain будет находится в директории "toolchain-3.4.x".

Скрипт "build_toolchain_3.0.x" выполнит сборку кросс-toolchain для ядра Linux 3.0.x.
Собранный кросс-toolchain будет находится в директории "toolchain-3.0.x". Ядро
Linux 3.0.x используется только для модели ASUS RT-N65U ввиду наличия бинарного
драйвера iNIC_mii.ko без исходных кодов.


* КОМПОНЕНТЫ КРОСС-TOOLCHAIN *

binutils-2.24 + upstream патчи
gcc-4.4.7 + upstream патчи
uClibc-0.9.33.2 + upstream патчи


* ПРИМЕЧАНИЕ *

Для сборки кросс-toolchain из под Linux дистрибутива Debian 'wheezy'/'jessie' требуются
пакеты (на других дистрибутивах состав и наименования пакетов могут отличаться):
- sudo
- build-essential
- gawk
- pkg-config
- gettext
- autoconf
- automake
- libtool
- bison
- flex
- texinfo
- libgmp3-dev
- libmpfr-dev
- libmpc-dev



-
10.05.2015
Padavan
