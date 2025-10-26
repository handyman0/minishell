# **************************************************************************** #
#                                   CONFIG                                     #
# **************************************************************************** #

NAME        = minishell

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -g3 -fsanitize=address
RM          = rm -rf

SRC_DIR     = src
OBJ_DIR     = obj
INC_DIR     = includes
LIBFT_DIR   = libft

LIBFT       = $(LIBFT_DIR)/libft.a

# **************************************************************************** #
#                                 CORES & MSGS                                 #
# **************************************************************************** #

RED     = \033[1;31m
GREEN   = \033[1;32m
BLUE    = \033[1;34m
YELLOW  = \033[1;33m
RESET   = \033[0m

# **************************************************************************** #
#                                DETECÇÃO DE ARQS                              #
# **************************************************************************** #

# Busca todos os .c de src/ e subpastas
SRCS    = $(shell find $(SRC_DIR) -type f -name "*.c")

# Cria hierarquia de obj/ igual à de src/
OBJS    = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

INCLUDES = -I$(INC_DIR) -I$(LIBFT_DIR)/include

# **************************************************************************** #
#                                    RULES                                     #
# **************************************************************************** #

all: libft $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	@$(CC) $(CFLAGS) $(OBJS) -L$(LIBFT_DIR) -lft -o $(NAME)
	@echo "$(GREEN)✅ $(NAME) compilado com sucesso!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "$(BLUE)🔹 Compilado:$(RESET) $<"

libft:
	@make -C $(LIBFT_DIR) -s

clean:
	@$(RM) $(OBJ_DIR)
	@make -C $(LIBFT_DIR) clean -s
	@echo "$(YELLOW)🧹 Objetos removidos$(RESET)"

fclean: clean
	@$(RM) $(NAME)
	@make -C $(LIBFT_DIR) fclean -s
	@echo "$(RED)💥 Binários removidos$(RESET)"

re: fclean all

debug: CFLAGS += -DDEBUG -g3 -fsanitize=address
debug: re
	@echo "$(YELLOW)🐛 Compilado em modo debug (sanitizer ON)$(RESET)"

valgrind: CFLAGS += -g3
valgrind: re
	@echo "$(YELLOW)🧠 Compilado para análise Valgrind$(RESET)"
	@echo "$(BLUE)👉 Exemplo: valgrind --leak-check=full --track-origins=yes ./$(NAME)$(RESET)"

.PHONY: all clean fclean re libft debug valgrind
