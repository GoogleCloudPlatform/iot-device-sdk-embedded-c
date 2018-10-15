/* tinytest.c -- Copyright 2009-2012 Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifdef TINYTEST_LOCAL
#include "tinytest_local.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "iotc_bsp_time.h"

/* This conditional compilation is endented for clarity */
#ifndef NO_FORKING
  #ifdef _WIN32
    #include <windows.h>
  #else
    #ifndef IOTC_EMBEDDED_TESTS
      #include <sys/types.h>
      #include <sys/wait.h>
      #include <unistd.h>
    #endif /* IOTC_EMBEDDED_TESTS */
  #endif /* _WIN32 */
  #if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
    #if (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1060 && \
        __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)
      /* Workaround for a bug in OSX 10.6 */
      #define FORK_BREAKS_GCOV
      #include <vproc.h>
    #endif /* defined(__APPLE__) ... */
  #endif /* (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED ... */
#endif /*  not NO_FORKING */

#ifndef __GNUC__
#define __attribute__(x)
#endif

#include "tinytest.h"
#include "tinytest_macros.h"

#ifdef IOTC_EMBEDDED_TESTS
#define LONGEST_TEST_NAME 160
#else
#define LONGEST_TEST_NAME 16384
#endif

static int in_tinytest_main = 0; /**< true if we're in tinytest_main().*/
static int n_ok = 0; /**< Number of tests that have passed */
static int n_bad = 0; /**< Number of tests that have failed. */

typedef struct iotc_tt_test_cases_s
{
	char* test_case_description;
	struct iotc_tt_test_cases_s* __next;
} iotc_tt_test_cases_t;

static iotc_tt_test_cases_t* iotc_tt_test_cases_failing;

static int n_skipped = 0; /**< Number of tests that have been skipped. */

static int opt_forked = 0; /**< True iff we're called from inside a win32 fork*/
#ifndef NO_FORKING
static int opt_nofork = 0; /**< Suppress calls to fork() for debugging. */
#endif

static int stop_on_failure = 0;

static int opt_verbosity = 1; /**< -==quiet,0==terse,1==normal,2==verbose */
const char *verbosity_flag = "";

const struct testlist_alias_t *cfg_aliases=NULL;

enum outcome { SKIP=2, OK=1, FAIL=0 };
static enum outcome cur_test_outcome = FAIL;
const char *cur_test_prefix = NULL; /**< prefix of the current test group */
/** Name of the current test, if we haven't logged is yet. Used for --quiet */
const char *cur_test_name = NULL;

#ifdef _WIN32
/* Copy of argv[0] for win32. */
static char commandname[MAX_PATH+1];
#endif

static void usage(struct testgroup_t *groups, int list_groups)
  __attribute__((noreturn));
static int process_test_option(struct testgroup_t *groups, const char *test);

static enum outcome
testcase_run_bare_(const struct testcase_t *testcase)
{
	void *env = NULL;
	enum outcome run_outcome;
	if (testcase->setup) {
		env = testcase->setup->setup_fn(testcase);
		if (!env)
			return FAIL;
		else if (env == (void*)TT_SKIP)
			return SKIP;
	}

	cur_test_outcome = OK;
	testcase->fn(); /* was testcase->fn(env) but we want to avoid undefined behaviour here */
	run_outcome = cur_test_outcome;

	if (testcase->setup) {
		if (testcase->setup->cleanup_fn(testcase, env) == 0)
			run_outcome = FAIL;
	}

	return run_outcome;
}

#define MAGIC_EXITCODE 42

#ifndef NO_FORKING

