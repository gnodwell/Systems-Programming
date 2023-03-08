#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdarg.h> 
#include <assert.h> 
#include <errno.h> 

#include "os_defs.h"

#ifndef OS_WINDOWS
# include <dlfcn.h> 
# include <unistd.h> 
#else
# include <conio.h>
#endif

#include "module_tools.h"
#include "module_api.h"

/**
 *	This is where your work needs to happen.  Code has been provided
 *	to create a list for you to put the loaded modules in, but you
 *	need to add in all the code to actually load the modules and use
 *	them.
 */

/**
 * Tool to manage a list of modules.  This creates and initializes
 * a data structure to hold a list of Module structures, into which
 * you can put all of your information.
 */
ModuleList *createEmptyModuleList(int max)
{
	ModuleList *newList;

	newList = (ModuleList *) malloc(sizeof(ModuleList));
	newList->nModules = 0;
	newList->max = max;
	newList->modList = (Module *) malloc(newList->max * sizeof(Module));
	memset(newList->modList, 0, newList->max * sizeof(Module));

	return newList;
}


/**
 * Load all the modules.  The short names are provided in the
 * moduleNames list of strings, and there is a Module structure
 * set aside for each in moduleList, but you need to add the
 * code to actually make the loading happen.
 */
int
loadAllModules(
		ModuleList *moduleList, StringList *moduleNames,
		char *moduleDir, int verbosity)
{
	int i;
    char libname[256];
    void *libHandle;
    char *error;

    //char * (*fnName)();
    void * (*fnBox)(char *);
    void * (*fnParen)(char *, int);
    void * (*fnShout)(char *);

	/**
	 * Load all the modules
	 */
	for (i = 0; i < moduleNames->nStrings; i++) {

		/**
		 * Do the work to locate and load the module
		 * whose short name (e.g.; "box") is in
		 *    moduleNames->strList[i]
		 *
		 * Use the moduleList structure to hold your
		 * modules.
		 */

		/** ... add your code here ... */
		printf("Searching for module '%s'\n", moduleNames->strList[i]);
        sprintf(libname, "modules/%s.so", moduleNames->strList[i]);
        printf("Loading Library '%s' . . . \n", libname);
        
        libHandle = dlopen (libname, RTLD_LAZY);
        dlerror();



        if (libHandle == NULL) {
            fprintf(stderr, "Failed loading library : %s\n", dlerror());
        }

        if (moduleList->nModules + 1 > moduleList->max) {
            fprintf(stderr, "Maximum number of values (%d) exceeded\n", moduleList->max);
            return -1;
        }
        Module *temp;
        temp = &moduleList->modList[i];
        //temp = malloc(sizeof(Module));
        //temp->data = malloc(sizeof(char) * 1000);
        temp->name = strdup(moduleNames->strList[i]);
        temp->sharedObject = libHandle;
        moduleList->nModules++;

        //moduleList->modList[moduleList->nModules++] = *temp;

	}
    printf("nModules: %d\n", moduleList->nModules);

    for (i = 0; i < moduleList->nModules; i++) {
        Module *temp2 = &moduleList->modList[i];
        printf("testing names: %s\n", temp2->name);
    }

	return moduleList->nModules;
}


/**
 * Call the finalization function for each module, and then unload
 * it from memory, cleaning up all the resources as you go.
 */
void
unloadAllModules(ModuleList *modules)
{
	void (*termSymb)(void *);
	int i;

	for (i = 0; i < modules->nModules; i++) {

		/**
		 * Unload the library
		 */
		/** ... add your code here ... */
        Module *mod;
        mod = &modules->modList[i];
        printf("Freeing %s\n", mod->name);
        dlclose(mod->sharedObject);
        free(mod->name);
        //free(mod->data);



		/**
		 * Clean up any other memory that we have to
		 */
		/** ... add your code here ... */

	}
    free(modules->modList);

	/**
	 * Flag the modules structure back to unitilialized state
	 */
	modules->nModules = 0;
    free(modules);
}


/**
 * Do the main work of processing the given file stream.
 *
 * For all lines in the file, call for some processing to
 * happen.  After the processing is complete, print out
 * the final result.
 */
