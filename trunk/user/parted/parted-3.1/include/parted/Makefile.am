if COMPILE_FOR_S390
S390_HDRS = fdasd.h vtoc.h
else
S390_HDRS =
endif

partedincludedir = $(includedir)/parted
partedinclude_HEADERS = constraint.h	\
			debug.h		\
			device.h	\
			disk.h		\
			exception.h	\
			filesys.h	\
			geom.h		\
			natmath.h	\
			timer.h		\
			unit.h		\
			parted.h

noinst_HEADERS	      = crc32.h		\
			endian.h	\
			$(S390_HDRS)

BUILT_SOURCES = \
  $(partedinclude_HEADERS)	\
  $(noinst_HEADERS)		\
  fdasd.h			\
  vtoc.h

EXTRA_DIST = $(BUILT_SOURCES:%.h=%.in.h)

# We use _GL_ATTRIBUTE_CONST and _GL_ATTRIBUTE_PURE internally,
# but cannot use those in a published header, so replace each of those
# strings with its expansion.
# Technically, attribute const was introduced in gcc-2.95 and
# attribute pure in 2.96, but to keep it simple, we're testing
# for 2.96 or newer for both.

hash_if = \n\#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)\n
subst_const_attr = \
  s/ _GL_ATTRIBUTE_CONST\b/$(hash_if)  __attribute ((__const__))\n\#endif\n/
subst_pure_attr = \
  s/ _GL_ATTRIBUTE_PURE\b/$(hash_if)  __attribute ((__pure__))\n\#endif\n/

$(BUILT_SOURCES): Makefile.am

SUFFIXES = .in.h .h
.in.h.h:
	$(AM_V_GEN)rm -f $@ $@-t
	$(AM_V_at)perl -p \
	    -e '$(subst_const_attr);' \
	    -e '$(subst_pure_attr);' \
	  $< > $@-t
	$(AM_V_at)chmod a=r $@-t
	$(AM_V_at)mv $@-t $@

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
