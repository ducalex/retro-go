

/*
 * splitline is a destructive argument parser, much like a very primitive
 * form of a shell parser. it supports quotes for embedded spaces and
 * literal quotes with the backslash escape.
 */

char *splitnext(char **pos)
{
	char *a, *d, *s;

	d = s = *pos;
	while (*s == ' ' || *s == '\t') s++;
	a = s;
	while (*s && *s != ' ' && *s != '\t')
	{
		if (*s == '"')
		{
			s++;
			while (*s && *s != '"')
			{
				if (*s == '\\')
					s++;
				if (*s)
					*(d++) = *(s++);
			}
			if (*s == '"') s++;
		}
		else
		{
			if (*s == '\\')
				s++;
			*(d++) = *(s++);
		}
	}
	while (*s == ' ' || *s == '\t') s++;
	*d = 0;
	*pos = s;
	return a;
}

int splitline(char **argv, int max, char *line)
{
	char *s;
	int i;

	s = line;
	for (i = 0; *s && i < max + 1; i++)
		argv[i] = splitnext(&s);
	argv[i] = 0;
	return i;
}