static enum outcome
testcase_run_forked_(const struct testgroup_t *group,
		     const struct testcase_t *testcase)
{
#ifdef _WIN32
	/* Fork? On Win32?  How primitive!  We'll do what the smart kids do:
	   we'll invoke our own exe (whose name we recall from the command
	   line) with a command line that tells it to run just the test we
	   want, and this time without forking.

	   (No, threads aren't an option.  The whole point of forking is to
	   share no state between tests.)
	 */
	int ok;
	char buffer[LONGEST_TEST_NAME+256];
	STARTUPINFOA si;
	PROCESS_INFORMATION info;
	DWORD exitcode;

	if (!in_tinytest_main) {
		printf("\nERROR.  On Windows, testcase_run_forked_ must be"
		       " called from within tinytest_main.\n");
		abort();
	}
	if (opt_verbosity>0)
		printf("[forking] ");

	snprintf(buffer, sizeof(buffer), "%s --RUNNING-FORKED %s %s%s",
		 commandname, verbosity_flag, group->prefix, testcase->name);

	memset(&si, 0, sizeof(si));
	memset(&info, 0, sizeof(info));
	si.cb = sizeof(si);

	ok = CreateProcessA(commandname, buffer, NULL, NULL, 0,
			   0, NULL, NULL, &si, &info);
	if (!ok) {
		printf("CreateProcess failed!\n");
		return 0;
	}
	WaitForSingleObject(info.hProcess, INFINITE);
	GetExitCodeProcess(info.hProcess, &exitcode);
	CloseHandle(info.hProcess);
	CloseHandle(info.hThread);
	if (exitcode == 0)
		return OK;
	else if (exitcode == MAGIC_EXITCODE)
		return SKIP;
	else
		return FAIL;
#else /* not _WIN32 */
	int outcome_pipe[2];
	pid_t pid;
	(void)group;

	if (pipe(outcome_pipe))
		perror("opening pipe");

	if (opt_verbosity>0)
		printf("[forking] ");
	pid = fork();
#ifdef FORK_BREAKS_GCOV
	vproc_transaction_begin(0);
#endif /* FORK_BREAKS_GCOV */
	if (!pid) {
		/* child. */
		int test_r, write_r;
		char b[1];
		close(outcome_pipe[0]);
		test_r = testcase_run_bare_(testcase);
		assert(0<=(int)test_r && (int)test_r<=2);
		b[0] = "NYS"[test_r];
		write_r = (int)write(outcome_pipe[1], b, 1);
		if (write_r != 1) {
			perror("write outcome to pipe");
			exit(1);
		}
		exit(0);
		return FAIL; /* unreachable */
	} else {
		/* parent */
		int status, r;
		char b[1];
		/* Close this now, so that if the other side closes it,
		 * our read fails. */
		close(outcome_pipe[1]);
		r = (int)read(outcome_pipe[0], b, 1);
		if (r == 0) {
			printf("[Lost connection!] ");
			return 0;
		} else if (r != 1) {
			perror("read outcome from pipe");
		}
		waitpid(pid, &status, 0);
		close(outcome_pipe[0]);
		return b[0]=='Y' ? OK : (b[0]=='S' ? SKIP : FAIL);
	}
#endif /* _WIN32 */
}

#endif /*  not NO_FORKING */

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define IOTC_TT_PRINT_COLORED( ansi_color, text ) \
	printf( ansi_color "%s" ANSI_COLOR_RESET, text );

