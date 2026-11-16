# **************************************************************************** #
#                                   CONFIG                                     #
# **************************************************************************** #

NAME        = minishell

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -O2 -MMD -MP
LDFLAGS     ?= -static
RM          = rm -rf

SRC_DIR     = src
OBJ_DIR     = obj
INC_DIR     = includes
MYLIB_DIR   = mylib
MYLIB       = $(MYLIB_DIR)/libmylib.a

INCLUDES    = -I$(INC_DIR) -I$(MYLIB_DIR)/include

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

$(NAME): $(MYLIB) $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -L$(MYLIB_DIR) -lmylib $(LDFLAGS) -o $(NAME)
	@echo "$(GREEN)[✔] $(NAME) compilado com sucesso!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(MYLIB):
	@make -C $(MYLIB_DIR) -s

clean:
	@$(RM) $(OBJ_DIR)
	@make -C $(MYLIB_DIR) clean -s
	@echo "$(YELLOW)🧹 Objetos removidos$(RESET)"

fclean: clean
	@$(RM) $(NAME)
	@make -C $(MYLIB_DIR) fclean -s
	@echo "$(RED)💥 Binários removidos$(RESET)"

re: fclean all
	@echo "$(BLUE)🔁 Recompilação completa$(RESET)"

debug: CFLAGS += -DDEBUG -g3 -fsanitize=address -O0
debug: LDFLAGS =
debug: re
	@echo "$(YELLOW)🐛 Compilado em modo debug$(RESET)"

valgrind: CFLAGS += -g3
valgrind: LDFLAGS =
valgrind: re
	@echo "$(BLUE)🧠 Build pronto para Valgrind$(RESET)"
	@echo "    👉 valgrind --leak-check=full ./$(NAME)"

-include $(OBJS:.o=.d)

PREFIX      ?= $(HOME)/.local

install: all
	@install -Dm755 $(NAME) $(PREFIX)/bin/$(NAME)
	@echo "$(GREEN)[✔] instalado em $(PREFIX)/bin/$(NAME)$(RESET)"

.PHONY: all clean fclean re debug valgrind install

ASAN_FLAGS  = -Wall -Wextra -Werror -g3 -O1 -fno-omit-frame-pointer \
	-fsanitize=address,undefined -MMD -MP

asan:
	@$(MAKE) --no-print-directory NAME=minishell_asan OBJ_DIR=obj_asan \
		CFLAGS="$(ASAN_FLAGS)" LDFLAGS=

stress: asan
	@bash tests/stress.sh ./minishell_asan

dynamic:
	@$(MAKE) --no-print-directory NAME=minishell_dyn OBJ_DIR=obj_dyn LDFLAGS=

compare: all
	@bash tests/compare.sh ./minishell

leaks: dynamic
	@bash tests/leaks.sh ./minishell_dyn

.PHONY: asan stress leaks dynamic compare