static int
processFPWithModuleList(
		FILE *ofp,
		FILE *ifp,
		const char *filename,
		ModuleList *moduleList,
		int verbosity
	)
{
	char line[BUFSIZ];
	int lineNo = 0;


	/**
	 * Do any setup that is required in your code before
	 * we get going...
	 */
	/* ... add your code here ... */
    Module *mod;
    void * (*fnBox)(char *, void *);
    void * (*fnParen)(char*, int, Module *);
    void * (*fnShout)(char *, Module *);
    char *error;


	/**
	 * Read lines, stopping when we get to EOF.  We can
	 * assume that BUFSIZ is plenty of length for each line
	 */
	while (fgets(line, BUFSIZ, ifp) != NULL) {

		/**
		 * Remove the trailing '\n' -- we will print it out
		 * separately below
		 */
		if (line[strlen(line)-1] == '\n') 
			line[strlen(line)-1] = '\0'; 

		/**
		 * apply each module in turn to the line.
		 */
		/** ... add your code here ... */
        for (int i = 0; i < moduleList->nModules; i++) {
            mod = &moduleList->modList[i];
            printf("testing here: %s\n", mod->name);
            if (strcmp(mod->name, "box") == 0) {
                fnBox = dlsym(mod->sharedObject, "boxFunction");
                if ((error = dlerror()) != NULL) {
                    fprintf(stderr, "Failed loading library: %s\n", dlerror());
                    return -1;
                }
                printf("Malloc box Here\n");
                mod->data = malloc(sizeof(char) * strlen(line) + 8);
                (*fnBox)(line, mod->data);
                printf("Test Data: %s\n", (char *)mod->data);
                strcpy(line, (char *)mod->data);
                free(mod->data);


            }
            else if (strcmp(mod->name, "paren") == 0) {
                fnParen = dlsym(mod->sharedObject, "parenFunction");
                if ((error = dlerror()) != NULL) {
                    fprintf(stderr, "Failed loading library: %s\n", dlerror());
                    return -1;
                }
                mod->data = malloc(sizeof(char) * strlen(line) + 8);
                printf("Malloc paren here\n");
                (*fnParen)(line, verbosity, mod->data);
                printf("Test Data: %s\n", (char *)mod->data);
                strcpy(line, (char *)mod->data);
                free(mod->data);
            }
            else if (strcmp(mod->name, "shout") == 0) {
                fnShout = dlsym(mod->sharedObject, "shoutFunction");
                if ((error = dlerror()) != NULL) {
                    fprintf(stderr, "Failed loading library: %s\n", dlerror());
                    return -1;
                }
                mod->data = malloc(sizeof(char) * strlen(line) + 2);
                printf("Malloc shout here\n");
                (*fnShout)(line, mod->data);
                printf("Test Data: %s\n", (char *)mod->data);
                strcpy(line, (char *)mod->data);
                free(mod->data);
            }


            printf("\n");

        }





		/**
		 * Now that we have applied all the modules to
		 * this line, wite out the result of all of the
		 * processing
		 */
		/** ... add your code here ... */

		/*
		 * Print out the line, now that we have applied all
		 * the edits from the modules. (Print out whatever
		 * buffer you are using after your processing, if
		 * not using the variable "line")
		 */
		fputs(line, ofp);

		/** add in the \n that we took off above */
		fputc('\n', ofp);
	}
	
	return 0;
}


/*
 * Open the indicated file, and then pass the open FILE handle
 * to the above function to actually do the processing.
 */
int
processFileWithModuleList(
		FILE *ofp,
		const char *filename,
		ModuleList *moduleList,
		int verbosity
	)
{
	FILE *ifp;
	int status;

    printf("Filename: %s\n", filename);

	ifp = fopen(filename, "r");
	if (ifp == NULL) {
		fprintf(stderr, "Cannot open input script '%s' : %s\n",
				filename, strerror(errno));
		return -1;
	}

	status = processFPWithModuleList(ofp, ifp,
			filename, moduleList, verbosity);

	fclose(ifp);
	return status;
}

