CXX      := g++
CXXFLAGS := -std=c++17 -O2 -g -Wall -Wextra -pedantic
INCLUDES := -Iinclude

BUILD := build
SRC   := $(wildcard src/*.cpp)
OBJ   := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRC))
APP   := 02_conflict_check

all: $(APP)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(APP): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

run: $(APP)
	./$(APP)

clean:
	rm -rf $(BUILD) $(APP)

.PHONY: all run clean
