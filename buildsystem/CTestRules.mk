# C test file compilation.
TESTSRCS += $(wildcard $(DIR)/*.c)
TESTOBJS += $(patsubst $(DIR)/%.c,$(TESTOBJ)/%.o,$(wildcard $(DIR)/*.c))

$(TESTOBJ)/%.o: $(DIR)/%.c | $(TESTOBJ)
		$(info $(call cls,C Test Object: $<)) $(CC) $(CFLAGS) $(INC) $(TESTINC) $(DEFS) -c $< -o $@

$(TESTOBJ)/%.d: $(DIR)/%.c | $(TESTOBJ)
		$(info $(call cls,Dependency: $<)) $(DEPSCRIPT) $< $(TESTOBJ) > $@ || $(RM) $@
