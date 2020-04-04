#
# For a description of the syntax of this configuration file,
# see extra/config/Kconfig-language.txt
#
config TARGET_ARCH
	default "arc"

config FORCE_OPTIONS_FOR_ARCH
	bool
	default y
	select ARCH_ANY_ENDIAN

config CONFIG_ARC_HAS_ATOMICS
	bool "Support for LLOCK/SCOND instructions"
	default y
	help
	  LLOCK/SCOND instructions are needed to implement atomic r-m-w
	  Otherwise libc needs kernel assisted atomic cmpxchg available
	  since v4.9 kernel

choice
	prompt "MMU Page Size"
	default CONFIG_ARC_PAGE_SIZE_8K

config CONFIG_ARC_PAGE_SIZE_8K
	bool "8KB"
	help
	  Choose between 4k, 8k (default) or 16k

config CONFIG_ARC_PAGE_SIZE_16K
	bool "16KB"

config CONFIG_ARC_PAGE_SIZE_4K
	bool "4KB"

endchoice
