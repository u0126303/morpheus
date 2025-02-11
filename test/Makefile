TESTS  = 
TESTS += case.1.2

LLS = $(patsubst %, %.ll, $(TESTS))
MIRS_BEFORE = $(patsubst %, %.before.mir, $(TESTS))
MIRS_AFTER = $(patsubst %, %.after.mir, $(TESTS))

all:

clean:
	$(RM) $(MIRS_AFTER)
	$(RM) $(LLS)
