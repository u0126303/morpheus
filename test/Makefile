LLC = ../llvm/build/bin/llc
LIT = ../llvm/build/bin/llvm-lit

EXTRA =

LLCFLAGS =
LLCFLAGS += -mtriple=riscv32
#LLCFLAGS += -stress-regalloc=1

# Only run register allocater
LLCREGALLOC += --run-pass=greedy,virtregrewriter,postrapseudos

# Directly emit assembly
LLCEMIT += --start-after=machine-opt-remark-emitter

LLCFLAGS += $(EXTRA)

LITFLAGS = 
LITFLAGS += -v
LITFLAGS += --config-prefix=lit.standalone

TESTS = 
TESTS += test_1_1

TESTS += test_1_2

TESTS += test_2_1
TESTS += test_2_1_1

TESTS += test_2_2

MIRS = $(patsubst %, %.mir, $(TESTS))
MIRS_ENABLE = $(patsubst %, %.enable.mir, $(TESTS))
MIRS_DISABLE = $(patsubst %, %.disable.mir, $(TESTS))
RISCVS = $(patsubst %, %.s, $(TESTS))
LITS = $(patsubst %, %.lit, $(TESTS))

all: $(MIRS_DISABLE) $(MIRS_ENABLE) $(RISCVS)

%.enable.mir: %.mir
	$(LLC) $(LLCFLAGS) $(LLCREGALLOC) -debug-only=ami-liveintervals $< -o $@

%.disable.mir: %.mir
	$(LLC) $(LLCFLAGS) $(LLCREGALLOC) --ami-disable-add-segment $< -o $@

%.s: %.enable.mir
	$(LLC) $(LLCFLAGS) $(LLCEMIT) $< -o $@

%.lit: %.mir
	$(LIT) $(LITFLAGS) $< 2>&1 | tee $@

.PHONY: %.test
test:
	$(RM) $(MIRS_ENABLE)
	$(RM) $(MIRS_DISABLE)
	$(LIT) .

.PHONY: %.cfg
%.cfg: %.mir
	$(LLC) $(LLCFLAGS) -ami-view-cfg $< -o - > /dev/null

clean:
	$(RM) $(MIRS_ENABLE)
	$(RM) $(MIRS_DISABLE)
	$(RM) $(LITS)
	$(RM) $(RISCVS)
