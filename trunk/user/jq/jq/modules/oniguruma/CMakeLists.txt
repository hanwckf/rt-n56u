
cmake_minimum_required(VERSION 2.8)
# required for exports? cmake_minimum_required (VERSION 2.8.6)
project(oniguruma C)

set(PACKAGE onig)
set(PACKAGE_VERSION "6.1.3")

set(USE_COMBINATION_EXPLOSION_CHECK 0)
set(USE_CRNL_AS_LINE_TERMINATOR 0)
set(VERSION ${PACKAGE_VERSION})

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
elseif(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()


include(cmake/dist.cmake)
include(CheckCSourceCompiles)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(TestBigEndian)

check_function_exists(alloca HAVE_ALLOCA)
check_include_files(alloca.h HAVE_ALLOCA_H)
set(HAVE_PROTOTYPES 1)
check_include_files(stdarg.h    HAVE_STDARG_PROTOTYPES)
check_include_files(stdint.h    HAVE_STDINT_H)
check_include_files(stdlib.h    HAVE_STDLIB_H)
check_include_files(strings.h   HAVE_STRINGS_H)
check_include_files(string.h    HAVE_STRING_H)
check_include_files(sys/times.h HAVE_SYS_TIMES_H)
check_include_files(sys/time.h  HAVE_SYS_TIME_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h    HAVE_UNISTD_H)
check_type_size(int SIZEOF_INT)
check_type_size(long SIZEOF_LONG)
check_type_size(short SIZEOF_SHORT)
check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/oniguruma.pc.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/oniguruma.pc @ONLY)


include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(_SRCS src/regint.h src/regparse.h src/regenc.h src/st.h
 src/regerror.c src/regparse.c src/regext.c src/regcomp.c src/regexec.c
 src/reggnu.c src/regenc.c src/regsyntax.c src/regtrav.c src/regversion.c
 src/st.c src/regposix.c src/regposerr.c src/onig_init.c
 src/unicode.c src/ascii.c src/utf8.c src/utf16_be.c src/utf16_le.c
 src/utf32_be.c src/utf32_le.c src/euc_jp.c src/sjis.c src/iso8859_1.c
 src/iso8859_2.c src/iso8859_3.c src/iso8859_4.c src/iso8859_5.c
 src/iso8859_6.c src/iso8859_7.c src/iso8859_8.c src/iso8859_9.c
 src/iso8859_10.c src/iso8859_11.c src/iso8859_13.c src/iso8859_14.c
 src/iso8859_15.c src/iso8859_16.c src/euc_tw.c src/euc_kr.c src/big5.c
 src/gb18030.c src/koi8_r.c src/cp1251.c
 src/euc_jp_prop.c src/sjis_prop.c
 src/unicode_unfold_key.c
 src/unicode_fold1_key.c src/unicode_fold2_key.c src/unicode_fold3_key.c)


add_library(onig ${_SRCS})

install_library(onig)

install_header(src/oniguruma.h src/onigposix.h src/oniggnu.h)

install_doc(doc/API doc/API.ja doc/RE doc/RE.ja doc/UNICODE_PROPERTIES)
install_data(AUTHORS COPYING HISTORY README)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/oniguruma.pc
  DESTINATION lib/pkgconfig)
