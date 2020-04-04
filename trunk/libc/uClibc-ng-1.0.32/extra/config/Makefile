top_srcdir ?= ../../

ifdef O
top_builddir ?= ../../
else
top_builddir = ../../
endif

include $(top_srcdir)Rules.mak
include $(top_srcdir)Makerules

# ugh
top_srcdir:=$(shell cd $(top_srcdir) && pwd)/

src := extra/config
obj := $(top_builddir)$(src)

generated := $(patsubst %_shipped,%,$(wildcard *_shipped))
generated := $(addprefix $(obj)/,$(generated:.c=.o))

include $(top_srcdir)extra/config/Makefile.kconfig
HOST_EXTRACFLAGS += -DCONFIG_='""'

# do not create temporary object in the readonly srctree
$(obj)/dochecklxdialog: CONFIG_SHELL:=cd $(obj) && $(CONFIG_SHELL)
HOSTCFLAGS_zconf.lex.o	:= -I$(top_srcdir)$(src)
HOSTCFLAGS_zconf.tab.o	:= -I$(top_srcdir)$(src)
conf-objs := $(addprefix $(obj)/,$(conf-objs))
mconf-objs := $(addprefix $(obj)/,$(mconf-objs))
nconf-objs := $(addprefix $(obj)/,$(nconf-objs))
kxgettext-objs := $(addprefix $(obj)/,$(kxgettext-objs))

ifeq ($(findstring mconf,$(MAKECMDGOALS)),mconf)
hostprogs-y += mconf
endif
ifeq ($(findstring nconf,$(MAKECMDGOALS)),nconf)
hostprogs-y += nconf
endif

__hostprogs := $(sort $(hostprogs-y) $(hostprogs-m))
host-csingle:= $(foreach m,$(__hostprogs),$(if $($(m)-objs),,$(m)))
host-cmulti := $(foreach m,$(__hostprogs),\
           $(if $($(m)-cxxobjs),,$(if $($(m)-objs),$(m))))
host-cobjs  := $(sort $(foreach m,$(__hostprogs),$($(m)-objs)))

conf mconf nconf kxgettext: %: $(obj)/%
$(obj)/conf $(obj)/mconf $(obj)/nconf $(obj)/kxgettext: BUILD_LDFLAGS=$(HOSTLOADLIBES_$(@F))
$(obj)/conf: $(conf-objs)
	$(hcompile.u)
$(obj)/mconf: $(mconf-objs)
	$(hcompile.u)
$(obj)/nconf: $(nconf-objs)
	$(hcompile.u)
$(obj)/kxgettext: $(kxgettext-objs)
	$(hcompile.u)

$(host-csingle) $(host-cmulti) $(host-cobjs): BUILD_CFLAGS+=$(HOST_EXTRACFLAGS) \
					$(HOSTCFLAGS) $(HOSTCFLAGS_$(@F))

host-cobjs.nogen := $(filter-out $(generated),$(host-cobjs))
host-cobjs.generated := $(filter $(generated),$(host-cobjs))

$(host-cobjs.nogen): $(obj)/%.o: $(top_srcdir)$(src)/%.c
	$(hcompile.o)
$(host-cobjs.generated): $(obj)/%.o: $(obj)/%.c
	$(hcompile.o)

# we use the pre-generated always
$(obj)/%:: $(top_srcdir)$(src)/%_shipped
	@$(disp_gen)
	$(Q)cat $< > $@

CLEAN_extra/config menuconfig_clean:
	$(do_rm) $(clean-files) $(lxdialog) conf $(wildcard *.o)
distclean: CLEAN_extra/config
	$(Q)$(RM) -r $(lxdialog) $(conf-objs) $(mconf-objs) $(nconf-objs) \
		$(kxgettext-objs) \
		$(hostprogs-y) $(qconf-cxxobjs) $(qconf-objs) $(gconf-objs) \
		.depend \
		$(top_builddir)include/config $(top_builddir)include/generated

.PHONY: clean distclean $(PHONY)
