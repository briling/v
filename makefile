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
   -Winline\
   -Wunsafe-loop-optimizations\
   -Wno-format\
   -Wswitch\
   -Wdangling-pointer\
   #-W -Wall\
   -Wconversion\
   -Wsign-compare\
   -Wjump-misses-init\
   #-Werror\

export VERSION_FLAGS=-DGIT_HASH="\"$(shell git rev-parse HEAD 2> /dev/null || echo --)\""\
                     -DGIT_BRANCH="\"$(shell git rev-parse --abbrev-ref HEAD 2> /dev/null || echo --)\""\
                     -DBUILD_USER="\"$(USER)@$(HOSTNAME)\""\
                     -DBUILD_DIRECTORY="\"$(PWD)\""

OS := $(shell uname)
ifeq ($(OS),Darwin)
	SONAME := install_name,@rpath/
else
	SONAME := soname,
endif

X11DIR= $(shell pkg-config --libs-only-L x11 xpm)  # for macOS
XFTLIB= $(shell pkg-config --libs xft)
XFTCFLAGS= $(shell pkg-config --cflags xft)
CFLAGS= -c -std=gnu11 $(OPT) $(GPROF) $(W) $(GDB) $(XFTCFLAGS)
OFLAGS= -lm $(GPROF) -lX11 -lXpm $(X11DIR) $(XFTLIB)

SRCDIR=src
OBJDIR=obj
PICDIR=obj-pic

SRCDIRS=$(shell find $(SRCDIR) -type d)
INCL=$(SRCDIRS:%=-I./%)
INCL+=$(shell pkg-config --cflags-only-I x11 xpm)  # for macOS

allsrc=$(shell find $(SRCDIR) -type f -name '*.c')
allobj=$(allsrc:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
allpic=$(allsrc:$(SRCDIR)/%.c=$(PICDIR)/%.o)
allmmd=$(shell find $(OBJDIR) -type f -name '*.d')

OBJDIRS=$(SRCDIRS:$(SRCDIR)%=$(OBJDIR)%)
PICDIRS=$(SRCDIRS:$(SRCDIR)%=$(PICDIR)%)
_=$(shell for i in $(OBJDIRS) $(PICDIRS); do mkdir -p $$i ; done)

default : v

all : v v.so

v  : $(allobj)
	$(CC) $^ -o $@ $(OFLAGS)

v.so: $(allpic)
	$(CC) $^ -shared -Wl,-$(SONAME)$@ $(OFLAGS) -o $@

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
cleancheck:
	rm -rf ./errors.xml ./cppcheck_html/
cleanall: clean cleand cleantags cleancheck

include $(allmmd)

cppcheck: cleancheck
	-.github/cppcheck.bash 2> errors.xml
	cppcheck-htmlreport --file=errors.xml --report-dir=cppcheck_html
