DIR := src/Core

# C source file compilation.
SRCS += $(wildcard $(DIR)/*.c)
OBJS += $(patsubst $(DIR)/%.c,$(OBJ)/%.o,$(wildcard $(DIR)/*.c))

$(OBJ)/%.o: $(DIR)/%.c | $(OBJ)
		$(info C Object: $<) $(CC) $(CFLAGS) $(INC) $(DEFS) -c $< -o $@

$(OBJ)/%.d: $(DIR)/%.c | $(OBJ)
		$(info Dependency: $<) $(DEPSCRIPT) $< $(OBJ) > $@ || $(RM) $@

DIR :=
