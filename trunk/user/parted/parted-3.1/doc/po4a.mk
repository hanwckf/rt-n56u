#
# You must set the $(lang) variable when you include this makefile.
#
# You can use the $(po4a_translate_options) variable to specify additional
# options to po4a.
# For example: po4a_translate_options=-L KOI8-R -A KOI8-R
#
#
# This makefile deals with the manpages generated from POs with po4a, and
# should be included in an automake Makefile.am.
#
# The po must be named:
#   <man>.$(lang).po
# If a man page require an addendum, you must name it:
#   <man>.$(lang).po.addendum
# Where <man> corresponds to a filename in the C directory (which contains
# the English man pages).
#
# The POs suffix is $(lang).po to allow dl10n to detect the outdated POs.
#
#
# If a man page cannot be generated (it is not sufficiently translated; the
# threshold is 80%), it won't be distributed, and the build won't fail.
#

mandir = $(mandir)/$(lang)

# Inform automake that we want to install some man pages in section 1, 5
# and 8.
# We can't simply use:
# dist_man_MANS = $(wildcard *.[1-9])
# Because when Makefile.in is generated, dist_man_MANS is empty, and
# automake do not generate the install-man targets.
dist_man_MANS =

# Override the automake's install-man target.
# And set dist_man_MANS according to the pages that could be generated
# when this target is called.
install-man: dist_man_MANS = pt_BR-parted.8
install-man: install-man1 install-man5 install-man8

# For each .po, try to generate the man page
all-local:
	for po in `ls -1 $(srcdir)/*.$(lang).po 2>/dev/null`; do \
		$(MAKE) $$(basename $${po%.$(lang).po}); \
	done

# Remove the man pages that were generated from a .po
clean-local:
	for po in `ls -1 $(srcdir)/*.$(lang).po 2>/dev/null`; do \
		rm -f $$(basename $${po%.$(lang).po}); \
	done

.PHONY: updatepo
# Update the PO in srcdir, according to the POT in C.
# Based on the gettext po/Makefile.in.in
updatepo:
	tmpdir=`pwd`; \
	cd $(srcdir); \
	for po in *.$(lang).po; do \
	  case "$$po" in '*'*) continue;; esac; \
	  pot=../C/po/$${po%$(lang).po}pot; \
	  echo "$(MSGMERGE) $$po $$pot -o $${po%po}new.po"; \
	  if $(MSGMERGE) $$po $$pot -o $$tmpdir/$${po%po}new.po; then \
	    if cmp $$po $$tmpdir/$${po%po}new.po >/dev/null 2>&1; then \
	      rm -f $$tmpdir/$${po%po}new.po; \
	    else \
	      if mv -f $$tmpdir/$${po%po}new.po $$po; then \
		:; \
	      else \
		echo "msgmerge for $$po failed: cannot move $$tmpdir/$${po%po}new.po to $$po" 1>&2; \
		exit 1; \
	      fi; \
	    fi; \
	  else \
	    echo "msgmerge for $$po failed!" 1>&2; \
	    rm -f $$tmpdir/$${po%po}new.po; \
	  fi; \
	  msgfmt -o /dev/null --statistics $$po; \
	done

dist-hook: updatepo

# Build the pages
partprobe.8:
	for locale in pt_BR ; do \
		po4a-translate -f man -m $(srcdir)/../C/$@ -p $@.$$locale.po -l $@ $(po4a_translate_options) ; \
		if [ -f $(srcdir)/$@.$$locale.po.addendum ]; then \
			po4a-translate -f man -m $(srcdir)/../C/$@ -p $@.$$locale.po -l $@ -a $(srcdir)/$@.$$locale.po.addendum $(po4a_translate_options) ; \
		fi ; \
	done
