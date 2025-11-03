#include "includes/parser.h"
#include "includes/utils.h"
#include <stdio.h>

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
	char *test_input = "ls -la | grep test";

	printf("🧪 Testando tokenizer...\n");
	printf("Input: '%s'\n\n", test_input);

	t_token *tokens = tokenize_line(test_input);
	t_token *tokens_start = tokens;  // ← GUARDA o início da lista

	if (tokens) {
		printf("✅ Tokens criados com sucesso!\n");
		print_tokens(tokens);

		// Testar parser
		printf("🧪 Testando parser...\n");
		t_node *ast = parse_tokens(&tokens);  // ← tokens é modificado

		if (ast) {
			printf("✅ AST criada com sucesso!\n");
			print_ast(ast, 0);
			free_ast(ast);
		} else {
			printf("❌ Falha ao criar AST\n");
		}

		free_tokens(tokens_start);  // ← LIBERA todos os tokens (início original)
	} else {
		printf("❌ Nenhum token criado\n");
	}

	return (0);
}
