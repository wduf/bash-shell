// includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

// defines
#define ARGS_SIZE 99
#define ARGS_LENGTH 99
#define INPUT_LENGTH 999
#define PATH_LENGTH 999
#define BASENAME_LENGTH 99
#define INPUT_HISTORY_SIZE 10
#define INPUT_HISTORY_LENGTH 999

// functions
// WORKING
void initializeStringArray(char** arr, int size, int length)
{  // allocates memory on heap for args string array
	for(int i = 0; i < size; i++)
	{
		arr[i] = (char*) malloc(length * sizeof(char));
		arr[i] = 0;
	}
}

// WORKING
void printArgs(char** args)
{  // prints all the strings in args
	for(int i = 0; i < ARGS_SIZE; i++)
	{
		if(args[i] != 0)
		{
		    printf("args[%d] = '%s'\n", i, args[i]);
		} else
		{  // debug
		    //printf("args[%d] is NULL\n", i);
		}
	}
}

/*
  working but if last char is a space, it cuts it off
*/
void splitArgs(char* input, char** args)
{

	char* curr_string = strtok(input, " ");
	int i;
	for(i = 0; curr_string != NULL; i++)
	{  // loop until end of input
		curr_string[strcspn(curr_string, "\n")] = 0;  // removes the trailing newline
		args[i] = curr_string;
		curr_string = strtok(NULL, " ");
	}
}

/*
  WORKING BUT IF LAST CHAR IS A SPACE, TREATS THAT AS "" ARG
*/
void readUserInput(char* input, char* basename)
{
	printf("%s$ ", basename);
	fgets(input, INPUT_LENGTH, stdin);  // reads line from stdin
}

// WORKING
void getPath(char* path)
{
	getcwd(path, PATH_LENGTH);
}

// WORKING
void getBasename(char* path, char* basename)
{
	char* curr_string = strtok(path, "/");
	char* prev_string;
	for(int i = 0; curr_string != NULL; i++)
	{  // loop until end of curr_string
		prev_string = curr_string;
		curr_string = strtok(NULL, "/");
	}
	memcpy(basename, prev_string, strlen(prev_string));  // set basename = prev_string
}

// WORKING
void cd_(char** args, char* path, char* basename, int* exit_value)
{
	if(args[2] != 0)
	{  // if more than one argument
		printf("wshell: cd: too many arguments\n");
	} else if((args[1] == 0) || (!strcmp(args[1], "~")))
	{  // if no arguments    
		chdir(getenv("HOME"));    
	} else if(chdir(args[1]) == 0)
	{  // if directory exists
		getPath(path);
		getBasename(path, basename);
	} else
	{
		printf("wshell: no such directory: %s\n", args[1]);
		*exit_value = 1;
	}
}

// WORKING
void pwd_()
{
	char absolute_path[500];
	printf("%s\n", getcwd(absolute_path, 500));
}

// WORKING
void echo_(char** args)
{
	for(int i = 1; args[i] != NULL; i++)
	{
		if(i > 1)
		{
			printf(" ");
		}
		printf("%s", args[i]);
	}
	puts("");  // newline
}

// WORKING
void history_(char** args, char** input_history)
{
	if(args[1] != 0)
	{  // argument given, history should not have any arguments
		printf("wshell: history: too many arguments\n");
	} else
	{
		for(int i = (INPUT_HISTORY_SIZE - 2); i > -1; i--)  // -2 because main prints history, so that's one less this needs to count
		{
		    if(input_history[i] != 0)
		    {
			printf("%s\n", input_history[i]);
		    }
		}
	}
}

// WORKING
void recordInput(char* input, char** input_history)
{
	for(int i = (INPUT_HISTORY_SIZE - 1); i > 0; i--)
	{
		input_history[i] = input_history[i - 1];
	}
	input_history[0] = input;
}

// WORKING
void splitCommands(char** args, char** remainder_args)
{
	int found = 0;  // if op found
	int j = 0;  // remainder_args index
	for(int i = 0; i < ARGS_SIZE; i++)
	{
		if(args[i] != NULL)
		{	
			if((!strcmp(args[i], "&&") || (!strcmp(args[i], "||"))) /* more ops here */)
			{
				found = 1;
			}
			if(found)
			{
				remainder_args[j] = args[i];  // writes everything starting at first op to remainder_args
				args[i] = 0;  // deletes everything starting at first op from args
				j++;
			}
		}
	}
}

// WORKING
int opCheck(char* curr_op)
{  // checks if first character in a string is an op, sets variable to that op, returns 1 on op found
	int R = 0;
	
	if(curr_op != NULL)
	{
		if((!strcmp(curr_op, "&&") || (!strcmp(curr_op, "||"))) /* more ops here */)
		{
			R = 1;
		}
	}
	return R;  // returns 1 if op found, 0 otherwise
}

