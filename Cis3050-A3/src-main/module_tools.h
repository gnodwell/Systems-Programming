#ifndef	__MODULE_TOOLS_HEADER__
#define	__MODULE_TOOLS_HEADER__

#include "os_defs.h"

#include <stdio.h>

#ifdef OS_WINDOWS
# include <windows.h>
#endif

#include "string_tools.h"

/**
 * This header file contains the types to manage a list of up to
 * 64 modules.  This number has been chosen as it is plenty for
 * you to demonstrate your design with, and saves you another level
 * of fiddly memory management.
 *
 * Code to populate the structures below has been roughed in.  It
 * follows the same pattern as the string handling for module names
 * in the string_tools.c file.
 */

#define MAX_N_MODULES	64


/*
 * define our types
 */
typedef struct Module {
	char *name;
	void *data;

	/** use an OS-specific type for the shared library */
#	ifdef OS_WINDOWS
	HINSTANCE sharedObject;
#	else
	void *sharedObject;
#	endif

} Module;

typedef struct ModuleList {
	int nModules;
	int max;
	Module *modList;
} ModuleList;

/** PROTOTYPES */

/** create a list for modules */
ModuleList *createEmptyModuleList(int max);
int destroyModuleList(ModuleList *list);

/** load and unload all modules */
int loadAllModules(
		ModuleList *moduleList, StringList *moduleNames,
		char *modpath, int verbosity);
void unloadAllModules(ModuleList *modules);

/** do the processing on a file */
int processFileWithModuleList(
		FILE *ofp,
		const char *filename,
		ModuleList *moduleList,
		int verbosity
	);

/** constant definitions */
#define	VAR_MODPATH	"MODPATH"

#ifdef OS_WINDOWS
# define	MOD_FILENAME_SUFFIX	".DLL"
#else
# define	MOD_FILENAME_SUFFIX	".so"
#endif

#endif /*	__MODULE_TOOLS_HEADER__ */
