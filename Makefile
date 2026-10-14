NAME		= Gomoku

SRC_DIR		= src
OBJ_DIR		= obj
INC_DIR		= include
TEST_DIR	= tests

ENGINE_SRCS	= $(SRC_DIR)/Board.cpp $(SRC_DIR)/GameEngine.cpp $(SRC_DIR)/AI.cpp
ENGINE_OBJS	= $(ENGINE_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/native/%.o)
MAIN_OBJ	= $(OBJ_DIR)/native/main.o

WASM_SRCS	= $(ENGINE_SRCS) $(SRC_DIR)/WasmBindings.cpp
WASM_OBJS	= $(WASM_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/wasm/%.o)
WASM_NAME	= gomoku.js

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -O2 -MMD -MP
INCLUDES	= -I$(INC_DIR)

TEST_NAME	= test_gomoku
TEST_SRCS	= $(filter-out $(TEST_DIR)/perf_main.cpp $(TEST_DIR)/test_ai_depth10.cpp, \
				$(shell find $(TEST_DIR) -name '*.cpp'))
TEST_OBJS	= $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/$(TEST_DIR)/%.o)

PERF_NAME	= perf_gomoku
PERF_OBJS	= $(ENGINE_OBJS) \
			  $(OBJ_DIR)/$(TEST_DIR)/perf_main.o \
			  $(OBJ_DIR)/$(TEST_DIR)/test_ai_depth10.o

WASM_CXX	= em++
WASM_FLAGS	= -std=c++17 -O3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=64MB \
			  -s MODULARIZE=1 -s EXPORT_NAME="createGomoku" --bind

all: $(NAME)

$(NAME): $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $(MAIN_OBJ) -o $(NAME)

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(ENGINE_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(ENGINE_OBJS) $(TEST_OBJS) -o $(TEST_NAME)

perf: $(PERF_NAME)
	./$(PERF_NAME)

$(PERF_NAME): $(PERF_OBJS)
	$(CXX) $(CXXFLAGS) $(PERF_OBJS) -o $(PERF_NAME)

$(OBJ_DIR)/native/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(TEST_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(TEST_NAME) $(PERF_NAME) $(WASM_NAME) gomoku.wasm

re: fclean all

# Invoked by ./Gomoku via Docker (emscripten/emsdk image).
wasm: $(WASM_NAME)

$(WASM_NAME): $(WASM_OBJS)
	$(WASM_CXX) $(WASM_FLAGS) $(INCLUDES) $(WASM_OBJS) -o $(WASM_NAME)

$(OBJ_DIR)/wasm/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(WASM_CXX) $(WASM_FLAGS) $(INCLUDES) -c $< -o $@

-include $(WASM_OBJS:.o=.d) $(ENGINE_OBJS:.o=.d) $(MAIN_OBJ:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all re clean fclean test perf
