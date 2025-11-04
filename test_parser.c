#include "includes/minishell.h"
#include <stdio.h>
#include <unistd.h>

static void	print_ast(t_node *node, int depth)
{
	int	i;

	if (!node)
		return ;
	for (i = 0; i < depth; i++)
		printf("	");

	switch (node->type)
	{
	case NODE_CMD:
		printf("CMD: ");
		if (node->data.cmd.argv)
		{
			for (i = 0; node->data.cmd.argv[i]; i++)
				printf("%s ", node->data.cmd.argv[i]);
		}
		printf("\n");
		break;
	case NODE_PIPE:
		printf("PIPE:\n");
		print_ast(node->data.op.left, depth + 1);
		print_ast(node->data.op.right, depth + 1);
		break;
	default:
		printf("UNKNOWN NODE TYPE: %d\n", node->type);
	}
}

int main(void)
{
	char *test_input = "ls -la | grep minishell";
	t_shell shell;

	// DEBUG: Verificar __environ
	printf("🔍 DEBUG: __environ = %p\n", __environ);

	// Inicializa o shell
	shell.env = env_init(__environ);

	// DEBUG: Verificar se o ambiente foi inicializado
	printf("🔍 DEBUG: shell.env = %p\n", shell.env);
	if (shell.env)
	{
		printf("🔍 DEBUG: Ambiente tem %d variáveis\n", env_size(shell.env));
		// DEBUG: Verificar se PATH existe
		char *path = env_get(shell.env, "PATH");
		printf("🔍 DEBUG: PATH = %p\n", path);
		if (path)
			printf("🔍 DEBUG: PATH value = %s\n", path);
	}

	shell.exit_status = 0;
	shell.running = 1;

	printf("🧪 Testando tokenizer e parser...\n");
	printf("Input: '%s'\n\n", test_input);

	t_token *tokens = tokenize_line(test_input);
	t_token *tokens_start = tokens;

	if (tokens) {
		printf("✅ Tokens criados com sucesso!\n");
		print_tokens(tokens);

		t_node *ast = parse_tokens(&tokens);

		if (ast) {
			printf("✅ AST criada com sucesso!\n");
			print_ast(ast, 0);

			printf("\n🧪 Testando executor...\n");
			shell.exit_status = execute_tree(ast, &shell);
			printf("✅ Comando executado com status: %d\n", shell.exit_status);

			free_ast(ast);
		} else {
			printf("❌ Falha ao criar AST\n");
		}

		free_tokens(tokens_start);
	} else {
		printf("❌ Nenhum token criado\n");
	}

	env_free(shell.env);
	return (0);
}
