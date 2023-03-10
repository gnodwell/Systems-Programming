/**
 * Companion to "flock.c" but based on record locking, as opposed to
 * whole file locking
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>

#include "os_defs.h"

#ifndef OS_WINDOWS
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#else
#include <windows.h>
#endif

/**
 * you will need to calculate your "os_defs.h" here so that you can use it
 * to determine the proper header files.
 * For now, we just define OS_WINDOWS
 */

//#define OS_WINDOWS

/**
 * Define the commands that we will handle, and the types specific to the
 * current O/S
 */
#ifndef OS_WINDOWS
# define VALID_COMMANDS  "XSUT"
#else
# define VALID_COMMANDS  "XSU"
#endif

#ifdef OS_WINDOWS
	typedef HANDLE LockFileHandle;
	typedef DWORD FileOffset;
#else
    typedef int LockFileHandle;
    typedef long FileOffset;
#endif



/**
 * print command line option help
 */
static void printHelpAndExit(char *progname, char *errormessage) {
	if (errormessage != NULL) {
		fprintf(stderr, "Error: %s\n\n", errormessage);
    }

	fprintf(stderr, "%s <datafile> [ <commandfile> ]\n", progname);
	fprintf(stderr, "    Commands come in the form:\n");
	fprintf(stderr, "        OPN OFF LEN\n");
	fprintf(stderr, "    where:\n");
	fprintf(stderr, "        OPN (operation) is one of:\n");
	fprintf(stderr, "           X : exclusive lock of region, waiting for availbility \n");
	fprintf(stderr, "           S : shared lock of region, waiting for availbility \n");
#ifndef  OS_WINDOWS
	fprintf(stderr, "           T : test whether region is locked\n");
#endif
	fprintf(stderr, "           U : unlock region\n");
	fprintf(stderr, "        OFF, and LEN are the offset and length in bytes\n");
	fprintf(stderr, "    If <commandfile> is not given, stdin is read\n");

	exit (-1);
}

/* wrap ctime(3) to get the current time in a string */
static char * curtimeString() {
	time_t curtime;

	/** get the time right now in seconds since the epoch */
	curtime = time(NULL);

	/* ctime(3) inconveniently takes the address of a time value */
	return ctime(&curtime);
}

/**
 * Perform the work of doing the locking operation, whatever it is.
 */
int doLockAction(LockFileHandle lockFileHandle, char operation, FileOffset offset, FileOffset length) {
	char *operationTag;
#ifndef OS_WINDOWS
	struct flock lockData;
#else
	OVERLAPPED windowsOverlap;
	int status, typeCode, opCode;
#endif


	/**
	 * Get a nice printable version of the lock operation
	 */
	operationTag = (operation == 'X' ? "Exclusive Lock": operation == 'S' ? "Shared Lock" :
						operation == 'T' ? "Test for lock" : operation == 'U' ? "Unlock" : "??? unknown command ???");

	printf("PID %5d performing operation %s\n", getpid(), operationTag);


	/**
	 * Code for Windows -- set up the OVERLAPPED structure, and use it
	 * to create a lock.  This structure is used with the LockFileEx()
	 * function to store the offset portion of our request.
	 */
#ifdef OS_WINDOWS
	memset(&windowsOverlap, 0, sizeof(windowsOverlap));
	windowsOverlap.hEvent = lockFileHandle;
	windowsOverlap.Offset = offset;
	windowsOverlap.OffsetHigh = 0;
#else
    memset(&lockData, 0, sizeof(lockData));
    lockData.l_type = F_UNLCK;
    lockData.l_whence = SEEK_SET;
    lockData.l_start = offset;
    lockData.l_len = length;

#endif

	/**
	 * Print a message indicating that we are going into the lock request
	 */
	printf("PID %5d : requesting '%c' lock at %ld for %ld bytes at %s\n", getpid(), operation, offset, length, curtimeString());


	/**
	 * If we want a lock, use LockFileEx() - otherwise give up the
	 * lock with UnlockFile()
	 */
	if (operation == 'X' || operation == 'S') {
		/**
		 * shared locks are the default (no value) so only exclusive
		 * locks have a bit flag
		 */
		if (operation == 'X') {
#ifdef OS_WINDOWS
			typeCode = LOCKFILE_EXCLUSIVE_LOCK;
#else
            lockData.l_type = F_WRLCK;
#endif
        }
		else {
#ifdef OS_WINDOWS
			typeCode = 0;
#else
            lockData.l_type = F_RDLCK;
#endif
        }

	
		/**
		 * Use the OVERLAP structure and our other values to request
		 * the lock
		 */
#ifdef OS_WINDOWS
        if ( ! LockFileEx(lockFileHandle, typeCode, 0, length, 0, &windowsOverlap)) {
			fprintf(stderr, "Error: LockFile returned failure : %d\n", GetLastError());
			return -1;
		}
#else
        //lock
        if (fcntl(lockFileHandle, F_SETLK, &lockData) == -1) {
            fprintf(stderr, "Error: Locking failure: '%s'\n", strerror(errno));
        }
#endif
    }
    else if (operation == 'U') {

		/**
		 * Release a lock on a portion of a file
		 */
#ifdef OS_WINDOWS
		if ( ! UnlockFile(lockFileHandle, offset, 0, length, 0)) {
			fprintf(stderr, "Error: LockFile returned failure : %d\n", GetLastError());
			return -1;
		}
#else
        //unlock
        lockData.l_type = F_UNLCK;
        if (fcntl(lockFileHandle, F_SETLK, &lockData) == -1) {
            fprintf(stderr, "Error: Unlocking failure: '%s'\n", strerror(errno));
        }
#endif
	} 
#ifndef OS_WINDOWS
    else if (operation == 'T') {
        if (fcntl(lockFileHandle, F_GETLK, &lockData) == -1) {
            fprintf(stderr, "Error: Get lock: '%s'\n", strerror(errno));
        }
        else if (lockData.l_type != F_UNLCK) {
            printf("File is locked\n");
            printf("Owner: %d\n", lockData.l_pid);
            printf("Lock Type: %hd\n", lockData.l_type);
#ifndef OS_LINUX
            printf("Lock Offset: %lld\n", lockData.l_start);
            printf("Lock Length: %lld\n", lockData.l_len);
#else
            printf("Lock Offset: %ld\n", lockData.l_start);
            printf("Lock Length: %ld\n", lockData.l_len);
#endif

            printf("Lock Whence: %hd\n", lockData.l_whence);
        }
        else {
            printf("File is unlocked\n");
        }
    }
#endif
    else {
		fprintf(stderr, "Error: Unknown operation '%c'\n", operation);
		return -1;
	}

	/** Tell the user what happened */
	printf("PID %5d : received   '%c' lock at %ld for %ld bytes at %s",
			getpid(), operation, offset, length, curtimeString());


	return 0;
}


