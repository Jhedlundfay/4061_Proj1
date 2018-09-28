#include "util.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>


int parse_into_tokens(char *input_string, char *tokens[], char *delim)
{
	int i=0;
	char *tok = strtok(input_string, delim);
	while(tok!=NULL && i<ARG_MAX)
		{
		tokens[i] = tok;
		i++;
		tok = strtok(NULL, delim);
		}
	tokens[i] = NULL;
	return i;
}
int main(){
        char input[] = "gcc abc def";
        char *cmd[1023]; 
        int result = parse_into_tokens(input,cmd," ");  
        //printf("%s",cmd[0]);

	char *sub= cmd[2];
	printf("%s",sub);

}
