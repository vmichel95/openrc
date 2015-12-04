/*
 * rc-service.c
 * Finds all OpenRC services
 */

/*
 * Copyright (c) 2008-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/master/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/master/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "rc-misc.h"

extern char *applet;

#include "_usage.h"
#define usagestring ""							\
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"		\
	"   or: rc-service [options] -e <service>\n"			\
	"   or: rc-service [options] -l\n"				\
	"   or: rc-service [options] -r <service>"
#define getoptstring "e:ilr:" getoptstring_COMMON
static const struct option longopts[] = {
	{ "exists",   1, NULL, 'e' },
	{ "ifexists", 0, NULL, 'i' },
	{ "list",     0, NULL, 'l' },
	{ "resolve",  1, NULL, 'r' },
	longopts_COMMON
};
static const char * const longopts_help[] = {
	"tests if the service exists or not",
	"if the service exists then run the command",
	"list all available services",
	"resolve the service name to an init script",
	longopts_help_COMMON
};
#include "_usage.c"

int
rc_service(int argc, char **argv)
{
	int opt;
	char *service;
	RC_STRINGLIST *list;
	RC_STRING *s;
	bool if_exists = false;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	while ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (int *) 0)) != -1)
	{
		switch (opt) {
		case 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
#ifdef DEBUG_MEMORY
			free(service);
#endif
			return opt;
			/* NOTREACHED */
		case 'i':
			if_exists = true;
			break;
		case 'l':
			list = rc_services_in_runlevel(NULL);
			if (TAILQ_FIRST(list) == NULL)
				return EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->value);
#ifdef DEBUG_MEMORY
			rc_stringlist_free(list);
#endif
			return EXIT_SUCCESS;
			/* NOTREACHED */
		case 'r':
			service = rc_service_resolve(optarg);
			if (service == NULL)
				return EXIT_FAILURE;
			printf("%s\n", service);
#ifdef DEBUG_MEMORY
			free(service);
#endif
			return EXIT_SUCCESS;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	if (*argv == NULL)
		eerrorx("%s: you need to specify a service", applet);
	if ((service = rc_service_resolve(*argv)) == NULL) {
		if (if_exists)
			return 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: %s", applet, strerror(errno));
	/* NOTREACHED */
}