// WORKING
void removeFrontOp(char** args, char* curr_op)
{  // moves every string one position forward in array, removes first one
	int i;
	if(args[0] != NULL)
	{
		memcpy(curr_op, args[0], 5);  // does not check for null, this might be a problem later
		for(i = 0; args[i + 1] != NULL; i++)
		{
			args[i] = args[i + 1];
		}
		args[i] = 0;
	}
}

// WORKING
void runCommands(char** args, char* path, char* basename, char** input_history, int* exit_value)
{
	if(!strcmp(args[0],"cd"))
	{  // returns 0 if strings are equal
		cd_(args, path, basename, exit_value);
	} else if(!strcmp(args[0], "pwd"))
	{
		pwd_();  // can't fail
	} else if(!strcmp(args[0], "echo"))
	{
		echo_(args); // can't fail
	} else if(!strcmp(args[0], "history"))
	{
		history_(args, input_history);  // can't fail
		printf("history\n");
	} else 
	{
	pid_t pid = fork();
		if(pid != 0)  // if not an internal command
		{  // parent code
			int child_status;
			waitpid(pid, &child_status, 0);
			*exit_value = WEXITSTATUS(child_status);
		} else
		{  // child code
			execvp(args[0], args);
			printf("wshell: could not execute command: %s\n", args[0]);
			exit(0);
		}
	}
}

int production(int argc, char* argv[], char** input_history)
{
	// variables
	char* input = (char*) malloc(INPUT_LENGTH * sizeof(char));
	char* path = (char*) malloc(PATH_LENGTH * sizeof(char));
	char* basename = (char*) malloc(BASENAME_LENGTH * sizeof(char));
	char** args = (char**) malloc(ARGS_SIZE * sizeof(char*));
	char** remainder_args = (char**) malloc(ARGS_SIZE * sizeof(char*));
	char* curr_op = (char*) malloc(20 * sizeof(char));  // 5 is slight overkill, really only need 2-3
	int exit_value = 0;  // exit value for previous exec, 26 is default non-standard case

	// default non-NULL op value
	memcpy(curr_op, "", 1);

	// set up string arrays
	initializeStringArray(args, ARGS_SIZE, ARGS_LENGTH);
	initializeStringArray(remainder_args, ARGS_SIZE, ARGS_LENGTH);

	// set up path and basename
	getPath(path);
	getBasename(path, basename);

	// get user input
	readUserInput(input, basename);
	input[strcspn(input, "\n")] = 0;  // remove the trailing newline, https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input

	// reads from file, necessary for test cases
	if(!isatty(fileno(stdin)))
	{
		printf("%s\n", input);  // added newline for convenience
		fflush(stdout);
	}

	// separate user input string into array of words in that string
	char* input_copy = (char*) malloc(INPUT_LENGTH * sizeof(char));  // strtok() modifies its buffer
	memcpy(input_copy, input, INPUT_LENGTH);
	splitArgs(input_copy, args);
	
	// splits up commands by ops
	splitCommands(args, remainder_args);
		
		
	int should_i_exec = 1;  // default 1, set to 0 if op isn't supposed to exec next command
	// code execution loop
	while(args[0] != NULL)
	{  // while there are still arguments
		if(opCheck(curr_op))
		{  // 1 if op found
			if(!strcmp(curr_op, "&&"))
			{
				if(exit_value == 0)
				{
					//printf("i should exec\n");
					should_i_exec = 1;
				} else
				{
					//printf("i'm not allowed to exec\n");
					should_i_exec = 0;
				}
			} else if(!strcmp(curr_op, "||"))
			{
				if(exit_value == 0)
				{
					should_i_exec = 0;
				} else
				{
					should_i_exec = 1;
				}
			}
		} 
		if(should_i_exec)
		{  // no op
			if(!strcmp(args[0], "exit"))
			{
				free(input);
				free(path);
				free(basename);
				free(args);
				free(remainder_args);
				free(curr_op);
				free(input_copy);
				return 0;
			}
			runCommands(args, path, basename, input_history, &exit_value);
		} else 
		{
			break;
		}
		
		removeFrontOp(remainder_args, curr_op);
		memcpy(args, remainder_args, ARGS_SIZE);
		initializeStringArray(remainder_args, ARGS_SIZE, ARGS_LENGTH);
		splitCommands(args, remainder_args);
		should_i_exec = 1;
	}  // while loop
	
	// record input for history command
	recordInput(input, input_history);

	return production(argc, argv, input_history);
}


//
int main(int argc, char* argv[])
{
	// initialized in main because i need this to be preserved between recursive calls
	char** input_history = (char**) malloc(INPUT_HISTORY_SIZE * sizeof(char**));
	initializeStringArray(input_history, INPUT_HISTORY_SIZE, INPUT_HISTORY_LENGTH);

	production(argc, argv, input_history);

	free(input_history);
	exit(0);
}
