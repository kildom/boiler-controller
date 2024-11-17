
ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),MINGW)
        EXE_EXT := .exe
    else ifeq ($(UNAME_S),MSYS)
        EXE_EXT := .exe
    else
        EXE_EXT :=
    endif
endif

TARGET = ../../build/test/unit$(EXE_EXT)

OBJ_DIR := ../../build/test/obj

ALLFLAGS :=

CFLAGS := \
	-Igtest/googletest/include \
	-Igtest/googletest \
	-Igtest/googlemock/include \
	-Igtest/googlemock \
	-I../../src/control \
	-I../../build/include

LDFLAGS :=

CFLAGS += -DDEBUG=1
ALLFLAGS += -O0 -g

OBJ := $(patsubst ./%.cpp,$(OBJ_DIR)/%.cpp.o,$(wildcard ./*.cpp))
OBJ += $(OBJ_DIR)/gtest/googletest/src/gtest-all.cc.o
OBJ += $(OBJ_DIR)/gtest/googlemock/src/gmock-all.cc.o
OBJ += $(OBJ_DIR)/gtest/googlemock/src/gmock_main.cc.o

all: $(TARGET)

clean:
	rm -Rf $(OBJ_DIR)
	rm -f $(TARGET)

run: $(TARGET)
	$(TARGET)

$(TARGET): $(OBJ) Makefile
	g++ $(LDFLAGS) $(ALLFLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%.cpp.o: ./%.cpp Makefile
	mkdir -p $(dir $@)
	g++ -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

$(OBJ_DIR)/%.cc.o: ./%.cc Makefile
	mkdir -p $(dir $@)
	g++ -MMD -c $(CFLAGS) $(ALLFLAGS) $(word 1,$<) -o $@

-include $(patsubst %.o,%.d,$(OBJ))