/**
 * System mainline -- simply process the arguments and input lines
 */
int main(int argc, char *argv[]) {
	/** use O/S specific type to track the right kind of file handle */
	LockFileHandle lockFileHandle;
	char commandLine[BUFSIZ];
	FILE *commandFP = stdin;
	int nItemsRead, keepGoing = 1;
	long offset, length;
	char operation;


	/** check that the arguments are correct */
	if (argc != 2 && argc != 3) {
        printHelpAndExit(argv[0], "incorrect arguments");
    }

	/**
	 * if we need to open a data file, we can do so with fopen() so that we
	 * can use fgets()
	 */
	if (argc == 3) {
		if ((commandFP = fopen(argv[2], "r")) == NULL) {
			fprintf(stderr, "Cannot open command file '%s' : %s\n", argv[2], strerror(errno));
			exit (-1);
		}
	}

	
	/** Windows specific file open */
	/*
    lockFileHandle = CreateFile(argv[1],
		GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
		FILE_SHARE_READ | FILE_SHARE_WRITE, //  dwShareMode
		NULL,           // lpSecurityAttributes
		OPEN_EXISTING,  // dwCreationDisposition
		FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAtributes
		NULL);  // hTemplateFile
    */
#ifdef OS_WINDOWS
    lockFileHandle = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


	if (lockFileHandle == NULL) {
		fprintf(stderr, "Failed opening '%s' : %d\n", argv[1], GetLastError());
		exit (-1);
	}
#else 
    lockFileHandle = open(argv[1], O_RDWR);
    if (lockFileHandle < 0) {
        fprintf(stderr, "Failed opening '%s'\n", argv[1]);
        exit (-1);
    }
#endif

	/**
	 * Print out the help overview on all O/S (but there is no test function
	 * on Windows)
	 */
	printf("Processing locks on '%s'\n", argv[1]);
	printf("Available commands:\n");
	printf("    X <start> <len>   : exclusive lock of <len> bytes from <start>\n");
	printf("    S <start> <len>   : shared lock of <len> bytes from <start>\n");
#ifndef OS_WINDOWS
	printf("    T <start> <len>   : test of lock of <len> bytes from <start>\n");
#endif
	printf("    U <start> <len>   : unlock <len> bytes from <start>\n");
	printf("    Q                 : quit the program\n");


	/** print first prompt */
	if (isatty(0)) {
		printf("CMD> ") >= 0 && fflush(stdout);
    }
	/**
	 * loop reading commands
	 * we use fgets() to ensure that we are reading whole lines
	 */
	while (keepGoing && fgets(commandLine, BUFSIZ, commandFP) != NULL) {

		/** scanf can do this sort of processing pretty well */
		nItemsRead = sscanf(commandLine, "%c %ld %ld", &operation, &offset, &length);

		int upperOperation = toupper(operation);

		/** if we get some flavour of Q, we are bailing out */
		if (upperOperation != 'Q') {
			/** make sure that we have enough data */
			if (nItemsRead < 3 || strchr(VALID_COMMANDS, upperOperation) == NULL) {
				fprintf(stderr, "Invalid operation - need \"OPN OFF LEN\"\n");
            }
			else {
				printf("CMD %c %2ld %2ld\n", upperOperation, offset, length);

				/** we ignore the results of the lock action here */
				(void) doLockAction(lockFileHandle, upperOperation, offset, length);
			}

			/* prompt again */
			if (isatty(0)) {
				printf("CMD> ") >= 0 && fflush(stdout);
            }
		}
        else {
			printf("Exitting ...\n");
			keepGoing = 0;
		}
	}


	/** clean up our files */
#ifdef OS_WINDOWS
	CloseHandle(lockFileHandle);
#else
    close (lockFileHandle);
#endif
	if (commandFP != stdin) {
        fclose(commandFP);
    }

	exit(0);
}
