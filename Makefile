CC := clang++
CXXFLAGS := -g -Wall -Wextra -Wshadow -Weffc++ -Wreorder -Wmissing-declarations -Wextra-semi -Wsign-conversion	-Wswitch-default  -Wuninitialized -Wnull-dereference -Wdouble-promotion -Wnon-virtual-dtor -pedantic -Wunused-function -Wold-style-cast -Wunused -Woverloaded-virtual -Wconversion -std=c++2b -g  


SRC_DIR := ./src
OBJ_DIR := ./obj

SRC_FILES := $(shell find $(SRC_DIR) -type f -name "*.cpp")
HEADERS := $(shell find $(SRC_DIR) -type f -name "*.hpp")

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
DEP_FILES := $(OBJ_FILES:.o=.d)
PROGRAM_NAME := ac

.PHONY: all clean format run

all: $(PROGRAM_NAME)

$(PROGRAM_NAME): $(OBJ_FILES)
	$(CC) $(CXXFLAGS) $(OBJ_FILES) -o $@

-include $(DEP_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) -MMD -c $< -o $@

lint:
	clang-tidy  $(SRC_FILES) -- $(CXXFLAGS)

format:
	astyle -xe --style=google --attach-return-type --align-pointer=name --indent=spaces=4 \
	--max-code-length=120 --break-after-logical --suffix=none $(SRC_FILES) $(HEADERS)

clean:
	@rm -f $(PROGRAM_NAME)
	@rm -rf $(OBJ_DIR)

