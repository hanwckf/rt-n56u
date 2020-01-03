#noinst_PROGRAMS = encode listcap names posix simple sql syntax crnl

lib_onig = ../src/libonig.la
LDADD  = $(lib_onig)

AM_LDFLAGS  = -L$(prefix)/lib
AM_CPPFLAGS = -I../src -I$(includedir)

TESTS = encode listcap names posix simple sql syntax user_property bug_fix

check_PROGRAMS = encode listcap names posix simple sql syntax user_property bug_fix

encode_SOURCES  = encode.c
listcap_SOURCES = listcap.c
names_SOURCES   = names.c
posix_SOURCES   = posix.c
simple_SOURCES  = simple.c
sql_SOURCES     = sql.c
syntax_SOURCES  = syntax.c
user_property_SOURCES = user_property.c
bug_fix         = bug_fix.c

sampledir = .

test: encode listcap names posix simple sql syntax user_property bug_fix
	$(sampledir)/encode
	$(sampledir)/listcap
	$(sampledir)/names
	$(sampledir)/posix
	$(sampledir)/simple
	$(sampledir)/sql
	$(sampledir)/syntax
	$(sampledir)/user_property
	$(sampledir)/bug_fix
