// File is taken from Chromium
#ifndef BUILD_BUILD_CONFIG_H_
#define BUILD_BUILD_CONFIG_H_

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// A set of macros to use for platform detection.
#if defined(__APPLE__)
#define OS_MACOSX 1
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define OS_IOS 1
#endif  // defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#elif defined(ANDROID)
#define OS_ANDROID 1
#elif defined(__native_client__)
#define OS_NACL 1
#elif defined(__FLASHPLAYER)
#define OS_FLASHPLAYER 1
#elif defined(__linux__)
#define OS_LINUX 1
#elif defined(_WIN32)
#define OS_WIN 1
#elif defined(__FreeBSD__)
#define OS_FREEBSD 1
#elif defined(__OpenBSD__)
#define OS_OPENBSD 1
#elif defined(__sun)
#define OS_SOLARIS 1
#elif defined(EMSCRIPTEN)
#define OS_EMSCRIPTEN 1
#else
#error Please add support for your platform in build_config.h
#endif

// For access to standard BSD features, use OS_BSD instead of a
// more specific macro.
#if defined(OS_FREEBSD) || defined(OS_OPENBSD)
#define OS_BSD 1
#endif

// For access to standard POSIXish features, use OS_POSIX instead of a
// more specific macro.
#if defined(OS_MACOSX) || defined(OS_LINUX) || defined(OS_FREEBSD) ||     \
    defined(OS_OPENBSD) || defined(OS_SOLARIS) || defined(OS_ANDROID) ||  \
    defined(OS_NACL)
#define OS_POSIX 1
#endif

#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID) && \
    !defined(OS_NACL)
#define USE_X11 1  // Use X for graphics.
#endif

// Compiler detection.
#if defined(__GNUC__)
#define COMPILER_GCC 1

#if defined(__clang__)
#define COMPILER_CLANG 1
#endif
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#elif defined(__TINYC__)
#define COMPILER_TCC 1
#else
#error Please add support for your compiler in build/build_config.h
#endif

// Processor architecture detection.  For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86_64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define ARCH_CPU_ALLOW_UNALIGNED 1
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define ARCH_CPU_ALLOW_UNALIGNED 1
#define ARCH_CPU_NEED_64BIT_ALIGN 1
#elif defined(__ARMEL__) || defined(__arm__) && defined(__ARMCC_VERSION)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARMEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARM64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define ARCH_CPU_ALLOW_UNALIGNED 1
#elif defined(__pnacl__)
#define ARCH_CPU_32_BITS 1
#elif defined(__MIPSEL__)
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPSEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(EMSCRIPTEN)
#define ARCH_CPU_JS 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__FLASHPLAYER)
#define ARCH_CPU_FLASHPLAYER 1
#define ARCH_CPU_32_BITS 1
#else
#error Please add support for your architecture in build_config.h
#endif

#if defined(ARCH_CPU_LITTLE_ENDIAN) && defined(ARCH_CPU_BIG_ENDIAN) || !defined(ARCH_CPU_LITTLE_ENDIAN) && !defined(ARCH_CPU_BIG_ENDIAN)
#error Please add support for your endianness in build_config.h
#endif


#endif  // BUILD_BUILD_CONFIG_H_
