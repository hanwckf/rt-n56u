#  The top-level makefile simply invokes all the other makefiles

prefix = /usr/local

PROGRAMS = sstrip

all: $(PROGRAMS)

bin/%:
	$(MAKE) -C$* $*
	cp $*/$* ../$*/

sstrip: bin/sstrip

clean:
	for dir in elfrw $(PROGRAMS) ; do $(MAKE) -C$$dir clean ; done
	rm -f ../$(PROGRAMS)/$(PROGRAMS)
