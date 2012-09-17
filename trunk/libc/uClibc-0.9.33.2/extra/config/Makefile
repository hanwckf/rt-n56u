top_srcdir ?= ../../

include $(top_srcdir)Rules.mak
include $(top_srcdir)Makerules

# ugh
top_srcdir:=$(shell cd $(top_srcdir) && pwd)/
ifdef O
top_builddir ?= ../../
else
top_builddir = ../../
endif
srctree := $(top_srcdir)
src := extra/config
obj := $(top_builddir)$(src)

generated := $(patsubst %_shipped,%,$(wildcard *_shipped))
generated := $(addprefix $(obj)/,$(generated:.c=.o))

include $(top_srcdir)extra/config/Makefile.kconfig
PHONY += $(always)
chk-lxdialog := $(top_srcdir)$(src)/lxdialog/check-lxdialog.sh
check-lxdialog := cd $(obj) && $(CONFIG_SHELL) $(chk-lxdialog)
HOST_EXTRACFLAGS = $(shell $(CONFIG_SHELL) -c '$(check-lxdialog) -ccflags')
HOST_LOADLIBES   = $(shell $(CONFIG_SHELL) -c '$(check-lxdialog) -ldflags $(HOSTCC)')
HOST_EXTRACFLAGS += -DLOCALE
HOST_EXTRACFLAGS += $(shell $(CONFIG_SHELL) -c '$(srctree)/$(src)/check.sh $(HOSTCC) $(HOSTCFLAGS)')

# do not create temporary object in the readonly srctree
$(obj)/dochecklxdialog:
	$(Q)cd $(obj) && $(CONFIG_SHELL) $(chk-lxdialog) -check $(HOSTCC) $(HOST_EXTRACFLAGS) $(HOST_LOADLIBES)
HOSTCFLAGS_lex.zconf.o	:= -I$(top_srcdir)$(src)
HOSTCFLAGS_zconf.tab.o	:= -I$(top_srcdir)$(src)
conf-objs := $(addprefix $(obj)/,$(conf-objs))
mconf-objs := $(addprefix $(obj)/,$(mconf-objs))
kxgettext-objs := $(addprefix $(obj)/,$(kxgettext-objs))

ifeq ($(findstring mconf,$(MAKECMDGOALS)),mconf)
hostprogs-y += mconf
endif

#BUILD_CFLAGS-config = -W -Wall -pedantic
#BUILD_CFLAGS-lxdialog = -W -Wall -pedantic

__hostprogs := $(sort $(hostprogs-y) $(hostprogs-m))
host-csingle:= $(foreach m,$(__hostprogs),$(if $($(m)-objs),,$(m)))
host-cmulti := $(foreach m,$(__hostprogs),\
           $(if $($(m)-cxxobjs),,$(if $($(m)-objs),$(m))))
host-cobjs  := $(sort $(foreach m,$(__hostprogs),$($(m)-objs)))

conf mconf kxgettext: %: $(obj)/%
$(obj)/conf $(obj)/mconf $(obj)/kxgettext: BUILD_LDFLAGS=$(HOST_LOADLIBES)
$(obj)/conf: $(conf-objs)
	$(hcompile.u)
$(obj)/mconf: $(mconf-objs)
	$(hcompile.u)
$(obj)/kxgettext: $(kxgettext-objs)
	$(hcompile.u)

$(host-csingle) $(host-cmulti) $(host-cobjs): BUILD_CFLAGS=$(HOST_EXTRACFLAGS) \
					$(HOSTCFLAGS) $(HOSTCFLAGS_$(@F))

host-cobjs.nogen := $(filter-out $(generated),$(host-cobjs))
host-cobjs.generated := $(filter $(generated),$(host-cobjs))

$(host-cobjs.nogen): $(obj)/%.o: $(top_srcdir)$(src)/%.c
	$(hcompile.o)
$(host-cobjs.generated): $(obj)/%.o: $(obj)/%.c
	$(hcompile.o)

ifndef LKC_GENPARSER
$(obj)/%:: $(top_srcdir)$(src)/%_shipped
	@$(disp_gen)
	$(Q)cat $< > $@
endif
CLEAN_extra/config menuconfig_clean:
	$(do_rm) $(clean-files) $(lxdialog) conf $(wildcard *.o)
distclean: clean
	$(do_rm) $(lxdialog) $(conf-objs) $(mconf-objs) \
		$(kxgettext-objs) \
		$(hostprogs-y) $(qconf-cxxobjs) $(qconf-objs) $(gconf-objs) \
		.depend
	$(do_rm) -r $(top_builddir)include/config

FORCE:
.PHONY: FORCE clean distclean $(always)
