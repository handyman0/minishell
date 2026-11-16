#include "../../includes/output.h"
#include "../../mylib/include/mylib.h"
#include <string.h>
#include <errno.h>

void	put_errno(const char *a, const char *b)
{
	int	saved;

	saved = errno;
	ml_put_all(STDERR_FILENO, "minishell: ", a ? a : "", a ? ": " : "",
		b ? b : "", b ? ": " : "", strerror(saved), "\n", NULL);
	errno = saved;
}
