CC=gcc
OPT=-O2
GPROF=#-pg
GDB=#-g
ASM=#-g -Wa,-adhln=$@.s
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

GIT_HASH=`git rev-parse HEAD 2> /dev/null || echo --`
GIT_BRANCH=`git rev-parse --abbrev-ref HEAD 2> /dev/null || echo --`
export VERSION_FLAGS=-DGIT_HASH="\"$(GIT_HASH)\""\
                     -DGIT_BRANCH="\"$(GIT_BRANCH)\""\
		     -DBUILD_USER="\"$(USER)@$(shell hostname)\""\
		     -DBUILD_DIRECTORY="\"$(PWD)\""

CFLAGS= -c -std=gnu11 -MMD $(OPT) $(GPROF) $(W) $(GDB) $(ASM)
OFLAGS= -lm $(GPROF)

INCL=\
-I$(SRCDIR)/mol  -I$(SRCDIR)/math \
-I$(SRCDIR)/v    -I$(SRCDIR)/sym

OBJDIR=./obj
SRCDIR=./src

molsrc=$(wildcard $(SRCDIR)/mol/*.c)
molobj=$(molsrc:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

mathsrc=$(wildcard $(SRCDIR)/math/*.c)
mathobj=$(mathsrc:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

vsrc=$(wildcard $(SRCDIR)/v/*.c)
vobj=$(vsrc:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

symsrc=$(wildcard $(SRCDIR)/sym/*.c)
symobj=$(symsrc:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

default : v

all : v

v  : $(vobj) $(molobj) $(mathobj) $(symobj) $(OBJDIR)/v.o
	$(CC) $^ -o $@ $(OFLAGS) -lX11 -lXpm

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@ $(INCL)

$(OBJDIR)/v.o : $(SRCDIR)/v.c
	$(CC) $(CFLAGS) $< -o $@ $(INCL) -I$(SRCDIR)/v $(VERSION_FLAGS)

clean:
	rm -f $(OBJDIR)/*/*/*.o $(OBJDIR)/*/*.o $(OBJDIR)/*.o v

cleand:
	rm -f $(OBJDIR)/*/*/*.d $(OBJDIR)/*/*.d $(OBJDIR)/*.d
cleantags:
	rm -f ./.tags ./.types.vim
cleanasm:
	rm -f $(OBJDIR)/*/*/*.s $(OBJDIR)/*/*.s $(OBJDIR)/*.s
cleanall: clean cleand cleantags cleanasm

include $(wildcard $(OBJDIR)/*/*.d $(OBJDIR)/*.d)

