.PHONY: all
all: build

CXX=clang++

fqbn := $(strip $(shell cat .board_name 2>/dev/null)) # make v3 doesn't support "file"
fqbn := $(strip $(fqbn))
ifeq ($(fqbn),)
	fqbn := arduino:renesas_uno:minima
endif
fqbn_path := $(subst :,.,$(fqbn))

.PHONY: build
build: BatterySpoof/build/$(fqbn_path)/BatterySpoof.ino.elf Can/build/$(fqbn_path)/Can.ino.elf

space := $() $()

.PHONY: init
init:
	arduino-cli core install $(subst $(space),:,$(wordlist 1,2,$(subst :, ,$(fqbn))))

extra_compilation_flags =
ifdef OSCILLATE_VOLTAGE
	extra_compilation_flags += -DOSCILLATE_VOLTAGE
endif
ifdef DEBUG_LOG
	extra_compilation_flags += -DDEBUG_LOG
endif
ifdef INTERACTIVE
	extra_compilation_flags += -DINTERACTIVE
endif

BatterySpoof/build/$(fqbn_path)/BatterySpoof.ino.elf: BatterySpoof/BatterySpoof.ino BatterySpoof/spoof.hpp BatterySpoof/spoof.cpp BatterySpoof/interactive.hpp BatterySpoof/interactive.cpp
	arduino-cli compile --fqbn $(fqbn) BatterySpoof --export-binaries --warnings all \
		--build-property compiler.cpp.extra_flags="$(extra_compilation_flags)"

Can/build/$(fqbn_path)/Can.ino.elf: Can/Can.ino
	arduino-cli compile --fqbn $(fqbn) Can --export-binaries --warnings all \
		--build-property compiler.cpp.extra_flags="$(extra_compilation_flags)"

.PHONY: install
install: build
	arduino-cli upload -p $$(arduino-cli board list | grep $(fqbn) | cut -f 1 -d ' ') \
		--fqbn $(fqbn) Can --build-path ./Can/build/$(fqbn_path)

.PHONY: monitor
monitor:
	arduino-cli monitor -p $$(arduino-cli board list | grep $(fqbn) | cut -f 1 -d ' ')

.PHONY: clean
clean:
	rm -rf BatterySpoof/build
	rm -rf BatterySpoof/test/build
	rm -rf Can/build

.PHONY: format
format:
	find BatterySpoof/ -iname '*.ino' -o -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
	find Can/ -iname '*.ino' -o -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i

.PHONY: check
check: format build test

.PHONY: test
test: build-test run-test

.PHONY: build-test
build-test: BatterySpoof/test/build/test

BatterySpoof/test/build/test: BatterySpoof/test/test.cpp BatterySpoof/spoof.hpp BatterySpoof/spoof.cpp BatterySpoof/interactive.hpp BatterySpoof/interactive.cpp
	mkdir -p BatterySpoof/test/build
	$(CXX) $(extra_compilation_flags) -o BatterySpoof/test/build/test BatterySpoof/test/test.cpp BatterySpoof/spoof.cpp BatterySpoof/interactive.cpp

.PHONY: run-test
run-test: build-test
	./BatterySpoof/test/build/test

.PHONY: list
list:
	arduino-cli board list
