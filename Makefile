#*******************************************************************************
#*   (c) 2019 Zondax GmbH
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************

# We use BOLOS_SDK to determine the development environment that is being used
# BOLOS_SDK IS  DEFINED	 	We use the plain Makefile for Ledger
# BOLOS_SDK NOT DEFINED		We use a containerized build approach

TESTS_JS_PACKAGE = "@zondax/ledger-oasis"
TESTS_JS_DIR = $(CURDIR)/js

ifeq ($(BOLOS_SDK),)
PRODUCTION_BUILD ?= 1
include $(CURDIR)/deps/ledger-zxlib/dockerized_build.mk
else
default:
	$(MAKE) -C app
%:
	$(info "Calling app Makefile for target $@")
	COIN=$(COIN) $(MAKE) -C app $@
endif

test_prepare:
	make
	make zemu_install

test_all:
	make
	make zemu_install
	make zemu_test

tests_tools_build:
	cd tests_tools/neon && yarn install

zemu_install: tests_tools_build

build_val: COIN=oasis_validator
build_val: buildS
	cp $(CURDIR)/app/bin/app.elf $(CURDIR)/app/bin/app_val.elf
