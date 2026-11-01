.PHONY: build flash monitor ci clean fullclean menuconfig size shell

COMPOSE    := docker compose -f build/docker-compose.yml
IDF        := idf.py -B build/out
CACHE_DIRS := build/.cache/ccache build/.cache/component_manager
WIN_PYTHON ?= /mnt/c/Users/caser/.espressif/python_env/idf5.5_py3.11_env/Scripts/python.exe
BAUD       ?= 460800

$(CACHE_DIRS):
	mkdir -p $(CACHE_DIRS)

build: $(CACHE_DIRS)
	$(COMPOSE) run --rm idf $(IDF) build
ifdef PORT
	@$(MAKE) flash PORT=$(PORT)
else ifdef FLASH
	@$(MAKE) flash
endif

flash:
	@if [ -z "$(PORT)" ]; then \
		echo "Auto-detecting ESP32-S3 on Windows COM ports..."; \
		cd build/out && "$(WIN_PYTHON)" -m esptool --chip esp32s3 -b $(BAUD) \
			--before default_reset --after hard_reset write_flash "@flash_args"; \
	elif echo "$(PORT)" | grep -iq "^COM"; then \
		cd build/out && "$(WIN_PYTHON)" -m esptool --chip esp32s3 -p $(PORT) -b $(BAUD) \
			--before default_reset --after hard_reset write_flash "@flash_args"; \
	else \
		$(COMPOSE) run --rm --device $(PORT):$(PORT) idf $(IDF) -p $(PORT) -b $(BAUD) flash; \
	fi

monitor:
	@if [ -z "$(PORT)" ] || echo "$(PORT)" | grep -iq "^COM"; then \
		"$(WIN_PYTHON)" -m esp_idf_monitor $(if $(PORT),-p $(PORT),) build/out/biciver.elf; \
	else \
		$(COMPOSE) run --rm --device $(PORT):$(PORT) idf $(IDF) -p $(PORT) monitor; \
	fi

ci: $(CACHE_DIRS)
	$(COMPOSE) run --rm idf bash -c "rm -f build/sdkconfig sdkconfig && $(IDF) fullclean && $(IDF) build && $(IDF) size-components"

clean: $(CACHE_DIRS)
	$(COMPOSE) run --rm idf $(IDF) clean

fullclean: $(CACHE_DIRS)
	$(COMPOSE) run --rm idf $(IDF) fullclean

menuconfig: $(CACHE_DIRS)
	$(COMPOSE) run --rm idf $(IDF) menuconfig

size: $(CACHE_DIRS)
	$(COMPOSE) run --rm idf $(IDF) size-components

shell: $(CACHE_DIRS)
	$(COMPOSE) run --rm idf bash
