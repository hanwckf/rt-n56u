#include <asm/asm-offsets.h>
#include <asm-generic/vmlinux.lds.h>

#undef mips
#define mips mips
OUTPUT_ARCH(mips)
ENTRY(kernel_entry)

#ifdef CONFIG_32BIT
	#ifdef CONFIG_CPU_LITTLE_ENDIAN
		jiffies  = jiffies_64;
	#else
		jiffies  = jiffies_64 + 4;
	#endif
#else
	jiffies  = jiffies_64;
#endif

SECTIONS
{
#ifdef CONFIG_BOOT_ELF64
  /* Read-only sections, merged into text segment: */
  /* . = 0xc000000000000000; */

  /* This is the value for an Origin kernel, taken from an IRIX kernel.  */
  /* . = 0xc00000000001c000; */

  /* Set the vaddr for the text segment to a value
        >= 0xa800 0000 0001 9000 if no symmon is going to configured
        >= 0xa800 0000 0030 0000 otherwise  */

  /* . = 0xa800000000300000; */
  /* . = 0xa800000000300000; */
  . = 0xffffffff80300000;
#endif
  . = VMLINUX_LOAD_ADDRESS;
  /* read-only */
  _text = .;			/* Text and read-only data */
  .text : {
/* workaround for SPI flash boot */
#if defined (CONFIG_RALINK_RT2880) || \
    defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT3052) || \
    defined (CONFIG_RALINK_RT6855) || \
    defined (CONFIG_RALINK_RT5350)
#ifdef CONFIG_MTD_SPI_RALINK
    /* move some sections up (memory hole for spi IO) */
    . = . + 0x8000;
#endif
#endif
    TEXT_TEXT
    SCHED_TEXT
    LOCK_TEXT
    *(.fixup)
    *(.gnu.warning)
  } =0

  _etext = .;			/* End of text section */

  . = ALIGN(16);		/* Exception table */
  __start___ex_table = .;
  __ex_table : { *(__ex_table) }
  __stop___ex_table = .;

  __start___dbe_table = .;	/* Exception table for data bus errors */
  __dbe_table : { *(__dbe_table) }
  __stop___dbe_table = .;

  RODATA

/* workaround for SPI flash boot */
#if defined (CONFIG_RALINK_RT2880) || \
    defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT3052) || \
    defined (CONFIG_RALINK_RT6855) || \
    defined (CONFIG_RALINK_RT5350)
#ifdef CONFIG_MTD_SPI_RALINK
  /* This hack need for SPI devices work only */
  /* init need start > 0x80220000 */
  /* (memory hole for spi IO) */
  /* Hack: size of .data segment rounded to 4K page size */
  #define DATASIZE 0x15000
  _data_start_h = ABSOLUTE(.);
  /* Segment started below end of hole and ends after end of hole? Yes - move */
  . = (_data_start_h <= 0x80220000 && _data_start_h + DATASIZE > 0x80200000) ? 0x80220000 : .;
#endif
#endif

  /* writeable */
  .data : {			/* Data */
    . = . + DATAOFFSET;		/* for CONFIG_MAPPED_KERNEL */
    /*
     * This ALIGN is needed as a workaround for a bug a gcc bug upto 4.1 which
     * limits the maximum alignment to at most 32kB and results in the following
     * warning:
     *
     *  CC      arch/mips/kernel/init_task.o
     * arch/mips/kernel/init_task.c:30: warning: alignment of ‘init_thread_union’
     * is greater than maximum object file alignment.  Using 32768
     */
    . = ALIGN(_PAGE_SIZE);
    *(.data.init_task)

    DATA_DATA

    CONSTRUCTORS
  }
  _gp = . + 0x8000;
  .lit8 : { *(.lit8) }
  .lit4 : { *(.lit4) }
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata     : { *(.sdata) }

  . = ALIGN(_PAGE_SIZE);
  __nosave_begin = .;
  .data_nosave : { *(.data.nosave) }
  . = ALIGN(_PAGE_SIZE);
  __nosave_end = .;

  . = ALIGN(1 << CONFIG_MIPS_L1_CACHE_SHIFT);
  .data.cacheline_aligned : { *(.data.cacheline_aligned) }

  /* rarely changed data like cpu maps */
  . = ALIGN(1 << CONFIG_MIPS_L1_CACHE_SHIFT);
  .data.read_mostly : { *(.data.read_mostly) }

  _edata =  .;			/* End of data section */

  /* will be freed after init */
  . = ALIGN(_PAGE_SIZE);		/* Init code and data */
  __init_begin = .;
