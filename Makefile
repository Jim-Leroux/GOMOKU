NAME		= Gomoku

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -O2 -MMD -MP

# Directories
SRC_DIR		= src
OBJ_DIR		= obj
INC_DIR		= include

# Sources
SRCS		= $(shell find $(SRC_DIR) -name '*.cpp')
OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Engine sources = everything except main.cpp.
# The test binary below links against these directly so it stays fast to build
# and has no GUI dependency.
ENGINE_SRCS	= $(filter-out $(SRC_DIR)/main.cpp, $(SRCS))
ENGINE_OBJS	= $(ENGINE_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TEST_DIR	= tests
TEST_NAME	= test_gomoku
TEST_SRCS	= $(shell find $(TEST_DIR) -name '*.cpp')
TEST_OBJS	= $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/$(TEST_DIR)/%.o)

# SFML (cross-platform: macOS via Homebrew, Linux via system paths)
OS			= $(shell uname -s)

ifeq ($(OS), Darwin)
    BREW_PREFIX	= $(shell brew --prefix sfml 2>/dev/null || echo /usr/local)
    SFML_INC	= $(BREW_PREFIX)/include
    SFML_LIB	= $(BREW_PREFIX)/lib
    SFML_FLAGS	= -L$(SFML_LIB) -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
else
    # Linux: SFML installée via apt/pacman, headers et libs dans les paths système
    SFML_INC	=
    SFML_LIB	=
    SFML_FLAGS	= -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
endif

# Includes
INCLUDES	= -I$(INC_DIR) $(if $(SFML_INC),-I$(SFML_INC),)

# Colors
GREEN		= \033[0;32m
YELLOW		= \033[0;33m
RESET		= \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)Linking $(NAME)...$(RESET)"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(SFML_FLAGS)
	@echo "$(GREEN)✓ $(NAME) built successfully$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(ENGINE_OBJS) $(TEST_OBJS)
	@echo "$(GREEN)Linking $(TEST_NAME)...$(RESET)"
	$(CXX) $(CXXFLAGS) $(ENGINE_OBJS) $(TEST_OBJS) -o $(TEST_NAME)

$(OBJ_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(TEST_DIR) -c $< -o $@

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(YELLOW)Removing $(NAME)...$(RESET)"
	rm -f $(NAME) $(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re test

-include $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)
