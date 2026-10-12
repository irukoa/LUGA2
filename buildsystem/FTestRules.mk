# Fortran test file compilation.
TESTSRCS += $(wildcard $(DIR)/*.cf)
TESTOBJS += $(patsubst $(DIR)/%.cf,$(TESTOBJ)/%.o,$(wildcard $(DIR)/*.cf))

$(TESTOBJ)/%.o: $(DIR)/%.cf $(FMODDEFS) | $(TESTOBJ) $(DSC)
		$(info $(call cls,Fortran Test Object: $<)) $(MKDIR) $(DSC)/$* ; \
		ConvertedSource="$(addsuffix .f90, $(DSC)/$*/$(notdir $<))"; \
		CObjectFile="$(TESTOBJ)/$(notdir $(basename $<)).co"; \
		FObjectFile="$(TESTOBJ)/$(notdir $(basename $<)).fo"; \
		$(CPP)          $(INC) $(TESTINC) $(DEFS) -DTSD_FORTRAN_IMPL   $<  > $$ConvertedSource; \
		$(CC) $(CFLAGS) $(INC) $(TESTINC) $(DEFS) -DTSD_KERNEL -x c -c $< -o $$CObjectFile; \
		$(FC) $(FFLAGS) $(INC) $(TESTINC) $(DEFS) -J$(DSC)/$* -c $$ConvertedSource -o $$FObjectFile; \
		$(CC) -r $$CObjectFile $$FObjectFile -o $@; \
		$(OBJCOPY) $(WEAKEN_SYMBOLS) $@

$(TESTOBJ)/%.d: $(DIR)/%.cf | $(TESTOBJ)
		$(info $(call cls,Dependency: $<)) $(DEPSCRIPT) $< $(TESTOBJ) -DTSD_KERNEL > $@ || $(RM) $@
		$(DEPSCRIPT) $< $(TESTOBJ) -DTSD_FORTRAN_IMPL >> $@ || $(RM) $@
