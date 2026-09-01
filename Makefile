# ==============================================================================
# GOMOKU - WebAssembly & React Makefile
# ==============================================================================

SRC_DIR		= src
OBJ_DIR		= obj
INC_DIR		= include

# Engine sources
ENGINE_SRCS	= $(SRC_DIR)/Board.cpp $(SRC_DIR)/GameEngine.cpp
WASM_SRCS	= $(ENGINE_SRCS) $(SRC_DIR)/WasmBindings.cpp
WASM_OBJS	= $(WASM_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/wasm/%.o)

WASM_NAME	= gomoku.js
WASM_CXX	= em++
WASM_FLAGS	= -std=c++17 -O3 -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME="createGomoku" --bind
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
		docker run --rm -v "$$(pwd):/src" emscripten/emsdk make wasm; \
	else \
		docker run --rm -v "$$(pwd):/src" -u $$(id -u):$$(id -g) emscripten/emsdk make wasm; \
	fi
	@echo "$(GREEN)Copying WASM files to React public folder...$(RESET)"
	cp $(WASM_NAME) gomoku.wasm web/public/
	@echo "$(GREEN)Starting React development server...$(RESET)"
	npm --prefix web run dev

run-prod:
	@echo "$(GREEN)Building and starting Production Docker Compose...$(RESET)"
	docker compose up --build -d
	@echo "$(GREEN)Production server running on http://localhost:8080$(RESET)"

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(YELLOW)Removing binaries...$(RESET)"
	rm -f $(WASM_NAME) gomoku.wasm

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

-include $(WASM_OBJS:.o=.d)

.PHONY: all run-web run-prod clean fclean re wasm
