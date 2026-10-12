# Fortran compiler-defined objects in interface-only files.

FMODDEFS :=

DIR := src/MAC
include buildsystem/FModuleDefs.mk

DIR := src/EINT
include buildsystem/FModuleDefs.mk

WEAKEN_SYMBOLS = $(foreach def, $(FMODDEFS), --weaken-symbols=$(def))

# Get module objects.

DIR := src/Core
include buildsystem/CModuleRules.mk
include buildsystem/FModuleRules.mk

DIR := src/MAC
include buildsystem/CModuleRules.mk
include buildsystem/FModuleRules.mk

DIR := src/EINT
include buildsystem/CModuleRules.mk
include buildsystem/FModuleRules.mk
