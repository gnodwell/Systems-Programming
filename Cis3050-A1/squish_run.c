#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "squish_run.h"
#include "squish_tokenize.h"


/**
 * Print a prompt if the input is coming from a TTY
 */
static void prompt(FILE *pfp, FILE *ifp)
{
	if (isatty(fileno(ifp))) {
		fputs(PROMPT_STRING, pfp);
	}
}

/**
 * Actually do the work
 */


int *findCommands(char **const tokens, int nTokens) {
	int *commandIndx;

	commandIndx = (int *)malloc(sizeof(int));
	commandIndx[0] = 0;
	int j = 1;

	for (int i = 0; i < nTokens; i++) {
		if (strcmp("|", tokens[i]) == 0) {
			/* printf("Next command --> %s\n", tokens[i+1]); */
			commandIndx = (int *)realloc(commandIndx, sizeof(int) * (i+1));
			commandIndx[j] = i+1;
			j++;
		}
		else if (i < nTokens) {
			commandIndx = (int *)realloc(commandIndx, sizeof(int) * (i+1));
			commandIndx[j] = -1;
		}
	}

	return commandIndx;
}


int *findRedirections (char **const tokens, int nTokens) {
	int *redirectIndx;

	redirectIndx = (int *)malloc(sizeof(int));
	int j = 0;

	for (int i = 0; i < nTokens; i++) {
		if (strcmp("<", tokens[i]) == 0 || strcmp(">", tokens[i]) == 0) {
			redirectIndx = (int *)realloc(redirectIndx, sizeof(int) * (i+1));
			redirectIndx[j] = i;
			j++;
		}
		else if (i < nTokens) {
			redirectIndx = (int *)realloc(redirectIndx, sizeof(int) * (i+1));
			redirectIndx[j] = -1;
		}
	}

	return redirectIndx;

}


char **getNextCommand(char **const tokens, int start, int end) {
	char** command;

	command = malloc(sizeof(char*) * (end-start+1));

	for (int i = start; i < end-1; i++) {
		command[i] = malloc(sizeof(char) * strlen(tokens[i]));
		strcpy(command[i], tokens[i]);
		printf("test copy --> %s\n", command[i]);
	}

	return command;
}


/* int countArg */


char **getCommand(char **const tokens, int nTokens, int pos) {
    int counter = 0;
    int j = 0;
    char **returnArr;
    returnArr = malloc(sizeof(char *) *nTokens);

    for (int i = 0; i < nTokens; i++) {
        if (strcmp("|", tokens[i]) == 0) {
            counter++;
        }
        if (counter == pos && strcmp("|", tokens[i]) != 0) {
            returnArr[j] = malloc(sizeof(char) * strlen(tokens[i]));
            strcpy(returnArr[j], tokens[i]);
            j++;
        }
    }
    returnArr[j] = NULL;

    return returnArr;
}





