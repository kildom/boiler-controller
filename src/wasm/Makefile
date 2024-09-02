
CLANG_HOME = ../../ext/wasi-sdk-21.0

TARGET = ../../dist/model.wasm
BUILD_TYPE := debug
#BUILD_TYPE := release

OBJ_DIR := ../../build/wasm_obj

ALLFLAGS := -fno-exceptions

CFLAGS := -I../../build/include

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

OBJ := $(patsubst ../../src/wasm/%.cc,$(OBJ_DIR)/src/wasm/%.cc.o,$(wildcard ../../src/wasm/*.cc))
OBJ += $(patsubst ../../src/model/%.cc,$(OBJ_DIR)/src/model/%.cc.o,$(wildcard ../../src/model/*.cc))
OBJ += $(patsubst ../../src/control/%.cc,$(OBJ_DIR)/src/control/%.cc.o,$(wildcard ../../src/control/*.cc))
#OBJ += $(patsubst ../../firmware/%.c,$(OBJ_DIR)/firmware/%.c.o,$(wildcard ../../firmware/*.c))
#OBJ += $(patsubst ../../firmware/%.cc,$(OBJ_DIR)/firmware/%.cc.o,$(wildcard ../../firmware/*.cc))

all: $(TARGET)

clean:
	rm -Rf $(OBJ_DIR)
	rm -f $(TARGET)
	rm -f ../../src/graph/struct.ts

$(TARGET): $(OBJ) ../../src/graph/struct.ts Makefile
	mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(ALLFLAGS) $(OBJ) -o $@

../../src/graph/struct.ts: ../../src/model/model.hh ../../scripts/build-struct.ts Makefile
	npx tsx ../../scripts/build-struct.ts

$(OBJ_DIR)/%.cpp.o: ../../%.cpp ../../build/include/autogen.inc Makefile
	mkdir -p $(dir $@)
	$(CXX) -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

$(OBJ_DIR)/%.cc.o: ../../%.cc ../../build/include/autogen.inc Makefile
	mkdir -p $(dir $@)
	$(CXX) -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

$(OBJ_DIR)/%.c.o: ../../%.c ../../build/include/autogen.inc Makefile
	mkdir -p $(dir $@)
	$(CC) -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

# ../../build/include/autogen.inc: $(wildcard ../control/*.hh) ../../scripts/autogen.ts Makefile
# 	npx tsx ../../scripts/autogen.ts

-include $(patsubst %.o,%.d,$(OBJ))
