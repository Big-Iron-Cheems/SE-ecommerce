CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Wpedantic # -Werror
LDFLAGS := -lredis++ -lhiredis -lpqxx -lpq # Link to redis and postgresql (including C++ versions)

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Find all .cpp files in subdirectories
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
EXEC := $(BIN_DIR)/ecommerce

$(EXEC): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(EXEC)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: clean
