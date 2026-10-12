DIR := test/Suites/MAC

# C test file compilation.
TESTSRCS += $(wildcard $(DIR)/*.c)
TESTOBJS += $(patsubst $(DIR)/%.c,$(TESTOBJ)/%.o,$(wildcard $(DIR)/*.c))

$(TESTOBJ)/%.o: $(DIR)/%.c | $(TESTOBJ)
		$(info C Test: $<) $(CC) $(CFLAGS) $(INC) $(TESTINC) $(DEFS) -c $< -o $@

$(TESTOBJ)/%.d: $(DIR)/%.c | $(TESTOBJ)
		$(info Dependency: $<) $(DEPSCRIPT) $< $(TESTOBJ) > $@ || $(RM) $@

# Fortran compiler-generated symbol weakening.
FCOBJS += $(patsubst $(DIR)/%.M90,$(OBJ)/%.def,$(wildcard $(DIR)/*.M90))

$(TESTOBJ)/%.def: $(DIR)/%.M90 | $(TESTOBJ) $(DSC)
		$(info Weak symbol list: $<) $(MKDIR) $(DSC)/$*; \
		ConvertedSource="$(addsuffix .F90, $(DSC)/$*/$(notdir $<))"; \
		ObjectFile="$(addsuffix .o, $(DSC)/$*/$(notdir $<))"; \
		$(CP) $< $$ConvertedSource; \
		$(FC) $(FFLAGS) $(INC) $(DEFS) -J$(DSC)/$* -c $$ConvertedSource -o $$ObjectFile; \
		$(NM) $(NMFLAGS) $$ObjectFile | $(AWK) '{print $$3}' > $@

# Fortran test file compilation.
TESTSRCS += $(wildcard $(DIR)/*.cf)
TESTOBJS += $(patsubst $(DIR)/%.cf,$(TESTOBJ)/%.o,$(wildcard $(DIR)/*.cf))

$(TESTOBJ)/%.o: $(DIR)/%.cf $(FCOBJS) | $(TESTOBJ) $(DSC)
		$(info Fortran Test: $<) $(MKDIR) $(DSC)/$* ; \
		ConvertedSource="$(addsuffix .f90, $(DSC)/$*/$(notdir $<))"; \
		CObjectFile="$(TESTOBJ)/$(notdir $(basename $<)).co"; \
		FObjectFile="$(TESTOBJ)/$(notdir $(basename $<)).fo"; \
		$(CPP)          $(INC) $(TESTINC) $(DEFS) -DTSD_FORTRAN_IMPL   $<  > $$ConvertedSource; \
		$(CC) $(CFLAGS) $(INC) $(TESTINC) $(DEFS) -DTSD_KERNEL -x c -c $< -o $$CObjectFile; \
		$(FC) $(FFLAGS) $(INC) $(TESTINC) $(DEFS) -J$(DSC)/$* -c $$ConvertedSource -o $$FObjectFile; \
		$(CC) -r $$CObjectFile $$FObjectFile -o $@; \
		$(OBJCOPY) $(WEAKEN_SYMBOLS) $@

$(TESTOBJ)/%.d: $(DIR)/%.cf | $(TESTOBJ)
		$(info Dependency: $<) $(DEPSCRIPT) $< $(TESTOBJ) -DTSD_KERNEL > $@ || $(RM) $@
		$(DEPSCRIPT) $< $(TESTOBJ) -DTSD_FORTRAN_IMPL >> $@ || $(RM) $@

DIR :=
