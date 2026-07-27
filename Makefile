# === Compiler and flags ===
CXX ?= g++
JOBS ?= $(shell nproc 2>/dev/null || echo $${NUMBER_OF_PROCESSORS:-4})
USE_CCACHE ?= auto

ifeq ($(filter -j% --jobs=%,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(JOBS)
endif

ifeq ($(USE_CCACHE),auto)
CCACHE := $(shell command -v ccache 2>/dev/null)
ifneq ($(CCACHE),)
CXX := ccache $(CXX)
endif
else ifeq ($(USE_CCACHE),1)
CXX := ccache $(CXX)
endif

# generate dependency files (-MMD -MP), keep other flags the same
DEPFLAGS := -MMD -MP
CXXFLAGS := -std=c++20 -O2 -g -Wall -Wextra -Isrc -DGLM_ENABLE_EXPERIMENTAL $(DEPFLAGS)
MINGW_PREFIX ?= /mingw64
MSYSTEM ?= native
LDFLAGS := -L$(MINGW_PREFIX)/lib

# MSYS installations can expose MSYSTEM with extra whitespace or inherited
# tokens. A multi-word value breaks the pattern rule below into multiple
# targets, so keep a single, filesystem-safe build tag.
BUILD_TAG := $(firstword $(strip $(MSYSTEM)))
ifeq ($(BUILD_TAG),)
BUILD_TAG := native
endif
ifeq ($(NOISE_TOOL),1)
CXXFLAGS += -DNOISE_TOOL
BUILD_TAG := $(BUILD_TAG)-noise
endif

# === Source layout ===
SRC_DIR := src
OBJ_DIR := obj/$(BUILD_TAG)
BIN_DIR := bin/$(BUILD_TAG)
TARGET := $(BIN_DIR)/voxel-game

# === Find source files ===
SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(wildcard $(SRC_DIR)/*/*.cpp) \
        $(wildcard $(SRC_DIR)/*/*/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

# === Linker flags ===
LIBS := -lglfw3 -lglew32 -lopengl32 -ldbghelp

# === Build rules ===
.PHONY: all clean run noisetool install

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

noisetool:
	$(MAKE) NOISE_TOOL=1 run

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	rm -f $(DEPS)

install:
	pacman -Syu
	pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-glew mingw-w64-x86_64-opengl

-include $(DEPS)
