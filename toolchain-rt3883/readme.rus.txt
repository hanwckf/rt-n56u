* ИНСТРУКЦИЯ ПО СБОРКЕ КРОСС-TOOLCHAIN для CPU RT3883/3662 *

Для сборки кросс-toolchain требуется Linux окружение 32 или 64 бита. Сборка 
кросс-toolchain протестирована на Linux дистрибутивах Debian squeeze 6.0.3 и 
Ubuntu 10.04.

Для сборки кросс-toolchain необходимо выполнить скрипт "build_toolchain" и 
дождаться окончания процедуры сборки. Сборка кросс-toolchain занимает от 10 
минут до нескольких часов, в зависимости от типа CPU хоста.


* ПРИМЕЧАНИЕ *

Для сборки кросс-toolchain из под Linux дистрибутива Debian Squeeze требуются пакеты:
- build-essential
- gawk
- sudo
- pkg-config
- gettext
- automake
- autoconf
- bison
- flex
- libgmp3-dev
- libmpfr-dev
- libmpc-dev



-
03.05.2012
Padavan
