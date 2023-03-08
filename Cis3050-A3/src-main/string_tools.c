#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_tools.h"

/**
 * Tool to manage a list of strings
 */
StringList *createEmptyStringList(int max)
{
	StringList *newList;

	newList = (StringList *) malloc(sizeof(StringList));
	newList->nStrings = 0;
	newList->max = max;
	newList->strList = (char **) malloc(newList->max * sizeof(char *));
	memset(newList->strList, 0, newList->max * sizeof(char *));

	return newList;
}

/**
 * Simple management of a list of strings.  The maximum size of
 * the list is fixed, and must be set during initialization
 */
int addStringToList(
		StringList *list,
		char *newString
	)
{
	if ((list->nStrings + 1) > list->max) {
		fprintf(stderr, "Maximum number of values (%d) exceeded\n",
				list->max);
		return -1;
	}

	list->strList[list->nStrings++] = strdup(newString);
	return list->nStrings;
}


/**
 * Clean up the string list indicated above
 */
int destroyStringList(StringList *list)
{
	/*while (list->nStrings > 0) {
		free(list->strList[list->nStrings--]);
	}*/
    for (int i = 0; i < list->nStrings; i++) {
        free(list->strList[i]);
    }
    free(list->strList);
	free(list);
	return 0;
}

/**
 * Use the above function to add each module name in the comma
 * separated list to the indicated list of module names.
 *
 * Note that strtok() is not thread safe, and not re-entrant,
 * so the only really safe way to use this function is in programs
 * with no threads (or only one using strtok()), and which do not
 * call any other functions that might use strtok() from any calls
 * made in this function.
 */
int addStringsToListWithDelimiter(
		StringList *stringList,
		char *delimitedString,
		char *delimiter
	)
{
	int nAdded = 0;
	char *token;

	/** set up the tokenizer for parsing the list */
	token = strtok(delimitedString, delimiter);
	if (token == NULL) {
		fprintf(stderr, "Cannot add empty string to list\n");
		return -1;
	}
	if (addStringToList(stringList, token) < 0) {
		return -1;
	}
	nAdded++;

	/** if there were any commas, keep going until we run out of tokens */
	while ((token = strtok(NULL, delimiter)) != NULL) {
		if (addStringToList(stringList, token) < 0) {
			return -1;
		}
		nAdded++;
	}

	return nAdded;
}


/**
 ** This is an ugly, clunky, but easy to write replacement for getopt,
 ** which is not available on Windows
 **/
char *pullArgvOptionArgument(
		char **argv,
		int *indexp,
		char *optiontag
	)
{
	char *optionArg;

	if (argv[*indexp][2] != '\0')
		optionArg = &argv[*indexp][2];
	else
		optionArg = argv[++(*indexp)];
	if (optionArg == NULL || strlen(optionArg) == 0) {
		fprintf(stderr, "No option given for '-%s'\n", optiontag);
		return NULL;
	}

    printf("pullArgvOptionArgument: %s\n", optionArg);
	return optionArg;
}

