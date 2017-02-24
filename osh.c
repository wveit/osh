#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_SIZE 80
#define MAX_TOKENS 10
#define TRUE 1
#define FALSE 0

static void extractTokens(char *inputString, char * tokenList[], int *numTokens);
static void printTokens(char * tokenList[]);
static void resetTokenList(char * tokenList[], int *numTokens);

int main(void)
{
	/* Set up variables */
	char buffer[BUFFER_SIZE] = "";
	char *tokenList[MAX_TOKENS + 1];
	int numTokens = 0;
	int parentShouldWait = TRUE;
	pid_t pid;
	
	tokenList[0] = NULL;
	
	while(1)
	{
		/* Send "osh>" prompt to console */
		printf("\nosh> ");
	
		/* Get input string from console */
		fgets(buffer, BUFFER_SIZE, stdin);
		puts(buffer);
		
		/* Extract tokens from input string and store in 2D char array tokenList */
		extractTokens(buffer, tokenList, &numTokens);
		printTokens(tokenList);
		
		/* If there are no tokens, skip the rest of this iteration of loop */
		if(numTokens == 0)
		{
			continue;
		}
		
		/* If first token is exit, exit this shell */
		if(strcmp(tokenList[0], "exit") == 0)
		{
			resetTokenList(tokenList, &numTokens);
			exit(0);
		}
		
		/* 
		 * If last token is '&', set parentShouldWait = TRUE and remove that token from 
		 * list. Otherwise, set parentShouldWait = FALSE.
		 */
		if(strcmp(tokenList[numTokens - 1], "&") == 0)
		{
			parentShouldWait = FALSE;
			
			free(tokenList[numTokens - 1]);
			tokenList[numTokens - 1] = NULL;
			numTokens--;
		}
		else
		{
			parentShouldWait = TRUE;
		}
		
		printf("parentShouldWait: %d\n", parentShouldWait);
		
		/* Fork process and store pid */
		pid = fork();
		
		/* If pid < 0 handle error */
		if(pid < 0)
		{
			printf("There was an error. Exiting.\n");
			exit(1);
		}
		
		/* Else if pid == 0, child process should execute program with argv */
		else if(pid == 0)
		{
			execvp(tokenList[0], tokenList);
			
			/*
			 * If we get here, there was a problem executing the given command.
			 * Give an error message and end this child process.
			 */
			 
			printf("Error: Could not start that program.\n");
			exit(1);
		}
		
		/* Else (pid > 0), parent process should wait if parentShouldWait == true */
		else if(parentShouldWait)
		{
			wait(NULL);
		}
		
		/* Cleanup tokenList to prevent memory leaks */
		resetTokenList(tokenList, &numTokens);
	}
	
}


static void extractTokens(char *inputString, char * tokenList[], int *numTokens)
{

/*

Algorithm Description:

Loop:
	If startIndex is on '\0' -> we have collected all tokens and are done.
		- Add NULL pointer to tokenList.
		- End this loop.
	If startIndex is on whitespace -> startIndex is not marking the beginning of a token
		- Increment startIndex until it is no longer on whitespace.
		- Set lastIndex equal to startIndex.
	If lastIndex is on a character -> lastIndex is not marking the end of a token
		- Increment lastIndex until it is no longer on a character. 
		- Add the current token (inputString[startIndex] to inputString[endIndex - 1]) 
			to the tokenList. 
		- Set startIndex = lastIndex.
	
*/

	int startIndex = 0;
	int lastIndex = 0;
	int tokenLength;
	
	while(TRUE)
	{
		if(inputString[startIndex] == '\0')
		{
			tokenList[*numTokens] = NULL;
			break;
		}
		else if(isspace(inputString[startIndex]))
		{
			while(isspace(inputString[startIndex]))
			{
				startIndex++;
			}
			lastIndex = startIndex;
		}
		else if(!isspace(inputString[lastIndex]) && inputString[lastIndex] != '\0')
		{
			while(!isspace(inputString[lastIndex]) && inputString[lastIndex] != '\0')
			{
				lastIndex++;
			}
			
			tokenLength = lastIndex - startIndex;
			tokenList[*numTokens] = (char*) malloc(tokenLength + 1);
			strncpy(
				tokenList[*numTokens], 
				inputString + startIndex, 
				lastIndex - startIndex + 1);
			tokenList[*numTokens][tokenLength] = '\0';
			(*numTokens)++;
			
			startIndex = lastIndex;
		}
	}

}

static void printTokens(char * tokenList[])
{
	int i = 0;

	printf("\n=========================================\n");
	printf("  Printing Tokens\n");
	printf("-----------------------------------------\n");
	
	while(tokenList[i] != NULL)
	{
		printf("     %s\n", tokenList[i]);
		i++;
	}
	
	printf("=========================================\n\n");
}

static void resetTokenList(char * tokenList[], int *numTokens)
{
	int index = 0;
	while(tokenList[index] != NULL)
	{
		free(tokenList[index]);
		tokenList[index] = NULL;
		index++;
	}
	
	*numTokens = 0;
}