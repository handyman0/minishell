#include "includes/parser.h"
#include "includes/utils.h"
#include <stdio.h>

int main(void)
{
	char *test_input = "ls -la | grep test && echo 'hello world'";

	printf("🧪 Testando tokenizer...\n");
	printf("Input: '%s'\n\n", test_input);

	t_token *tokens = tokenize_line(test_input);

	if (tokens) {
		printf("Tokens criados com sucesso!\n");
		print_tokens(tokens);  // Vamos implementar essa função
		free_tokens(tokens);
	} else {
		printf("Nenhum token criado\n");
	}

	return (0);
}
