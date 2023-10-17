
CLANG_HOME = /home/doki/my/ZeroTierOne-1.10.4/tools/wasi-sdk-19.0

TARGET = ../web/model.wasm
BUILD_TYPE := debug
#BUILD_TYPE := release

OBJ_DIR := obj

ALLFLAGS := -fno-exceptions

CFLAGS := -I../../lowlevel -I../../firmware

LDFLAGS := -mexec-model=reactor

ifeq ($(BUILD_TYPE),debug)
  CFLAGS += -DDEBUG=1
  ALLFLAGS += -O0 -g -gdwarf-5 -gpubnames
else
  CFLAGS += -DNDEBUG=1
  ALLFLAGS += -O3 -flto
endif

CC=$(CLANG_HOME)/bin/clang --sysroot $(CLANG_HOME)/share/wasi-sysroot
CXX=$(CLANG_HOME)/bin/clang++ --sysroot $(CLANG_HOME)/share/wasi-sysroot

OBJ := $(patsubst ../../simu/model/%.cpp,$(OBJ_DIR)/simu/model/%.cpp.o,$(wildcard ../../simu/model/*.cpp))
#OBJ += $(patsubst ../../firmware/%.c,$(OBJ_DIR)/firmware/%.c.o,$(wildcard ../../firmware/*.c))
OBJ += $(patsubst ../../firmware/%.cc,$(OBJ_DIR)/firmware/%.cc.o,$(wildcard ../../firmware/*.cc))

all: $(TARGET)

clean:
	rm -Rf $(OBJ_DIR)
	rm -f $(TARGET)

$(TARGET): $(OBJ) Makefile
	mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(ALLFLAGS) $(OBJ) -o $@
	npx ts-node-esm build-struct.ts

$(OBJ_DIR)/%.cpp.o: ../../%.cpp Makefile
	mkdir -p $(dir $@)
	$(CXX) -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

$(OBJ_DIR)/%.cc.o: ../../%.cc Makefile
	mkdir -p $(dir $@)
	$(CXX) -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

$(OBJ_DIR)/%.c.o: ../../%.c Makefile
	mkdir -p $(dir $@)
	$(CC) -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

-include $(patsubst %.o,%.d,$(OBJ))
