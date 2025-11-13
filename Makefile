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

INCLUDES    = -I$(INC_DIR) -I$(LIBFT_DIR)/include

# **************************************************************************** #
#                                   COLORS                                     #
# **************************************************************************** #

RED         = \033[1;31m
GREEN       = \033[1;32m
BLUE        = \033[1;34m
YELLOW      = \033[1;33m
RESET       = \033[0m

# **************************************************************************** #
#                                   FILES                                      #
# **************************************************************************** #

SRCS        = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS        = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# **************************************************************************** #
#                                   RULES                                      #
# **************************************************************************** #

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -L$(LIBFT_DIR) -lft -lreadline -o $(NAME)
	@echo "$(GREEN)[✔] $(NAME) compilado com sucesso!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIBFT):
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
	@echo "$(BLUE)🔁 Recompilação completa$(RESET)"

debug: CFLAGS += -DDEBUG -g3 -fsanitize=address
debug: re
	@echo "$(YELLOW)🐛 Compilado em modo debug$(RESET)"

valgrind: CFLAGS += -g3
valgrind: re
	@echo "$(BLUE)🧠 Build pronto para Valgrind$(RESET)"
	@echo "    👉 valgrind --leak-check=full ./$(NAME)"

.PHONY: all clean fclean re debug valgrind
