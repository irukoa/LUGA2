# Header.
MKDIR := mkdir -p
CP    := cp
MV    := mv
RM    := rm -rf

# Profile.
PROFILE := debug

# Environment.
OBJ := obj
BIN := bin
DSC := discard

# Rules
.PHONY: main clean
.DEFAULT_GOAL := main
ifeq ($(MAKECMDGOALS), )
override MAKECMDGOALS := main
endif

# Compiler, flags, definitions and linker options.
CC         := gcc
FC         := gfortran
CPP        := cpp
DEPSCRIPT  := ./tools/GetDeps.sh
ifeq ($(PROFILE), debug)
CFLAGS := -g -Wall -O0 -Wextra -pedantic -std=gnu23 --coverage -fPIC
FFLAGS := -g -Wall -O0 -Wextra -pedantic -std=gnu   --coverage -fPIC -Wno-c-binding-type
DEFS   := -DTESTABLE
else
CFLAGS := -Werror -O3 -std=gnu23 -fPIC -fopenmp
FFLAGS := -Werror -O3 -std=gnu   -fPIC -fopenmp
DEFS   := -DNDEBUG
endif
LDFLAGS :=
LDLIBS  :=

# Binutils
NM      := nm
AR      := ar
ARFLAGS := rcs
AWK     := awk
NMFLAGS := -g --defined-only
OBJCOPY := objcopy

# Dependency graph.
#	- Source code.
SRCS :=
#	- Include directories.
INC := -I./
# - Objects.
OBJS :=
# Fortran compiler-defined objects in interface-only files.
FCOBJS :=
# - Graph creation is delegated to modules in registry.
include ModuleRegistry.mk
# - Dependency inclusion.
DEPS := $(patsubst $(OBJ)/%.o,$(OBJ)/%.d,$(OBJS))
ifneq ($(filter-out clean, $(MAKECMDGOALS)), )
include $(DEPS)
endif
WEAKEN_SYMBOLS = $(foreach def, $(FCOBJS), --weaken-symbols=$(def))

# Rule for the library.
LIBST := $(BIN)/libLUGA.a
LIBDY := $(BIN)/libLUGA.so

main: $(LIBST) $(LIBDY)
$(LIBST): $(OBJS) | $(BIN)
		$(info Binary: $@) $(AR) $(ARFLAGS) $@ $^
$(LIBDY): $(OBJS) | $(BIN)
		$(info Binary: $@) $(FC) $(FFLAGS) $^ -shared -o $@ -Wl,-soname,libLUGA.so $(LDFLAGS) $(LDLIBS)

ifeq ($(PROFILE), debug)
# Testing profile.
TESTGOALS := test runtest coverage clean
ifneq ($(filter $(TESTGOALS), $(MAKECMDGOALS)), )
include test/Test.mk
endif
endif

$(OBJ) $(BIN) $(DSC):
		$(info Directory: $@) $(MKDIR) $@

clean::
		$(RM) $(OBJ) $(BIN) $(DSC)
