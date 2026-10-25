# Fortran compiler-defined objects in interface-only files.

FTESTMODDEFS :=

DIR := test/CoreSupport
include buildsystem/FModuleTestDefs.mk

WEAKEN_TEST_SYMBOLS = $(foreach def, $(FTESTMODDEFS), --weaken-symbols=$(def))

# Get module objects.

DIR := test/CoreSupport
include buildsystem/CTestRules.mk
include buildsystem/FTestRules.mk

DIR := test/Suites/MAC
include buildsystem/CTestRules.mk
include buildsystem/FTestRules.mk

DIR := test/Suites/EINT
include buildsystem/CTestRules.mk
include buildsystem/FTestRules.mk

DIR := test/Suites/LDW
include buildsystem/CTestRules.mk
include buildsystem/FTestRules.mk

DIR := test/Suites/GRTP
include buildsystem/CTestRules.mk
include buildsystem/FTestRules.mk

DIR := test/Suites/TBCI
include buildsystem/CTestRules.mk
include buildsystem/FTestRules.mk
