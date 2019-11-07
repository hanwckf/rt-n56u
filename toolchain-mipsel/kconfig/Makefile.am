## vim: set noet :

## FIXME when kconfig is split into a subpackage, need to remove this option from sub-configure
transform		= s,x,x,

pkglibexec_PROGRAMS    = conf nconf mconf

EXTRA_DIST		= zconf.y zconf.l \
				  expr.h list.h lkc.h lkc_proto.h nconf.h lxdialog/dialog.h \
				  confdata.c expr.c kconf_id.c menu.c symbol.c util.c

CLEANFILES		= zconf.lex.c zconf.c
BUILT_SOURCES	= zconf.c zconf.lex.c

AM_LFLAGS		= -L -Pzconf
AM_YFLAGS		= -l -b zconf -p zconf
AM_CPPFLAGS		= -include config.h -DCONFIG_=\"CT_\"
AM_LIBTOOLFLAGS	= --tag CC

conf_SOURCES    = conf.c zconf.c
conf_LDADD      = $(LIBINTL)

nconf_SOURCES	= nconf.c nconf.gui.c zconf.c
nconf_CFLAGS	= $(CURSES_CFLAGS)
nconf_LDADD     = $(MENU_LIBS) $(PANEL_LIBS) $(CURSES_LIBS) $(LIBINTL)

mconf_SOURCES	= mconf.c zconf.c lxdialog/checklist.c lxdialog/inputbox.c \
				  lxdialog/menubox.c lxdialog/textbox.c lxdialog/util.c \
				  lxdialog/yesno.c
mconf_LDADD     = $(CURSES_LIBS) $(LIBINTL)

# automake's support for yacc/lex/gperf is too idiosyncratic. It doesn't
# support a common pattern of including lex-generated file into yacc, nor does
# it support using a different base name for the generated C files.
YACCCOMPILE	= $(YACC) $(AM_YFLAGS) $(YFLAGS)
AM_V_YACC		= $(am__v_YACC_@AM_V@)
am__v_YACC_		= $(am__v_YACC_@AM_DEFAULT_V@)
am__v_YACC_0	= @echo "  YACC    " $@;

LEXCOMPILE	= $(LEX) $(AM_LFLAGS) $(LFLAGS)
AM_V_LEX		= $(am__v_LEX_@AM_V@)
am__v_LEX_		= $(am__v_LEX_@AM_DEFAULT_V@)
am__v_LEX_0		= @echo "  LEX     " $@;

zconf.c: zconf.y
	$(AM_V_YACC)$(YACCCOMPILE) -o$@ $<

zconf.lex.c: zconf.l
	$(AM_V_LEX)$(LEXCOMPILE) -o$@ $<
