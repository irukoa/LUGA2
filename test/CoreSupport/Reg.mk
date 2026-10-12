DIR := test/CoreSupport

# C source file compilation.
TESTSRCS += $(wildcard $(DIR)/*.c)
TESTOBJS += $(patsubst $(DIR)/%.c,$(TESTOBJ)/%.o,$(wildcard $(DIR)/*.c))

$(TESTOBJ)/%.o: $(DIR)/%.c | $(TESTOBJ)
		$(info C Object: $<) $(CC) $(CFLAGS) $(INC) $(DEFS) -c $< -o $@

$(TESTOBJ)/%.d: $(DIR)/%.c | $(TESTOBJ)
		$(info Dependency: $<) $(DEPSCRIPT) $< $(TESTOBJ) > $@ || $(RM) $@

DIR :=
