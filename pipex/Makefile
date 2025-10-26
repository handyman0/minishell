NAME		= pipex
NAME_BONUS	= pipex_bonus

CC			= cc
CFLAGS		= -Wall -Wextra -Werror
RM			= rm -f

SRC_DIR		= src
BONUS_DIR	= bonus
OBJ_DIR		= obj
OBJ_BONUS_DIR	= obj_bonus
INC_DIR		= include
LIBFT_DIR	= libft

SRCS		= $(wildcard $(SRC_DIR)/*.c)
OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

BONUS_SRCS	= $(wildcard $(BONUS_DIR)/*.c)
COMMON_SRCS	= $(SRC_DIR)/utils.c
BONUS_OBJS	= $(BONUS_SRCS:$(BONUS_DIR)/%.c=$(OBJ_BONUS_DIR)/%.o) \
			  $(COMMON_SRCS:$(SRC_DIR)/%.c=$(OBJ_BONUS_DIR)/%.o)

INCLUDES	= -I $(INC_DIR) -I $(LIBFT_DIR)/include
LIBFT		= $(LIBFT_DIR)/libft.a

RED		= \033[1;31m
GREEN	= \033[1;32m
BLUE	= \033[1;34m
YELLOW	= \033[1;33m
RESET	= \033[0m

all: libft $(NAME)

bonus: libft $(NAME_BONUS)

libft:
	@make -C $(LIBFT_DIR) -s

$(NAME): $(OBJS) $(LIBFT)
	@$(CC) $(CFLAGS) $(OBJS) -L $(LIBFT_DIR) -lft -o $(NAME)
	@echo "$(GREEN)✅ Pipex compilado!$(RESET)"

$(NAME_BONUS): $(BONUS_OBJS) $(LIBFT)
	@$(CC) $(CFLAGS) $(BONUS_OBJS) -L $(LIBFT_DIR) -lft -o $(NAME_BONUS)
	@echo "$(GREEN)✅ Pipex bonus compilado!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_BONUS_DIR)/%.o: $(BONUS_DIR)/%.c | $(OBJ_BONUS_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_BONUS_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_BONUS_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_BONUS_DIR):
	@mkdir -p $(OBJ_BONUS_DIR)

clean:
	@$(RM) -r $(OBJ_DIR) $(OBJ_BONUS_DIR)
	@make -C $(LIBFT_DIR) clean -s
	@echo "$(YELLOW)🧹 Objetos removidos$(RESET)"

fclean: clean
	@$(RM) $(NAME) $(NAME_BONUS)
	@make -C $(LIBFT_DIR) fclean -s
	@echo "$(RED)🧨 Binários removidos$(RESET)"

re: fclean all

.PHONY: all clean fclean re bonus libft
