DISTGEN = maketable normal pareto paretonormal
DISTDATA = normal.dist pareto.dist paretonormal.dist experimental.dist

HOSTCC ?= $(CC)
LDLIBS += -lm 

all: $(DISTGEN) $(DISTDATA)

$(DISTGEN):
	$(HOSTCC) $(CCOPTS) -I../include -o $@ $@.c -lm

%.dist: %
	./$* > $@

experimental.dist: maketable experimental.dat
	./maketable experimental.dat > experimental.dist

stats: stats.c
	$(HOSTCC) $(CCOPTS) -I../include -o $@ $@.c -lm

clean:
	rm -f $(DISTDATA) $(DISTGEN)
