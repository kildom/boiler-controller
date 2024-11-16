
CLANG_HOME = ../../ext/wasi-sdk-21.0

TARGET = ../../dist_simu/model.wasm
#BUILD_TYPE := debug
BUILD_TYPE := release

OBJ_DIR := ../../build/wasm_obj

ALLFLAGS := -fno-exceptions -mno-atomics -mno-exception-handling -mno-extended-const -mno-mutable-globals

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

OBJ := $(patsubst ../../simu/wasm/%.cc,$(OBJ_DIR)/simu/wasm/%.cc.o,$(wildcard ../../simu/wasm/*.cc))
OBJ += $(patsubst ../../simu/model/%.cc,$(OBJ_DIR)/simu/model/%.cc.o,$(wildcard ../../simu/model/*.cc))
OBJ += $(patsubst ../../src/control/%.cc,$(OBJ_DIR)/src/control/%.cc.o,$(wildcard ../../src/control/*.cc))

all: $(TARGET)

clean:
	rm -Rf $(OBJ_DIR)
	rm -f $(TARGET)
	rm -f ../../simu/app/model-struct-generated.ts

$(TARGET): $(OBJ) ../../simu/app/model-struct-generated.ts Makefile
	mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(ALLFLAGS) $(OBJ) -o $@

../../simu/app/model-struct-generated.ts: ../../simu/model/model.hh ../../scripts/build-struct.ts Makefile
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

../../build/include/autogen.inc: $(wildcard ../../src/control/*.hh) ../../scripts/autogen-inc.ts Makefile
	npx tsx ../../scripts/autogen-inc.ts $(wildcard ../../src/control/*.hh) ../../scripts/autogen-inc.ts Makefile

-include $(patsubst %.o,%.d,$(OBJ))
