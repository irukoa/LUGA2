# Fortran compiler-generated symbol weakening.
FCOBJS += $(patsubst $(DIR)/%.M90,$(OBJ)/%.def,$(wildcard $(DIR)/*.M90))

$(OBJ)/%.def: $(DIR)/%.M90 | $(OBJ) $(DSC)
		$(info $(call cls,Weak symbol list: $<)) $(MKDIR) $(DSC)/$*; \
		ConvertedSource="$(addsuffix .F90, $(DSC)/$*/$(notdir $<))"; \
		ObjectFile="$(addsuffix .o, $(DSC)/$*/$(notdir $<))"; \
		$(CP) $< $$ConvertedSource; \
		$(FC) $(FFLAGS) $(INC) $(DEFS) -J$(DSC)/$* -c $$ConvertedSource -o $$ObjectFile; \
		$(NM) $(NMFLAGS) $$ObjectFile | $(AWK) '{print $$3}' > $@

# Fortran source file compilation.
SRCS += $(wildcard $(DIR)/*.F90)
OBJS += $(patsubst $(DIR)/%.F90,$(OBJ)/%.o,$(wildcard $(DIR)/*.F90))

$(OBJ)/%.o: $(DIR)/%.F90 $(FCOBJS) | $(OBJ) $(DSC)
		$(info $(call cls,Fortran object: $<)) $(MKDIR) $(DSC)/$*
		$(FC) $(FFLAGS) $(INC) $(DEFS) -J$(DSC)/$* -c $< -o $@
		$(OBJCOPY) $(WEAKEN_SYMBOLS) $@

$(OBJ)/%.d: $(DIR)/%.F90 | $(OBJ)
		$(info $(call cls,Dependency: $<)) $(DEPSCRIPT) $< $(OBJ) > $@ || $(RM) $@
