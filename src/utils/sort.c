#include "../../libft/include/libft.h"

void	sort_strings(char **items, int count)
{
	int		gap;
	int		i;
	int		j;
	char	*tmp;

	gap = count / 2;
	while (gap > 0)
	{
		i = gap;
		while (i < count)
		{
			tmp = items[i];
			j = i;
			while (j >= gap && ft_strcmp(items[j - gap], tmp) > 0)
			{
				items[j] = items[j - gap];
				j -= gap;
			}
			items[j] = tmp;
			i++;
		}
		gap /= 2;
	}
}
