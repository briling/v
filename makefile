CC=gcc
OPT=-O2
UNAME_S:=$(shell uname -s)
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

CFLAGS= -c -std=gnu11 $(OPT) $(GPROF) $(W) $(GDB)
OFLAGS= -lm $(GPROF)

USE_X11 ?= auto

X11_CPPFLAGS=
X11_LDFLAGS=
ifeq ($(UNAME_S),Darwin)
  ifneq ($(wildcard /opt/X11/include/X11/Xlib.h),)
    X11_CPPFLAGS += -I/opt/X11/include
  endif
  ifneq ($(wildcard /opt/X11/lib),)
    X11_LDFLAGS += -L/opt/X11/lib
  endif
endif

ifeq ($(USE_X11),auto)
  HAVE_X11_HEADERS:=$(shell sh -c "echo | $(CC) $(X11_CPPFLAGS) -E -include X11/Xlib.h -include X11/xpm.h - >/dev/null 2>&1 && echo 1 || echo 0")
  HAVE_X11_LIBS:=$(shell sh -c "$(CC) -shared -x c /dev/null $(X11_LDFLAGS) -lX11 -lXpm -o /tmp/v_x11_libprobe.so >/dev/null 2>&1 && rm -f /tmp/v_x11_libprobe.so && echo 1 || echo 0")
  HAVE_X11:=$(shell [ "$(HAVE_X11_HEADERS)" = "1" ] && [ "$(HAVE_X11_LIBS)" = "1" ] && echo 1 || echo 0)
else ifeq ($(USE_X11),1)
  HAVE_X11:=1
else
  HAVE_X11:=0
endif

SRCDIR=src
OBJDIR=obj
PICDIR=obj-pic

SRCDIRS=$(shell find $(SRCDIR) -type d)
INCL=$(SRCDIRS:%=-I./%)

allsrc_raw=$(shell find $(SRCDIR) -type f -name '*.c')
src_x11_only=$(SRCDIR)/x/x.c $(SRCDIR)/x/ac3_draw.c $(SRCDIR)/x/loop.c $(SRCDIR)/x/xinput.c
src_no_x11=$(SRCDIR)/no-x/x.c $(SRCDIR)/no-x/ac3_draw.c
ifeq ($(HAVE_X11),1)
  allsrc=$(filter-out $(src_no_x11),$(allsrc_raw))
  CFLAGS += $(X11_CPPFLAGS)
  OFLAGS += $(X11_LDFLAGS) -lX11 -lXpm
else
  allsrc=$(filter-out $(src_x11_only),$(allsrc_raw))
  CFLAGS += -DNO_X11
endif
ifeq ($(HAVE_X11),1)
  $(info Building with X11 support (set USE_X11=0 to force headless build))
else
  $(info X11 not detected; building in headless mode (set USE_X11=1 to require X11))
endif
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
