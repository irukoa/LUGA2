# Modules included in the main targets.

DIR := src/Core
include buildsystem/CModuleRules.mk

DIR := src/MAC
include buildsystem/CModuleRules.mk
include buildsystem/FModuleRules.mk