int execFullCommandLine(
		FILE *ofp,
		char ** const tokens,
		int nTokens,
		int verbosity)
{
	if (verbosity > 0) {
		fprintf(stderr, " + ");
		fprintfTokens(stderr, tokens, 1);
	}

	/** Now actually do something with this command, or command set */

	/*List of arguments to search for
	 *
	 * Search for different commands in the tokens
	 * run the different commands in children using pipes
	 *
	 *
	 *
	 */

	int *commandIndx;
	int *redirectIndx;

	commandIndx = findCommands(tokens, nTokens);
	redirectIndx = findRedirections(tokens, nTokens);


	int commandCount = 0;
	for (int i = 0; commandIndx[i] != -1; i++) {
		commandCount++;
	}

    printf("tokens --> ");
    for (int i = 0; i < nTokens; i++) {
        printf("%s ", tokens[i]);
    }
    printf("\n");


    int pos = 0;
    int counter = 0;
    char **currentCommand;
    int fd[2];
    pipe(fd);
    pid_t pid;
    int status;

    printf("I am here\n");
    printf("I am also here\n");



    for (int i = 0; i < commandCount; i++) {
        currentCommand = getCommand(tokens, nTokens, i);
        printf("current command --> ");
        for (int k = 0; currentCommand[k] != NULL; k++) {
            printf("%s ", currentCommand[k]);
        }
        printf("\n");
        
        if (i == 0 && commandCount == 1) {
            //execute first command
            printf("first route\n");
            pid = fork();
            if (pid == 0) {
                close(fd[0]);
                close(fd[1]);
                //execlp(currentCommand[0], *currentCommand, (char *) NULL);
                execvp(currentCommand[0], currentCommand);
                exit(1);
            }
            else {
                close(fd[0]);
                close(fd[1]);
	 		    waitpid(pid, &status, 0);
                printf("Finsihed first route\n");
            }
        }

        else if (i == 0 && commandCount > 1){
            printf("second route\n");
            pid = fork();
            if (pid == 0) {
                close(fd[0]);
                dup2(fd[1], 1);
                close(fd[1]);
                //execlp(currentCommand[0], *currentCommand, (char *) NULL);
                execvp(currentCommand[0], currentCommand);
                exit(1);
            }
            else {
                close(fd[0]);
                close(fd[1]);
	 		    waitpid(pid, &status, 0);
                printf("finsihed second route\n");
            }
        }
        else if (i > 0 && i < commandCount-1) {
            printf("third route\n");
            pid = fork();
            if (pid == 0) {
                //dup2(fd[0], 0);
                //dup2(fd[1], 1);
                //close(fd[0]);
                //close(fd[1]);
                //execlp(currentCommand[0], *currentCommand, (char *) NULL);
                printf("test control\n");
                execvp(currentCommand[0], currentCommand, );
                printf("test control\n");
                exit(1);
            }
            else {
                close(fd[0]);
                close(fd[1]);
	 		    waitpid(pid, &status, 0);
                printf("finsihed third route\n");
            }
        }
        else {
            //execute following commands
            printf("fourth route\n");
            pid = fork();
            if (pid == 0) {
                close(fd[1]);
                dup2(fd[0], 0);
                close(fd[0]);
                //execlp(currentCommand[0], *currentCommand, (char *) NULL);
                execvp(currentCommand[0], currentCommand);
                exit(1);
            }
            else {
                close(fd[0]);
                close(fd[1]);
	 		    waitpid(pid, &status, 0);
                printf("Finsihed fourth route\n");
            }

        }
        close(fd[0]);
        close(fd[1]);

    }

    close(fd[0]);
    close(fd[1]);



    





	/* char *firstCmd = "echo"; */
	/* char *firstArg = "helloWorld"; */
	/* char *secondCmd = "echo"; */
	/* char *secondArg = "shit"; */
    /*  */
	/* pid_t pid; */
	/* int fd[2]; */
    /*  */
	/* pipe(fd); */
    /*  */
	/* pid = fork(); */
    /*  */
	/* if (pid == 0) { */
	/* 	printf("I am the child\n"); */
    /*  */
	/* 	dup2(fd[1], 1); */
	/* 	close(fd[0]); */
	/* 	close(fd[1]); */
    /*  */
	/* 	printf("tokens[0] --> %s\n", tokens[0]); */
	/* 	execlp(firstCmd, firstCmd, firstArg, (char *) NULL); */
	/* 	fprintf(stderr, "Failed to execute '%s'\n", tokens[0]); */
	/* 	exit(1); */
	/* } */
	/* else { */
	/* 	pid = fork(); */
    /*  */
	/* 	if (pid ==  0) { */
	/* 		dup2(fd[0], 0); */
	/* 		close(fd[0]); */
	/* 		close(fd[1]); */
	/* 		execlp(secondCmd, secondCmd, secondArg, (char *)NULL); */
	/* 		fprintf(stderr, "Failed to execute '%s'\n", tokens[0]); */
	/* 		exit(1); */
    /*  */
	/* 	} */
	/* 	else { */
	/* 		int status; */
	/* 		close(fd[0]); */
	/* 		close(fd[1]); */
	/* 		waitpid(pid, &status, 0); */
	/* 	} */
	/* } */





	/* printf("nTokens --> %d\n", nTokens); */
	/* for (int i = 0; i < nTokens; i++) { */
	/* 	printf("Token --> %s\n", tokens[i]); */
	/* } */

    /*  */
	/* int id = fork(); */
	/* #<{(| int fd[2]; |)}># */
	/* #<{(| pipe(fd); |)}># */
	/* if (id == 0) { */
	/* #<{(| 	close(fd[0]); |)}># */
	/* #<{(| 	dup2(fd[1], 1); |)}># */
	/* #<{(| 	dup2(fd[1], 2); |)}># */
	/* #<{(| 	close(fd[1]); |)}># */
    /*  */
    /*  */
	/* 	printf("I am the child\n"); */
	/* 	for (int i = 0; i < nTokens; i++) { */
	/* 		printf("Token --> %s", tokens[i]); */
	/* 	} */
	/* 	#<{(| execl("/bin/echo", *tokens, NULL); |)}># */
	/* 	#<{(| execl("/bin/ls", "ls", NULL); |)}># */
    /*  */
	/* 	#<{(| execv("/bin/echo", tokens); |)}># */
    /*  */
	/* 	execlp("echo", "echo", "test", "execlp", (char *) NULL); */
	/* 	#<{(| execlp(*tokens, (char *)NULL); |)}># */
	/* 	printf("test control\n"); */
	/* } */
	/* else { */
	/* 	wait(NULL); */
	/* 	printf("I am the main program\n"); */
	/* } */









	return 1;
}

/**
 * Load each line and perform the work for it
 */
int
runScript(
		FILE *ofp, FILE *pfp, FILE *ifp,
		const char *filename, int verbosity
	)
{
	char linebuf[LINEBUFFERSIZE];
	char *tokens[MAXTOKENS];
	int lineNo = 1;
	int nTokens, executeStatus = 0;

	fprintf(stderr, "SHELL PID %ld\n", (long) getpid());

	prompt(pfp, ifp);
	while ((nTokens = parseLine(ifp,
				tokens, MAXTOKENS,
				linebuf, LINEBUFFERSIZE, verbosity - 3)) > 0) {
		lineNo++;

		if (nTokens > 0) {

			executeStatus = execFullCommandLine(ofp, tokens, nTokens, verbosity);

			if (executeStatus < 0) {
				fprintf(stderr, "Failure executing '%s' line %d:\n    ",
						filename, lineNo);
				fprintfTokens(stderr, tokens, 1);
				return executeStatus;
			}
		}
		prompt(pfp, ifp);
	}

	return (0);
}


/**
 * Open a file and run it as a script
 */
int
runScriptFile(FILE *ofp, FILE *pfp, const char *filename, int verbosity)
{
	FILE *ifp;
	int status;

	ifp = fopen(filename, "r");
	if (ifp == NULL) {
		fprintf(stderr, "Cannot open input script '%s' : %s\n",
				filename, strerror(errno));
		return -1;
	}

	status = runScript(ofp, pfp, ifp, filename, verbosity);
	fclose(ifp);
	return status;
}

