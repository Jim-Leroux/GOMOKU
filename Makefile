# ==============================================================================
# GOMOKU - WebAssembly & React Makefile
# ==============================================================================

SRC_DIR		= src
OBJ_DIR		= obj
INC_DIR		= include
TEST_DIR	= tests

# Engine sources
ENGINE_SRCS	= $(SRC_DIR)/Board.cpp $(SRC_DIR)/GameEngine.cpp $(SRC_DIR)/AI.cpp
ENGINE_OBJS	= $(ENGINE_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/native/%.o)
WASM_SRCS	= $(ENGINE_SRCS) $(SRC_DIR)/WasmBindings.cpp
WASM_OBJS	= $(WASM_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/wasm/%.o)

# Native test binary (no SFML / WASM dependency)
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -O2 -MMD -MP
TEST_NAME	= test_gomoku
TEST_SRCS	= $(filter-out $(TEST_DIR)/perf_main.cpp $(TEST_DIR)/test_ai_depth10.cpp, \
				$(shell find $(TEST_DIR) -name '*.cpp'))
TEST_OBJS	= $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/$(TEST_DIR)/%.o)

PERF_NAME	= perf_gomoku
PERF_OBJS	= $(ENGINE_OBJS) \
			  $(OBJ_DIR)/$(TEST_DIR)/perf_main.o \
			  $(OBJ_DIR)/$(TEST_DIR)/test_ai_depth10.o

WASM_NAME	= gomoku.js
WASM_CXX	= em++
WASM_FLAGS	= -std=c++17 -O3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=64MB -s MODULARIZE=1 -s EXPORT_NAME="createGomoku" --bind
INCLUDES	= -I$(INC_DIR)

GREEN		= \033[0;32m
YELLOW		= \033[0;33m
RESET		= \033[0m

# ==============================================================================
# Commands
# ==============================================================================

all: run-web

run-web:
	@echo "$(GREEN)Building WASM with Docker for local dev...$(RESET)"
	if docker --version | grep -qi podman; then \
		docker run --rm -v "$$(pwd):/src:z" emscripten/emsdk make wasm; \
	else \
		docker run --rm -v "$$(pwd):/src:z" -u $$(id -u):$$(id -g) emscripten/emsdk make wasm; \
	fi
	@echo "$(GREEN)Copying WASM files to React public folder...$(RESET)"
	cp $(WASM_NAME) gomoku.wasm web/public/
	@echo "$(GREEN)Starting React development server...$(RESET)"
	npm --prefix web run dev

run-prod:
	@echo "$(GREEN)Building and starting Production Docker Compose...$(RESET)"
	docker compose up --build -d
	@echo "$(GREEN)Production server running on http://localhost:8080$(RESET)"

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(ENGINE_OBJS) $(TEST_OBJS)
	@echo "$(GREEN)Linking $(TEST_NAME)...$(RESET)"
	$(CXX) $(CXXFLAGS) $(ENGINE_OBJS) $(TEST_OBJS) -o $(TEST_NAME)
	@echo "$(GREEN)✓ $(TEST_NAME) built successfully$(RESET)"

$(OBJ_DIR)/native/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(TEST_DIR) -c $< -o $@

perf: $(PERF_NAME)
	./$(PERF_NAME)

$(PERF_NAME): $(PERF_OBJS)
	@echo "$(GREEN)Linking $(PERF_NAME)...$(RESET)"
	$(CXX) $(CXXFLAGS) $(PERF_OBJS) -o $(PERF_NAME)

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(YELLOW)Removing binaries...$(RESET)"
	rm -f $(WASM_NAME) gomoku.wasm $(TEST_NAME) $(PERF_NAME)

re: fclean all

# ==============================================================================
# Internal Compilation Rules (used by Docker)
# ==============================================================================

wasm: $(WASM_NAME)

$(WASM_NAME): $(WASM_OBJS)
	@echo "$(GREEN)Linking $(WASM_NAME) with Emscripten...$(RESET)"
	$(WASM_CXX) $(WASM_FLAGS) $(INCLUDES) $(WASM_OBJS) -o $(WASM_NAME)
	@echo "$(GREEN)✓ $(WASM_NAME) built successfully!$(RESET)"

$(OBJ_DIR)/wasm/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling WASM $<...$(RESET)"
	$(WASM_CXX) $(WASM_FLAGS) $(INCLUDES) -c $< -o $@

-include $(WASM_OBJS:.o=.d) $(ENGINE_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all run-web run-prod test perf clean fclean re wasm
