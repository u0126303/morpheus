LLC = ../llvm/build/bin/llc

LLCFLAGS =
LLCFLAGS += -mtriple=riscv32
LLCFLAGS += -debug-only=ami-liveintervals
LLCFLAGS += --run-pass=greedy,virtregrewriter
LLCFLAGS += -print-slotindexes

TESTS = 
TESTS += case_1_2

MIRS_BEFORE = $(patsubst %, %.before.mir, $(TESTS))
MIRS_AFTER = $(patsubst %, %.after.mir, $(TESTS))

all: $(MIRS_AFTER)

%.after.mir: %.mir
	$(LLC) $(LLCFLAGS) $< -o $@

clean:
	$(RM) $(MIRS_AFTER)
