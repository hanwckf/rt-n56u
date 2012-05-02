GENLOBJ=genl.o

include ../Config

GENLMODULES :=
GENLMODULES += ctrl.o

GENLOBJ += $(GENLMODULES)

GENLLIB :=

LDFLAGS += -Wl,-export-dynamic
LDLIBS  += -lm -ldl

all: genl

genl: $(GENLOBJ) $(LIBNETLINK) $(LIBUTIL) $(GENLLIB)

clean:
	rm -f $(GENLOBJ) $(GENLLIB) genl
