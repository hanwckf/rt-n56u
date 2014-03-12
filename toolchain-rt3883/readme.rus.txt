* ИНСТРУКЦИЯ ПО СБОРКЕ КРОСС-TOOLCHAIN для CPU Ralink RT3883/3662 (MIPS-74Kc) *

Для сборки кросс-toolchain требуется Linux окружение 32 или 64 бита. Сборка 
кросс-toolchain протестирована на Linux дистрибутивах Debian squeeze 6.0.7 и 
Ubuntu 10.04.

Для сборки кросс-toolchain необходимо выполнить скрипт "build_toolchain" 
и дождаться окончания процедуры сборки. Сборка занимает от 10 минут до 
нескольких часов, в зависимости от типа CPU хоста.

Скрипт "build_toolchain" выполнит сборку кросс-toolchain для ядра Linux 3.0.x.
Собранный кросс-toolchain будет находится в директории "toolchain-3.0.x".

Скрипт "build_toolchain_3.4.x" выполнит сборку кросс-toolchain для ядра Linux 3.4.x.
Собранный кросс-toolchain будет находится в директории "toolchain-3.4.x".


* КОМПОНЕНТЫ КРОСС-TOOLCHAIN *

binutils-2.24 + upstream патчи
gcc-4.4.7 + upstream патчи
uClibc-0.9.33.2 + upstream патчи


* ПРИМЕЧАНИЕ *

Для сборки кросс-toolchain из под Linux дистрибутива Debian Squeeze требуются пакеты:
- build-essential
- gawk
- sudo
- pkg-config
- gettext
- automake
- autoconf
- libtool
- bison
- flex
- texinfo
- libgmp3-dev
- libmpfr-dev
- libmpc-dev



-
12.03.2014
Padavan
