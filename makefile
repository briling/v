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
OFLAGS= -lm $(GPROF) -lX11 -lXpm

INCL=\
-I$(SRCDIR)/mol  -I$(SRCDIR)/math \
-I$(SRCDIR)/v    -I$(SRCDIR)/sym

OBJDIR=./obj
SRCDIR=./src
src=$(wildcard $(SRCDIR)/*/*.c)
obj=$(src:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
allobj=$(obj) $(OBJDIR)/v.o


default : v

all : v

v  : $(allobj)
	$(CC) $^ -o $@ $(OFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@ $(INCL)

$(OBJDIR)/v.o : $(SRCDIR)/v.c
	$(CC) $(CFLAGS) $< -o $@ $(INCL) $(VERSION_FLAGS)

# clean object files that can cause trouble if built without -fPIC
clean_no_fpic_o:
	@for o in $(allobj) ; do \
		if [[ -f $$o ]] ; then \
		dump=$$(objdump --reloc $$o); \
		if echo "$${dump}" | egrep -q '\.rodata\.|\.rodata$$' ; then \
		rm -vf $$o ; \
		fi ; \
		if echo "$${dump}" | grep 'dis-' | grep -q 'R_X86_64_PC32' ; then \
		rm -vf $$o ; \
		fi ; \
		fi ; \
		done
v.so : clean_no_fpic_o v.so_inner
v.so_inner: CFLAGS += -fPIC
v.so_inner: $(allobj)
	$(CC) $^ -shared -Wl,-soname,$@ $(OFLAGS) -o v.so


clean:
	rm -f $(OBJDIR)/*/*/*.o $(OBJDIR)/*/*.o $(OBJDIR)/*.o v v.so

cleand:
	rm -f $(OBJDIR)/*/*/*.d $(OBJDIR)/*/*.d $(OBJDIR)/*.d
cleantags:
	rm -f ./.tags ./.types.vim
cleanasm:
	rm -f $(OBJDIR)/*/*/*.s $(OBJDIR)/*/*.s $(OBJDIR)/*.s
cleanall: clean cleand cleantags cleanasm

include $(wildcard $(OBJDIR)/*/*.d $(OBJDIR)/*.d)