/* workaround for SPI flash boot */
#if defined (CONFIG_RALINK_RT2880) || \
    defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT3052) || \
    defined (CONFIG_RALINK_RT6855) || \
    defined (CONFIG_RALINK_RT5350)
#ifdef CONFIG_MTD_SPI_RALINK
#ifdef CONFIG_RT2880_FLASH_4M
  /* This hack need for SPI devices work only */
  /* init need start > 0x80220000 */
  /* (memory hole for spi IO) */
  /* Hack: size of .init segment rounded to 4K page size */
  #define INITSIZE 0x18000
  _init_start_h = ABSOLUTE(.);
  /* Segment started below end of hle and ends after end of hole? Yes - move */
  . = (_init_start_h <= 0x80220000 && _init_start_h + INITSIZE > 0x80200000) ? 0x80220000 : .;
#endif
#endif
#endif
  .init.text : {
	_sinittext = .;
	*(.init.text)
	_einittext = .;
  }
  .init.data : { *(.init.data) }
  . = ALIGN(16);
  __setup_start = .;
  .init.setup : { *(.init.setup) }
  __setup_end = .;

  __initcall_start = .;
  .initcall.init : {
	INITCALLS
  }
  __initcall_end = .;

  __con_initcall_start = .;
  .con_initcall.init : { *(.con_initcall.init) }
  __con_initcall_end = .;
  SECURITY_INIT
    /* .exit.text is discarded at runtime, not link time, to deal with
     references from .rodata */
  .exit.text : { *(.exit.text) }
#if defined(CONFIG_BLK_DEV_INITRD)
  . = ALIGN(_PAGE_SIZE);
  __initramfs_start = .;
  .init.ramfs : { *(.init.ramfs) }
  __initramfs_end = .;
#endif
  . = ALIGN(32);
  __per_cpu_start = .;
  .data.percpu  : { *(.data.percpu) }
  __per_cpu_end = .;
  . = ALIGN(_PAGE_SIZE);
  __init_end = .;
  /* freed after init ends here */

  __bss_start = .;		/* BSS */
  .sbss      : {
    *(.sbss)
    *(.scommon)
  }
  .bss : {
    *(.bss)
    *(COMMON)
    *(.cod0)
    *(.cod1)
    *(.dec0)
    *(.dec1)
    . = (ALIGN(_PAGE_SIZE) - .) < 8 ? ALIGN(_PAGE_SIZE) + 4 : .        ;
  }
  __bss_stop = .;

  _end = . ;

  /* Sections to be discarded */
  /DISCARD/ : {
	*(.exit.data)
        *(.exitcall.exit)

	/* ABI crap starts here */
	*(.comment)
	*(.MIPS.options)
	*(.note)
	*(.options)
	*(.pdr)
	*(.reginfo)
	*(.mdebug*)
  }

  /* This is the MIPS specific mdebug section.  */
  .mdebug : { *(.mdebug) }

  STABS_DEBUG

  DWARF_DEBUG

  /* These must appear regardless of  .  */
  .gptab.sdata : { *(.gptab.data) *(.gptab.sdata) }
  .gptab.sbss : { *(.gptab.bss) *(.gptab.sbss) }
  .note : { *(.note) }
}
