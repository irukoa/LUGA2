# Fortran source file compilation.
SRCS += $(wildcard $(DIR)/*.F90)
OBJS += $(patsubst $(DIR)/%.F90,$(OBJ)/%.o,$(wildcard $(DIR)/*.F90))

$(OBJ)/%.o: $(DIR)/%.F90 $(FMODDEFS) | $(OBJ) $(DSC)
		$(info $(call cls,Fortran object: $<)) $(MKDIR) $(DSC)/$*
		$(FC) $(FFLAGS) $(INC) $(DEFS) -J$(DSC)/$* -c $< -o $@
		$(OBJCOPY) $(WEAKEN_SYMBOLS) $@

$(OBJ)/%.d: $(DIR)/%.F90 | $(OBJ)
		$(info $(call cls,Dependency: $<)) $(DEPSCRIPT) $< $(OBJ) > $@ || $(RM) $@
