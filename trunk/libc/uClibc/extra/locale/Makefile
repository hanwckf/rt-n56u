
TOPDIR = ../../
EXTRA_LOCALE_DIR:=${shell cd $(TOPDIR)/extra/locale && pwd}
include $(TOPDIR)Rules.mak

CFLAGS_wc8bit=-DCTYPE_PACKED=1
CFLAGS_mmap=
ifeq ($(strip $(UCLIBC_HAS_WCHAR)),y)
    CFLAGS_wc8bit += -DDO_WIDE_CHAR=1
    CFLAGS_mmap += -D__WCHAR_ENABLED=1
endif

all: links-target

codesets.txt:
	@if [ ! -f codesets.txt ] ; then \
	    set -e; \
	    echo " "; \
	    echo "You do not have a codesets.txt file.  Please create this "; \
	    echo "file in the extra/locale directory by running something like: "; \
	    echo -e "  find $(EXTRA_LOCALE_DIR)/charmaps -name \"*.pairs\" > \\"; \
	    echo -e "        $(EXTRA_LOCALE_DIR)/codesets.txt"; \
	    echo "and then edit that file to disable/enable the codesets you wish to support. "; \
	    echo " "; \
	    false; \
	fi;

gen_wc8bit: gen_wc8bit.c
	$(HOSTCC) $(HOSTCFLAGS) $(NATIVE_LDFLAGS) $(CFLAGS_wc8bit) -D__UCLIBC_GEN_LOCALE $^ -o $@

c8tables.h: gen_wc8bit codesets.txt
	./gen_wc8bit `cat codesets.txt`

gen_wctype: gen_wctype.c
	$(HOSTCC) $(HOSTCFLAGS) $(NATIVE_LDFLAGS) -D__UCLIBC_GEN_LOCALE $^ -o $@

# Warning! Beware tr_TR toupper/tolower exceptions!
wctables.h: gen_wctype
	./gen_wctype en_US

gen_locale: gen_locale.c c8tables.h
	$(HOSTCC) $(HOSTCFLAGS) $(NATIVE_LDFLAGS) gen_locale.c -o $@

locale_tables.h: gen_locale locales.txt
	./gen_locale locales.txt

lt_defines.h: locale_tables.h locale_collate.h
	grep "^#define" locale_tables.h > lt_defines.h
	grep "^#define __lc" locale_collate.h >> lt_defines.h

gen_collate: gen_collate.c
	$(HOSTCC) $(HOSTCFLAGS) $(NATIVE_LDFLAGS) gen_collate.c -o $@

locale_collate.h: gen_collate locale_tables.h
	grep COL_IDX_ locale_tables.h | sed -e "s/^.*COL_IDX_\([^, ]*\).*$$/\1/" | sort | uniq | xargs ./gen_collate

locales.txt:
	@if [ ! -f locales.txt ] ; then \
	    set -e; \
	    echo " "; \
	    echo "You do not have a locales.txt file in the extra/locale "; \
	    echo "directory, Please copy the LOCALES file to locales.txt by "; \
	    echo "running something like: "; \
	    echo "  cp $(EXTRA_LOCALE_DIR)/LOCALES \\ "; \
	    echo "      $(EXTRA_LOCALE_DIR)/locales.txt "; \
	    echo "then edit locales.txt to disable/enable the locales you wish "; \
	    echo "to support. "; \
	    echo " "; \
	    false; \
	fi;

gen_ldc: gen_ldc.c c8tables.h wctables.h locale_tables.h locale_collate.h
	$(HOSTCC) $(HOSTCFLAGS) $(NATIVE_LDFLAGS) $(CFLAGS_mmap) gen_ldc.c -o $@

locale_data.c: gen_ldc
	./gen_ldc

uClibc_locale_data.h: c8tables.h wctables.h lt_defines.h locale_mmap.h
	grep -v "define __LC" lt_defines.h > uClibc_locale_data.h
	cat c8tables.h wctables.h locale_mmap.h >> uClibc_locale_data.h

links-target: locale_data.c uClibc_locale_data.h
	cat uClibc_locale_data.h | awk 'BEGIN{i=1}{ if ( /WANT_/ ) i = /endif/ ; else if (i) print $0 }' > ../../include/bits/uClibc_locale_data.h

pregen:
	cat uClibc_locale_data.h | awk 'BEGIN{i=1}{ if ( /WANT_/ ) i = /endif/ ; else if (i) print $0 }' > ../../include/bits/uClibc_locale_data.h

clean:
	$(RM) *.[oa] *~ core
	$(RM) gen_wc8bit gen_wctype gen_locale gen_ldc gen_collate
	$(RM) c8tables.h wctables.h locale_tables.h lt_defines.h locale_collate.h
	$(RM) gen_mmap locale.mmap lmmtolso
	$(RM) locale_data.c uClibc_locale_data.h

.PHONY: pregen links-target
