#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "gnuboy.h"
#include "defs.h"
#include "rc.h"


static rcvar_t *rcvars;

static int nvars;


void rc_export(rcvar_t *v)
{
	const rcvar_t end = RCV_END;

	if (!v) return;
	nvars++;
	rcvars = realloc(rcvars, sizeof (rcvar_t) * (nvars+1));
	if (!rcvars)
		die("out of memory adding rcvar %s\n", v->name);
	rcvars[nvars-1] = *v;
	rcvars[nvars] = end;
}

void rc_exportvars(rcvar_t *vars)
{
	while(vars->type)
		rc_export(vars++);
}



int rc_findvar(char *name)
{
	int i;
	if (!rcvars) return -1;
	for (i = 0; rcvars[i].name; i++)
		if (!strcmp(rcvars[i].name, name))
			break;
	if (!rcvars[i].name)
		return -1;
	return i;
}


int my_atoi(const char *s)
{
	int a = 0;
	if (*s == '0')
	{
		s++;
		if (*s == 'x' || *s == 'X')
		{
			s++;
			while (*s)
			{
				if (isdigit((unsigned char)*s))
					a = (a<<4) + *s - '0';
				else if (strchr("ABCDEF", *s))
					a = (a<<4) + *s - 'A' + 10;
				else if (strchr("abcdef", *s))
					a = (a<<4) + *s - 'a' + 10;
				else return a;
				s++;
			}
			return a;
		}
		while (*s)
		{
			if (strchr("01234567", *s))
				a = (a<<3) + *s - '0';
			else return a;
			s++;
		}
		return a;
	}
	if (*s == '-')
	{
		s++;
		for (;;)
		{
			if (isdigit((unsigned char)*s))
				a = (a*10) + *s - '0';
			else return -a;
			s++;
		}
	}
	while (*s)
	{
		if (isdigit((unsigned char)*s))
			a = (a*10) + *s - '0';
		else return a;
		s++;
	}
	return a;
}


int rc_setvar_n(int i, int c, char **v)
{
	int j;
	int *n;
	char **s;

	switch (rcvars[i].type)
	{
	case rcv_int:
		if (c < 1) return -1;
		n = (int *)rcvars[i].mem;
		*n = my_atoi(v[0]);
		return 0;
	case rcv_string:
		if (c < 1) return -1;
		s = (char **)rcvars[i].mem;
		if (*s) free(*s);
		*s = strdup(v[0]);
		if (!*s)
			die("out of memory setting rcvar %s\n", rcvars[i].name);
		return 0;
	case rcv_vector:
		if (c > rcvars[i].len)
			c = rcvars[i].len;
		for (j = 0; j < c ; j++)
			((int *)rcvars[i].mem)[j] = my_atoi(v[j]);
		return 0;
	case rcv_bool:
		if (c < 1 || atoi(v[0]) || strchr("yYtT", v[0][0]))
			*(int *)rcvars[i].mem = 1;
		else if (strchr("0nNfF", v[0][0]))
			*(int *)rcvars[i].mem = 0;
		else
			return -1;
		return 0;
	}
	return -1;
}


int rc_setvar(char *name, int c, char **v)
{
	int i;

	i = rc_findvar(name);
	if (i < 0) return i;

	return rc_setvar_n(i, c, v);
}


void *rc_getmem_n(int i)
{
	return rcvars[i].mem;
}


void *rc_getmem(char *name)
{
	int i;
	i = rc_findvar(name);
	if (i < 0) return NULL;
	return rcvars[i].mem;
}

int rc_getint_n(int i)
{
	if (i < 0) return 0;
	switch (rcvars[i].type)
	{
	case rcv_int:
	case rcv_bool:
		return *(int *)rcvars[i].mem;
	}
	return 0;
}

int *rc_getvec_n(int i)
{
	if (i < 0) return NULL;
	switch (rcvars[i].type)
	{
	case rcv_int:
	case rcv_bool:
	case rcv_vector:
		return (int *)rcvars[i].mem;
	}
	return NULL;
}

char *rc_getstr_n(int i)
{
	if (i < 0) return 0;
	switch (rcvars[i].type)
	{
	case rcv_string:
		return *(char **)rcvars[i].mem;
	}
	return 0;
}

int rc_getint(char *name)
{
	return rc_getint_n(rc_findvar(name));
}

int *rc_getvec(char *name)
{
	return rc_getvec_n(rc_findvar(name));
}

char *rc_getstr(char *name)
{
	return rc_getstr_n(rc_findvar(name));
}
