# Kompilátor
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src

# Složky
SRC_DIR = src
BUILD_DIR = build

# Soubory
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
TARGET = $(BUILD_DIR)/app

# Pravidla
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: all
	./$(TARGET)
