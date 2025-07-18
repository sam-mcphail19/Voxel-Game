# === Compiler and flags ===
CXX := g++
CXXFLAGS := -std=c++20 -O2 -g -Wall -Wextra -Isrc -DGLM_ENABLE_EXPERIMENTAL
LDFLAGS := -L/mingw64/lib

# === Source layout ===
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TARGET := $(BIN_DIR)/voxel-game

# === Find source files ===
SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(wildcard $(SRC_DIR)/*/*.cpp) \
        $(wildcard $(SRC_DIR)/*/*/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# === Linker flags ===
LIBS := -lglfw3 -lglew32 -lopengl32 -ldbghelp

# === Build rules ===
.PHONY: all clean run install

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

install:
	pacman -Syu
	pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-glew mingw-w64-x86_64-opengl
