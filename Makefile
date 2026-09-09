CMAKE ?= cmake
BUILD_DIR ?= build
.PHONY: all configure run clean test
all: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel
configure:
	$(CMAKE) -S . -B $(BUILD_DIR)
run: all
	./$(BUILD_DIR)/saturday
test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure
clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean
