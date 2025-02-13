LLC = ../llvm/build/bin/llc
LIT = ../llvm/build/bin/llvm-lit

EXTRA =

LLCFLAGS =
LLCFLAGS += -mtriple=riscv32
LLCFLAGS += --run-pass=greedy,virtregrewriter
#LLCFLAGS += -stress-regalloc=1

LLCFLAGS += $(EXTRA)

LITFLAGS = 
LITFLAGS += -v

TESTS = 
TESTS += test_1_1
TESTS += test_1_2
TESTS += test_2_1
TESTS += test_2_2

MIRS = $(patsubst %, %.mir, $(TESTS))
MIRS_ENABLE = $(patsubst %, %.enable.mir, $(TESTS))
MIRS_DISABLE = $(patsubst %, %.disable.mir, $(TESTS))
LITS = $(patsubst %, %.lit, $(TESTS))

all: $(MIRS_DISABLE) $(MIRS_ENABLE)

%.enable.mir: %.mir
	$(LLC) $(LLCFLAGS) -debug-only=ami-liveintervals $< -o $@

%.disable.mir: %.mir
	$(LLC) $(LLCFLAGS) --ami-disable-add-segment $< -o $@

%.lit: %.mir
	$(LIT) $(LITFLAGS) $< 2>&1 | tee $@

clean:
	$(RM) $(MIRS_ENABLE)
	$(RM) $(MIRS_DISABLE)
	$(RM) $(LITS)
