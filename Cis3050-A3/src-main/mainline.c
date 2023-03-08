#include <stdio.h>

/** include os_defs before checks based on OS */
#include "os_defs.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef OS_WINDOWS
# include <unistd.h>
# include <unistd.h> 
#else
# include <conio.h>
#endif

#include "module_tools.h"
#include "string_tools.h"


/**
 * Print out the program help
 */
static int
printHelp(char *progname)
{
	printf("%s <options> [ <files> ]\n", progname);
	printf("\n");
	printf("Run scripts from files, or stdin if no file specified\n");
	printf("\n");
	printf("Options:\n");
	printf("-o <file>    : place output in <file>\n");
	printf("-v           : be more verbose\n");
	printf("-m <modules> : add <modules> to processing chain.\n");
	printf("             : The value in <modules> is a comma separated list made\n");
	printf("             : up of short module names: box, paren, shout\n");
	printf("\n");
	exit (1);
}


/**
 * Mainline.  Processes command line arguments and calls for processing
 * to happen.  You probably don't need to modify this function in any
 * way to acheive your objectives.
 */
int
main(int argc, char **argv)
{
	FILE *ofp = stdout;
	ModuleList *loadedModules;
	StringList *moduleNames;
	char *moduleDir = NULL;
	char *optionArg;
	int verbosity = 0;
	int processingStatus = 0;
	int fileOnlyArgC = 1;
	int exitStatus = (-1);
	int i, ch;

	/**
	 * Ensure that both the StringList and ModuleList
	 * know they are empty.
	 */
	loadedModules = createEmptyModuleList(MAX_N_MODULES);
	moduleNames = createEmptyStringList(MAX_N_MODULES);


	/**
	 * Get the environment variable whose name is stored in VAR_MODPATH
	 *
	 * i.e.; moduleDir = <get the environment variable value>
	 */
	moduleDir = "modules";

	/** there is no getopt on Windows, so just do the work ourselves */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-'
#					ifdef OS_WINDOWS
					|| argv[i][0] == '/'
#					endif
				) {
			/** is a flag */
			switch (argv[i][1]) {
			case 'v':
				verbosity++;
				break;

			case 'o':
				optionArg = pullArgvOptionArgument(argv, &i, "o");
				if (optionArg == NULL) {
					printHelp(argv[0]);
					exit(-1);
				}
					
				if ((ofp = fopen(optionArg, "w")) == NULL) {
					(void) fprintf(stderr,
							"failed opening output file '%s' : %s\n",
							optionArg, strerror(errno));
					printHelp(argv[0]);
				}
				break;

			case 'm':
				optionArg = pullArgvOptionArgument(argv, &i, "m");
				if (optionArg == NULL) {
					printHelp(argv[0]);
					exit(-1);
				}

				if (addStringsToListWithDelimiter(
							moduleNames, optionArg, ",") < 0) {
					fprintf(stderr, "Failed adding module name to list\n");
					exit(-1);
				}
				break;


			case '?':
			case 'h':
			default:
				printf("HELP!\n");
				printHelp(argv[0]);
				break;
			}
		} else {
			/**
			 * is a file -- move it up the list
			 * (overwriting non-file options)
			 */
			argv[fileOnlyArgC++] = argv[i];
		}
	}

	/**
	 ** Prepare to process the files -- set up the module system
	 **/

	/** if no modules are loaded yet, try the "default" module */
	if (moduleNames->nStrings == 0) {
		printf("Error: no modules specified!\n");
		printf("Provide at least one -m option\n\n");
		printHelp(argv[0]);
		exit (-1);
	}

	if (fileOnlyArgC <= 1) {
		printf("Error: no files specified!\n");
		printf("Provide at least one text file to process\n\n");
		printHelp(argv[0]);
		exit (-1);
	}
    //printf("Fourth\n");
    //for (int j = 0; j < argc; j++) {
    //    printf("argv[%d]: %s\n", j, argv[j]);
    //}


	/*
	 * Load all the modules
	 */
	if (loadAllModules(
			loadedModules, moduleNames,
			moduleDir, verbosity) < 0) {
		fprintf(stderr, "Modules not successfully loaded\n");
		exit (-1);
	}

	for (i = 1; i < fileOnlyArgC; i++) {
		if (processFileWithModuleList(ofp, argv[i], loadedModules, verbosity) < 0) {
			exitStatus = (-1);
			break;
		}
	}

	/**
	 * clean everything up
	 */
	unloadAllModules(loadedModules);
	
	destroyStringList(moduleNames);

    //fclose(ofp);

	return exitStatus;
}