int
testcase_run_one(const struct testgroup_t *group,
		 const struct testcase_t *testcase)
{
	enum outcome run_outcome;

	if (testcase->flags & (TT_SKIP|TT_OFF_BY_DEFAULT)) {
		if (opt_verbosity>0)
		{
			IOTC_TT_PRINT_COLORED(ANSI_COLOR_YELLOW, (testcase->flags & TT_SKIP) ? "[  SKIPPED ]" : "[ DISABLED ]");
			printf(" %s%s\n", group->prefix, testcase->name);
		}
		++n_skipped;
		return SKIP;
	}

	if (opt_verbosity>0 && !opt_forked) {
		IOTC_TT_PRINT_COLORED(ANSI_COLOR_GREEN, "[.RUN      ] ");
		printf("%s%s\n", group->prefix, testcase->name);
	} else {
		if (opt_verbosity==0) printf(".");
		cur_test_prefix = group->prefix;
		cur_test_name = testcase->name;
	}

	long t0_testcase = iotc_bsp_time_getcurrenttime_milliseconds();

#ifndef NO_FORKING
	if ((testcase->flags & TT_FORK) && !(opt_forked||opt_nofork)) {
		run_outcome = testcase_run_forked_(group, testcase);
	} else {
#else /* NO_FORKING */
	{
#endif /* not NO_FORKING */
		run_outcome = testcase_run_bare_(testcase);
	}

	long t1_testcase = iotc_bsp_time_getcurrenttime_milliseconds();
	const unsigned long deltatime_testcase = t1_testcase - t0_testcase;

	if (run_outcome == OK) {
		++n_ok;
		if (opt_verbosity>0 && !opt_forked)
		{
			IOTC_TT_PRINT_COLORED(ANSI_COLOR_GREEN, opt_verbosity==1?"[       OK ]":" ");
			printf(" %s%s (%lu ms)\n", group->prefix, testcase->name, deltatime_testcase);

		}
	} else if (run_outcome == SKIP) {
		++n_skipped;
		if (opt_verbosity>0 && !opt_forked)
		{
			IOTC_TT_PRINT_COLORED(ANSI_COLOR_YELLOW, "[  SKIPPED ]");
			printf(" %s%s (%lu ms)\n", group->prefix, testcase->name, deltatime_testcase);
		}
	} else {
		++n_bad;
		if (!opt_forked)
		{
			IOTC_TT_PRINT_COLORED(ANSI_COLOR_RED, "[. FAILED  ]");
			printf(" %s%s (%lu ms)\n", group->prefix, testcase->name, deltatime_testcase);

			if ( 1 ) /* collect failing tests for printing at the end */
			{
				iotc_tt_test_cases_t* new_failing =
					( iotc_tt_test_cases_t* ) malloc( sizeof( iotc_tt_test_cases_t ) );
				memset( new_failing, 0, sizeof( iotc_tt_test_cases_t ) );

				char buffer[ 300 ] = { 0 };
				sprintf( buffer, " %s%s (%lu ms)\n", group->prefix, testcase->name, deltatime_testcase );

				new_failing->test_case_description = ( char* ) malloc( strlen( buffer ) + 1 );
				memcpy( new_failing->test_case_description, (void*)buffer, strlen( buffer ) + 1 );

				{	/* put the new failing description to the end of the list */
					iotc_tt_test_cases_t* last_in_chain = iotc_tt_test_cases_failing;

					while ( last_in_chain && last_in_chain->__next )
					{
						last_in_chain = last_in_chain->__next;
					}

					if ( last_in_chain )
					{
						last_in_chain->__next = new_failing;
					}
					else
					{
						iotc_tt_test_cases_failing = new_failing;
					}
				}
			}
		}
	}

#ifndef NO_FORKING
	if (opt_forked) {
		exit(outcome==OK ? 0 : (outcome==SKIP?MAGIC_EXITCODE : 1));
		return 1; /* unreachable */
	} else {
		return (int) run_outcome;
	}
#else /* NO_FORKING */
	return (int) run_outcome;
#endif /* not NO_FORKING */
}

int
tinytest_set_flag_(struct testgroup_t *groups, const char *arg, int set, unsigned long flag)
{
	int i, j;
	size_t length = LONGEST_TEST_NAME;
	char fullname[LONGEST_TEST_NAME];
	int found=0;
	if (strstr(arg, ".."))
		length = strstr(arg,"..")-arg;
	for (i=0; groups[i].prefix; ++i) {
		for (j=0; groups[i].cases[j].name; ++j) {
			struct testcase_t *testcase = &groups[i].cases[j];
			snprintf(fullname, sizeof(fullname), "%s%s",
				 groups[i].prefix, testcase->name);
			if (!flag) { /* Hack! */
				printf("    %s", fullname);
				if (testcase->flags & TT_OFF_BY_DEFAULT)
					puts("   (Off by default)");
				else if (testcase->flags & TT_SKIP)
					puts("  (DISABLED)");
				else
					puts("");
			}
			if (!strncmp(fullname, arg, length)) {
				if (set)
					testcase->flags |= flag;
				else
					testcase->flags &= ~flag;
				++found;
			}
		}
	}
	return found;
}

static void
usage(struct testgroup_t *groups, int list_groups)
{
	puts("Options are: [--verbose|--quiet|--terse] [--no-fork]");
	puts("  Specify tests by name, or using a prefix ending with '..'");
	puts("  To skip a test, prefix its name with a colon.");
	puts("  To enable a disabled test, prefix its name with a plus.");
	puts("  Use --list-tests for a list of tests.");
	if (list_groups) {
		puts("Known tests are:");
		tinytest_set_flag_(groups, "..", 1, 0);
	}
	exit(0);
}

static int
process_test_alias(struct testgroup_t *groups, const char *test)
{
	int i, j, n, r;
	for (i=0; cfg_aliases && cfg_aliases[i].name; ++i) {
		if (!strcmp(cfg_aliases[i].name, test)) {
			n = 0;
			for (j = 0; cfg_aliases[i].tests[j]; ++j) {
				r = process_test_option(groups, cfg_aliases[i].tests[j]);
				if (r<0)
					return -1;
				n += r;
			}
			return n;
		}
	}
	printf("No such test alias as @%s!",test);
	return -1;
}

static int
process_test_option(struct testgroup_t *groups, const char *test)
{
	int flag = TT_ENABLED_;
	int n = 0;
	if (test[0] == '@') {
		return process_test_alias(groups, test + 1);
	} else if (test[0] == ':') {
		++test;
		flag = TT_SKIP;
	} else if (test[0] == '+') {
		++test;
		++n;
		if (!tinytest_set_flag_(groups, test, 0, TT_OFF_BY_DEFAULT)) {
			printf("No such test as %s!\n", test);
			return -1;
		}
	} else {
		++n;
	}
	if (!tinytest_set_flag_(groups, test, 1, flag)) {
		printf("No such test as %s!\n", test);
		return -1;
	}
	return n;
}

void
tinytest_set_aliases(const struct testlist_alias_t *aliases)
{
	cfg_aliases = aliases;
}

int
tinytest_main(int c, const char **v, struct testgroup_t *groups)
{
	int i, j, n=0;

#ifdef _WIN32
	const char *sp = strrchr(v[0], '.');
	const char *extension = "";
	if (!sp || stricmp(sp, ".exe"))
		extension = ".exe"; /* Add an exe so CreateProcess will work */
	snprintf(commandname, sizeof(commandname), "%s%s", v[0], extension);
	commandname[MAX_PATH]='\0';
#endif /* _WIN32 */
	for (i=1; i<c; ++i) {
		if (v[i][0] == '-') {
			if (!strcmp(v[i], "--RUNNING-FORKED")) {
				opt_forked = 1;
#ifndef NO_FORKING
			} else if (!strcmp(v[i], "--no-fork")) {
				opt_nofork = 1;
#endif
			} else if (!strcmp(v[i], "--quiet")) {
				opt_verbosity = -1;
				verbosity_flag = "--quiet";
			} else if (!strcmp(v[i], "--verbose")) {
				opt_verbosity = 2;
				verbosity_flag = "--verbose";
			} else if (!strcmp(v[i], "--terse")) {
				opt_verbosity = 0;
				verbosity_flag = "--terse";
			} else if (!strcmp(v[i], "--help")) {
				usage(groups, 0);
			} else if (!strcmp(v[i], "--list-tests")) {
				usage(groups, 1);
			} else if (!strcmp(v[i], "--stoponfailure")) {
				stop_on_failure = 1;
			} else {
				//printf("tinytest: Unknown option %s.  Try --help\n",v[i]);
				//return -1;
			}
		} else {
			int r = process_test_option(groups, v[i]);
			if (r<0)
				return -1;
			n += r;
		}
	}
	if (!n)
		tinytest_set_flag_(groups, "..", 1, TT_ENABLED_);

#ifndef IOTC_EMBEDDED_TESTS
#ifdef _IONBF
	setvbuf(stdout, NULL, _IONBF, 0);
#endif /* _IONBF */
#endif /* IOTC_EMBEDDED_TESTS */

	long t0_all = iotc_bsp_time_getcurrenttime_milliseconds();

	char run_tests = 1;
	++in_tinytest_main;
	for (i=0; groups[i].prefix && run_tests; ++i)
	{
		for (j=0; groups[i].cases[j].name && run_tests; ++j)
		{
			if (groups[i].cases[j].flags & TT_ENABLED_)
			{
				if (FAIL == testcase_run_one(&groups[i],
						 &groups[i].cases[j]) && 1 == stop_on_failure)
				{
					run_tests = 0;
				}
			}
		}
	}

	long t1_all = iotc_bsp_time_getcurrenttime_milliseconds();
	unsigned long deltatime_all = t1_all - t0_all;

	--in_tinytest_main;

	IOTC_TT_PRINT_COLORED(ANSI_COLOR_GREEN, "\n[==========]");
	printf(" %d tests ran. (%lu ms total)\n", n_ok, deltatime_all);
	if (opt_verbosity >= 1)
	{
		IOTC_TT_PRINT_COLORED(ANSI_COLOR_GREEN, "[. PASSED  ]")
		printf(" %d tests. (%d skipped)\n", n_ok, n_skipped);
	}

	if (n_bad)
	{
		IOTC_TT_PRINT_COLORED(ANSI_COLOR_RED, "[. FAILED  ]")
		printf(" %d tests, listed below: (%d skipped)\n", n_bad, n_skipped);

		if ( iotc_tt_test_cases_failing )
		{
			while ( iotc_tt_test_cases_failing )
			{
				IOTC_TT_PRINT_COLORED(ANSI_COLOR_RED, "[. FAILED  ]")
				printf( "%s", iotc_tt_test_cases_failing->test_case_description );

				iotc_tt_test_cases_t* not_needed_anymore = iotc_tt_test_cases_failing;
				iotc_tt_test_cases_failing = iotc_tt_test_cases_failing->__next;

				free( not_needed_anymore->test_case_description );
				free( not_needed_anymore );
			}
		}
	}

	if (opt_verbosity >= 1)
	{
		printf("\n");
	}

	return (n_bad == 0) ? 0 : 1;
}

int
tinytest_get_verbosity_(void)
{
	return opt_verbosity;
}

void
tinytest_set_test_failed_(void)
{
	if (opt_verbosity <= 0 && cur_test_name) {
		if (opt_verbosity==0) puts("");
		printf("%s%s: ", cur_test_prefix, cur_test_name);
		cur_test_name = NULL;
	}
	cur_test_outcome = (enum outcome) FAIL;
}

void
tinytest_set_test_skipped_(void)
{
	if (cur_test_outcome==OK)
		cur_test_outcome = SKIP;
}

