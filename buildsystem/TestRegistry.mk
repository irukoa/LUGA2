DIR := test/CoreSupport
include buildsystem/CTestRules.mk

DIR := test/Suites/MAC
include buildsystem/CTestRules.mk
include buildsystem/FTestRules.mk
