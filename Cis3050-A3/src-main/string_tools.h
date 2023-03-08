#ifndef	__FORMATTER_STRINGS_HEADER__
#define	__FORMATTER_STRINGS_HEADER__

typedef struct StringList {
	int nStrings;
	int max;
	char **strList;
} StringList;

/**
 * Simple management of a list of strings.  The maximum size of
 * the list is fixed by maxStrings
 */
StringList *createEmptyStringList(int maxStrings);
int destroyStringList(StringList *list);
int addStringToList(StringList *list, char *newString);

/**
 * Use the above function to add each module name in the comma
 * separated list to the indicated list of module names.
 */
int addStringsToListWithDelimiter(
		StringList *stringList,
		char *delimitedString,
		char *delimiter
	);

/** help parse argv */
char *pullArgvOptionArgument(
		char **argv,
		int *indexp,
		char *optiontag
	);

#endif /*	__FORMATTER_STRINGS_HEADER__ */
