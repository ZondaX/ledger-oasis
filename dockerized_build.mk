#*******************************************************************************
#*   (c) 2019 ZondaX GmbH
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

LEDGER_SRC=$(CURDIR)/app
DOCKER_APP_SRC=/project

DOCKER_IMAGE=zondax/ledger-docker-bolos:latest
DOCKER_BOLOS_SDK=/project/deps/nanos-secure-sdk
DOCKER_BOLOS_SDKX=/project/deps/nano2-sdk

SCP_PUBKEY=049bc79d139c70c83a4b19e8922e5ee3e0080bb14a2e8b0752aa42cda90a1463f689b0fa68c1c0246845c2074787b649d0d8a6c0b97d4607065eee3057bdf16b83
SCP_PRIVKEY=ff701d781f43ce106f72dc26a46b6a83e053b5d07bb3d4ceab79c91ca822a66b

define run_docker
	docker run -it --rm \
	--privileged \
	-e SCP_PRIVKEY=$(SCP_PRIVKEY) \
	-e BOLOS_SDK=$(1) \
	-e BOLOS_ENV=/opt/bolos \
	-p 1234:1234 \
	-p 8001:8001 \
	-p 9998-9999:9998-9999 \
	-u $(shell id -u) \
	-v $(shell pwd):/project \
	-e DISPLAY=$(shell echo ${DISPLAY}) \
	-v /tmp/.X11-unix:/tmp/.X11-unix:ro \
	$(DOCKER_IMAGE) \
	"$(2)"
endef

.PHONY: all deps build clean load delete
all: build

pull:
	docker pull $(DOCKER_IMAGE)

build:
	$(info Replacing app icon)
	@cp $(LEDGER_SRC)/nanos_icon.gif $(LEDGER_SRC)/glyphs/icon_app.gif
	$(info calling make inside docker)
	$(call run_docker,$(DOCKER_BOLOS_SDK),make -C $(DOCKER_APP_SRC))

buildX:
	@cp $(LEDGER_SRC)/nanos_icon.gif $(LEDGER_SRC)/glyphs/icon_app.gif
	@convert $(LEDGER_SRC)/nanos_icon.gif -crop 14x14+1+1 +repage -negate $(LEDGER_SRC)/nanox_icon.gif
	$(call run_docker,$(DOCKER_BOLOS_SDKX),make -C $(DOCKER_APP_SRC))

clean:
	$(call run_docker,$(DOCKER_BOLOS_SDK),make -C $(DOCKER_APP_SRC) clean)

shell:
	$(call run_docker,$(DOCKER_BOLOS_SDK) -t,bash)

debug: build
	$(call run_docker,$(DOCKER_BOLOS_SDK),/home/test/speculos/speculos.py -d -n -t $(DOCKER_APP_SRC)/bin/app.elf)

emu: build
	$(call run_docker,$(DOCKER_BOLOS_SDK),/home/test/speculos/speculos.py -o -z 3 -v 8001 $(DOCKER_APP_SRC)/bin/app.elf)

deps:
	@echo "Install dependencies"
	$(CURDIR)/install_deps.sh

load:
	${LEDGER_SRC}/pkg/zxtool.sh load

delete:
	${LEDGER_SRC}/pkg/zxtool.sh delete

# This target will initialize the device with the integration testing mnemonic
dev_init:
	@echo "Initializing device with test mnemonic! WARNING TAKES 2 MINUTES AND REQUIRES RECOVERY MODE"
	@python -m ledgerblue.hostOnboard --apdu --id 0 --prefix "" --passphrase "" --pin 5555 --words "equip will roof matter pink blind book anxiety banner elbow sun young"

# This target will setup a custom developer certificate
dev_ca:
	@python -m ledgerblue.setupCustomCA --targetId 0x31100004 --public $(SCP_PUBKEY) --name zondax

dev_ca_delete:
	@python -m ledgerblue.resetCustomCA --targetId 0x31100004

# This target will setup a custom developer certificate
dev_ca2:
	@python -m ledgerblue.setupCustomCA --targetId 0x33000004 --public $(SCP_PUBKEY) --name zondax

dev_ca_delete2:
	@python -m ledgerblue.resetCustomCA --targetId 0x33000004

update_zxlib:
	rsync -a --exclude='.git/' ../ledger-zxlib $(CURDIR)/deps
