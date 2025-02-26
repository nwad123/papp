BUILD_DIR ?= build
RESULT_DIR ?= results
CPU_ID ?= 3

build: build_ 
	cmake --build ${BUILD_DIR}

.PHONY: build_ 
build_: build_dir result_dir_
	cd $(BUILD_DIR); cmake ..

# Generates a build directory
.PHONY: build_dir
build_dir:
	@mkdir -p $(BUILD_DIR);

# Cleans all build files
.PHONY: clean
clean:
	@echo "Removing build directory (${BUILD_DIR})..."
	@rm -rf ${BUILD_DIR}
	@rm -rf .cache

# creates the result directory
.PHONY: result_dir_
result_dir_: 
	@mkdir -p ${RESULT_DIR}
