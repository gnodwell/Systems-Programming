#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "os_defs.h"
#include "module_api.h"

/**
 * Replace with your own API.
 *
 * DLLs under Windows need to have extra "decoration" in their
 * signatures.  This decoration differs depending on if they
 * are being "exported" from a DLL, or "imported" from a DLL via
 * a header file.  In addition, if we are NOT on Windows, then
 * there needs to be no decoration at all.
 *
 * If you have a function in a DLL with this signature:
 *		int myFunc(char *)
 * then this must become
 *		__declspec(dllexport) int __cdecl myFunc(char *)
 * here -- but only for Windows.
 *
 * To help you with this, you will find that OS_EXPORT and
 * OS_C_DECL are defined conditionally based on "OS_WINDOWS"
 * in the "os_defs.h" header file -- but first you will have
 * to do the work to figure out when "OS_WINDOWS" shoud be
 * defined.
 *
 * See the os_defs.h header file for details, but then all of
 * the functions that you want to call on your loaded library
 * should follow this example below so that they are visible
 * after they are loaded.
 *
 * Remember that the API is up to you, so these functions can
 * be CALLED anything you want, and take any types of arguments
 * that you want.
 */
OS_EXPORT int OS_C_DECL 
shoutFunction(char * data, void *ret)
{
    printf("Hello from shout.c\n");

    int i = 0;
    char ch;
    while(data[i]){
        data[i] = toupper(data[i]);
        i++;
    }
    sprintf(ret, "%s", data);
    printf("test return: %s\n", (char *)ret);


	return -1;
}

