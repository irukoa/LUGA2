# Arguments to the test runner.
TARGS :=

.PHONY: $(TESTGOALS)

# Testing: common infraestructure.
TESTEXE := test/Run_Tests.x
TESTOBJ := test/obj
TESTREP := test/reports

COVOMIT := awk -f buildsystem/OmitTerminatorLines.awk

TESTCOMMONSRC  := test/TSD
TESTCOMMONOBJ  := test/commonobj
TESTCOMMONSRCS := $(wildcard $(TESTCOMMONSRC)/*.c)
TESTCOMMONOBJS := $(patsubst $(TESTCOMMONSRC)/%.c,$(TESTCOMMONOBJ)/%.o,$(TESTCOMMONSRCS))
TESTINC        := -I$(TESTCOMMONSRC)

LDTESTFLAGS :=
LDTESTLIBS  := -llapack -lblas

# Common objects.
$(TESTCOMMONOBJ)/%.o: $(TESTCOMMONSRC)/%.c | $(TESTCOMMONOBJ)
		$(info $(call cls,Test common: $<)) $(CC) $(CFLAGS) $(INC) $(TESTINC) $(DEFS) -c $< -o $@
$(TESTCOMMONOBJ)/%.d: $(TESTCOMMONSRC)/%.c | $(TESTCOMMONOBJ)
		$(info $(call cls,Test common: $<)) $(DEPSCRIPT) $< $(TESTCOMMONOBJ) > $@ || $(RM) $@
TESTCOMMONDEPS := $(patsubst $(TESTCOMMONOBJ)/%.o,$(TESTCOMMONOBJ)/%.d,$(TESTCOMMONOBJS))
ifneq ($(filter-out clean, $(MAKECMDGOALS)), )
include $(TESTCOMMONDEPS)
endif

TESTSRCS :=
TESTOBJS :=
include buildsystem/TestRegistry.mk
TESTDEPS := $(patsubst $(TESTOBJ)/%.o,$(TESTOBJ)/%.d,$(TESTOBJS))
ifneq ($(filter-out clean, $(MAKECMDGOALS)), )
include $(TESTDEPS)
endif

test: $(TESTEXE)
$(TESTEXE): $(OBJS) $(TESTCOMMONOBJS) $(TESTOBJS)
		$(info $(call cls,Test binary: $@)) $(FC) $(FFLAGS) $(INC) $(TESTINC) $(DEFS) $^ -o $@ $(LDTESTFLAGS) $(LDTESTLIBS)

runtest: $(TESTEXE) | $(TESTREP)
		$(RM) $(TESTREP)/*
		$(RM) $(OBJ)/*.gcda
		./$(TESTEXE) $(TARGS) 2>&1 | tee $(TESTREP)/TestRun.log
		$(RM) $(OBJ)/Core.gcda
		lcov --capture --directory $(OBJ) --rc branch_coverage=1 --output-file=test/coverage.info
		$(COVOMIT) test/coverage.info > test/coverage.filtered.info
		$(MV) test/coverage.filtered.info test/coverage.info
		genhtml test/coverage.info --output-directory=$(TESTREP)
		$(MV) test/coverage.info $(TESTREP)
		valgrind --leak-check=full --show-leak-kinds=all ./$(TESTEXE) $(TARGS) 2>&1 | tee $(TESTREP)/ValgrindTestRun.log

coverage: $(TESTEXE) | $(TESTREP)
		$(RM) $(TESTREP)/*
		$(RM) $(OBJ)/*.gcda
		./$(TESTEXE) $(TARGS) 2>&1 | tee $(TESTREP)/TestRun.log
		$(RM) $(OBJ)/Core.gcda
		lcov --capture --directory $(OBJ) --rc branch_coverage=1 --output-file=test/coverage.info
		$(COVOMIT) test/coverage.info > test/coverage.filtered.info
		$(MV) test/coverage.filtered.info test/coverage.info
		genhtml test/coverage.info --output-directory=$(TESTREP)
		$(MV) test/coverage.info $(TESTREP)
		valgrind --leak-check=full --show-leak-kinds=all ./$(TESTEXE) $(TARGS) 2>&1 | tee $(TESTREP)/ValgrindTestRun.log
		firefox test/reports/index.html

$(TESTOBJ) $(TESTCOMMONOBJ) $(TESTREP):
		$(info Directory: $@) $(MKDIR) $@

clean::
		$(RM) $(TESTOBJ) $(TESTCOMMONOBJ)
		$(RM) $(TESTREP)
		$(RM) $(TESTEXE)
