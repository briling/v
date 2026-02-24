CC=gcc
OPT=-O2
GPROF=#-pg
GDB=#-g
W= \
   -Warray-bounds\
   -Wfloat-equal\
   -Wimplicit\
   -Wmaybe-uninitialized\
   -Wmissing-braces\
   -Wparentheses\
   -Wsequence-point\
   -Wtype-limits\
   -Wundef\
   -Wuninitialized\
   -Wmisleading-indentation\
   -Wempty-body\
   -Wmemset-elt-size\
   -Wduplicated-branches\
   -Wswitch-unreachable\
   -Wunused\
   -Wunused-but-set-variable\
   -Wunused-parameter\
   #-Winline\
   #-Wunsafe-loop-optimizations\
   #-W -Wall\
   -Wswitch\
   -Wno-format\
   -Wconversion\
   -Wsign-compare\
   -Wjump-misses-init\
   #-Werror\

export VERSION_FLAGS=-DGIT_HASH="\"$(shell git rev-parse HEAD 2> /dev/null || echo --)\""\
                     -DGIT_BRANCH="\"$(shell git rev-parse --abbrev-ref HEAD 2> /dev/null || echo --)\""\
                     -DBUILD_USER="\"$(USER)@$(HOSTNAME)\""\
                     -DBUILD_DIRECTORY="\"$(PWD)\""

CFLAGS= -c -std=gnu11 $(OPT) $(GPROF) $(W) $(GDB)
OFLAGS= -lm $(GPROF) -lX11 -lXpm

SRCDIR=src
OBJDIR=obj
PICDIR=obj-pic

SRCDIRS=$(shell find $(SRCDIR) -type d)
INCL=$(SRCDIRS:%=-I./%)

allsrc=$(shell find $(SRCDIR) -type f -name '*.c')
allobj=$(allsrc:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
allpic=$(allsrc:$(SRCDIR)/%.c=$(PICDIR)/%.o)
allmmd=$(allsrc:$(SRCDIR)/%.c=$(OBJDIR)/%.d)

OBJDIRS=$(SRCDIRS:$(SRCDIR)%=$(OBJDIR)%)
PICDIRS=$(SRCDIRS:$(SRCDIR)%=$(PICDIR)%)
_=$(shell for i in $(OBJDIRS) $(PICDIRS); do mkdir -p $$i ; done)

default : v

all : v v.so

v  : $(allobj)
	$(CC) $^ -o $@ $(OFLAGS)

v.so: $(allpic)
	$(CC) $^ -shared -Wl,-soname,$@ $(OFLAGS) -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS)       $< -o $@ $(INCL) $(VERSION_FLAGS) -MMD -MT "$@ $(patsubst $(OBJDIR)%,$(PICDIR)%,$@)"

$(PICDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -fPIC $< -o $@ $(INCL) $(VERSION_FLAGS) -MMD -MT "$(patsubst $(PICDIR)%,$(OBJDIR)%,$@) $@" -MF $(patsubst $(PICDIR)/%.o,$(OBJDIR)/%.d,$@)

clean:
	rm -f $(allobj) $(allpic) v v.so
cleand:
	rm -f $(allmmd)
cleantags:
	rm -f ./.tags ./.types.vim
cleanall: clean cleand cleantags

include $(allmmd)
