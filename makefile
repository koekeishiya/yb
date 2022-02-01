FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework CoreServices -framework SkyLight
BUILD_FLAGS    = -std=c99 -Wall -g -O0 -fvisibility=hidden -mmacosx-version-min=10.13 -fno-objc-arc -arch x86_64 -arch arm64
BUILD_PATH     = ./bin
SRC            = ./src/manifest.c
BINS           = $(BUILD_PATH)/yb

.PHONY: all clean install

all: clean $(BINS)

install: BUILD_FLAGS=-std=c99 -Wall -DNDEBUG -O2 -fvisibility=hidden -mmacosx-version-min=10.13 -fno-objc-arc -arch x86_64 -arch arm64
install: clean $(BINS)

clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/yb: $(SRC)
	mkdir -p $(BUILD_PATH)
	xcrun clang $^ $(BUILD_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@
