#
# For a description of the syntax of this configuration file,
# see extra/config/Kconfig-language.txt
#

config TARGET_ARCH
	string
	default "sh"

config FORCE_OPTIONS_FOR_ARCH
	bool
	default y
	select ARCH_ANY_ENDIAN

choice
	prompt "Target Processor Type"
	default CONFIG_SH4
	help
	  This is the processor type of your CPU. This information is used for
	  optimizing purposes, as well as to determine if your CPU has an MMU,
	  an FPU, etc.  If you pick the wrong CPU type, there is no guarantee
	  that uClibc will work at all....

	  Here are the available choices:
	  - "SH2A" Renesas SH-2A (SH7206)
	  - "SH2" SuperH SH-2
	  - "SH3" SuperH SH-3
	  - "SH4" SuperH SH-4

config CONFIG_SH2A
	select ARCH_HAS_NO_MMU
	select HAVE_NO_PIC
	bool "SH2A"

config CONFIG_SH2
	select ARCH_HAS_NO_MMU
	bool "SH2"

config CONFIG_SH3
	select ARCH_HAS_MMU
	bool "SH3"

config CONFIG_SH4
	bool "SH4"

endchoice

config ARCH_HAS_BWD_MEMCPY
       bool
       default y
       depends on CONFIG_SH4
