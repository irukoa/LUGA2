# Fortran compiler-generated symbol weakening.
FTESTMODDEFS += $(patsubst $(DIR)/%.M90,$(TESTOBJ)/%.def,$(wildcard $(DIR)/*.M90))

$(TESTOBJ)/%.def: $(DIR)/%.M90 | $(TESTOBJ) $(DSC)
		$(info $(call cls,Weak symbol list: $<)) $(MKDIR) $(DSC)/$*; \
		ConvertedSource="$(addsuffix .F90, $(DSC)/$*/$(notdir $<))"; \
		ObjectFile="$(addsuffix .o, $(DSC)/$*/$(notdir $<))"; \
		$(CP) $< $$ConvertedSource; \
		$(FC) $(FFLAGS) $(INC) $(DEFS) -J$(DSC)/$* -c $$ConvertedSource -o $$ObjectFile; \
		echo "Definitions:" > $@;\
		$(NM) $(NMFLAGS) $$ObjectFile | $(AWK) '{print $$3}' >> $@
