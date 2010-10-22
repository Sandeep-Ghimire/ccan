#define _GNU_SOURCE
#include <ccan/tap/tap.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ccan/opt/opt.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

unsigned int test_cb_called;
char *test_noarg(void *arg)
{
	test_cb_called++;
	return NULL;
}

char *test_arg(const char *optarg, const char *arg)
{
	test_cb_called++;
	ok1(strcmp(optarg, arg) == 0);
	return NULL;
}

void show_arg(char buf[OPT_SHOW_LEN], const char *arg)
{
	strncpy(buf, arg, OPT_SHOW_LEN);
}

char *err_output = NULL;

static void save_err_output(const char *fmt, ...)
{
	va_list ap;
	char *p;

	va_start(ap, fmt);
	vasprintf(&p, fmt, ap);
	va_end(ap);

	if (err_output) {
		err_output = realloc(err_output,
				     strlen(err_output) + strlen(p) + 1);
		strcat(err_output, p);
		free(p);
	} else
		err_output = p;
}	

/* FIXME: This leaks, BTW. */
bool parse_args(int *argc, char ***argv, ...)
{
	char **a;
	va_list ap;

	va_start(ap, argv);
	*argc = 1;
	a = malloc(sizeof(*a) * (*argc + 1));
	a[0] = (*argv)[0];
	while ((a[*argc] = va_arg(ap, char *)) != NULL) {
		(*argc)++;
		a = realloc(a, sizeof(*a) * (*argc + 1));
	}
	*argv = a;
	/* Re-set before parsing. */
	optind = 0;

	return opt_parse(argc, *argv, save_err_output);
}

struct opt_table short_table[] = {
	/* Short opts, different args. */
	OPT_WITHOUT_ARG("-a", test_noarg, "a", "Description of a"),
	OPT_WITH_ARG("-b", test_arg, show_arg, "b", "Description of b"),
	OPT_ENDTABLE
};

struct opt_table long_table[] = {
	/* Long opts, different args. */
	OPT_WITHOUT_ARG("--ddd", test_noarg, "ddd", "Description of ddd"),
	OPT_WITH_ARG("--eee <filename>", test_arg, show_arg, "eee", ""),
	OPT_ENDTABLE
};

struct opt_table long_and_short_table[] = {
	/* Short and long, different args. */
	OPT_WITHOUT_ARG("--ggg|-g", test_noarg, "ggg", "Description of ggg"),
	OPT_WITH_ARG("-h|--hhh", test_arg, NULL, "hhh", "Description of hhh"),
	OPT_ENDTABLE
};

/* Sub-table test. */
struct opt_table subtables[] = {
	/* Two short, and two long long, no description */
	OPT_WITH_ARG("--jjj|-j|--lll|-l", test_arg, show_arg, "jjj", ""),
	/* Hidden option */
	OPT_WITH_ARG("--mmm|-m", test_arg, show_arg, "mmm", opt_hidden),
	OPT_SUBTABLE(short_table, NULL),
	OPT_SUBTABLE(long_table, "long table options"),
	OPT_SUBTABLE(long_and_short_table, NULL),
	OPT_ENDTABLE
};
