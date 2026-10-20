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
