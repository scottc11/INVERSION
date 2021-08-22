DOCKER_IMAGE := ok:mbed
BUILD := BUILD

.PHONY: default
default: build

.PHONY: docker.image
docker.image:
	docker build --tag ${DOCKER_IMAGE} .
	clear

.PHONY: docker.cmd
docker.cmd: docker.image
	docker run \
		--mount type=bind,source="$(shell pwd)",target=/ok \
		${DOCKER_IMAGE} \
		${DOCKER_CMD}

.PHONY: build
build: DOCKER_CMD := mbed compile
build: docker.cmd


######################################
# OpenOCD stuff
######################################

# name of .elf file
TARGET = ok-degree

# openodc .cfg file name
CHIPSET ?= stm32f4x
FLASH_ADDRESS ?= 0x08000000

OCD=openocd
OCD_DIR ?= /usr/local/share/openocd/scripts
PGM_DEVICE ?= interface/stlink.cfg
OCDFLAGS = -f $(PGM_DEVICE) -f target/$(CHIPSET).cfg

program:
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS) \
		-c "program ./BUILD/OK_BOARD_F446RE/GCC_ARM/$(TARGET).elf verify reset exit"