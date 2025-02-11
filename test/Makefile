LLC = ../llvm/build/bin/llc

LLCFLAGS =
LLCFLAGS += -mtriple=riscv32
LLCFLAGS += --run-pass=greedy,virtregrewriter
#LLCFLAGS += -stress-regalloc=1

TESTS = 
TESTS += test_1_2
TESTS += test_2_1
TESTS += test_2_2

MIRS = $(patsubst %, %.mir, $(TESTS))
MIRS_ENABLE = $(patsubst %, %.enable.mir, $(TESTS))
MIRS_DISABLE = $(patsubst %, %.disable.mir, $(TESTS))

all: $(MIRS_DISABLE) $(MIRS_ENABLE)

%.enable.mir: %.mir
	$(LLC) $(LLCFLAGS) -debug-only=ami-liveintervals $< -o $@

%.disable.mir: %.mir
	$(LLC) $(LLCFLAGS) --ami-disable-add-segment $< -o $@

clean:
	$(RM) $(MIRS_ENABLE)
	$(RM) $(MIRS_DISABLE)
