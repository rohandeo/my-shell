#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

#define MAX_INPUT_CHARS 1024
#define MAX_ARGUMENTS 25
#define MAX_ARGUMENT_CHARS 10
#define MAX_PIPED_COMMANDS 10

char* get_sub_dir(){ //returns the current working directory of the user
	char string_buffer[1024];
	char* current_directory = getcwd(string_buffer, sizeof(string_buffer));
	return current_directory;
}
void print_user_info(){ //prints outs cwd and username
	printf("\033[1;32m");
	printf("\nENTS669C");
	printf("\033[0m");
	printf(":");
	printf("\033[0;34m");
	printf("%s", get_sub_dir());
	printf("\033[0m");
	printf("$");
}

int parse_for_char(char* input_command, int find){ //parses the input_command and returns a flag indicating presence of semi colon
	
	char* index = strchr(input_command, find);
	if(index == NULL) //return false if semi colon not found
		return 0;
	else //return true if semi colon found
		return 1;
}

static void execute_command(char* input_command, char** parsed_command, int pipe_exists, int ampersand_exists){ // execute the command according to execution flow

	// code block to execute a piped command
	if(pipe_exists == 1){
		char* temp_string;
		char** single_pipe_string =  malloc(MAX_ARGUMENTS * (sizeof(char *)));
		int i, j = 0;
        for(j = 0; j < MAX_PIPED_COMMANDS; j++){
            // initi file descriptor and process variable to capture return value for fork()
            i = 0;
            int file_descriptor[2];
            pid_t process_id; 
            int temp_file_descriptor;
            if(parsed_command[j] == NULL)
                break;
            // code block to separate individual piped commands by whitespace
            temp_string = strtok(parsed_command[j]," ");
            while(temp_string != NULL){
                single_pipe_string[i++] = temp_string;                        
                temp_string = strtok(NULL, " ");
            }
            single_pipe_string[i] = NULL;
            // code block to execute child process
            pipe(file_descriptor);
            process_id = fork();
            if (process_id == 0){ //child process
                dup2(temp_file_descriptor,0); 
                    if(parsed_command[j+1] != NULL){
                        dup2(file_descriptor[1], 1); 
                    }
                    close(file_descriptor[0]); 
                    execvp(single_pipe_string[0], single_pipe_string); 
                    exit(0); 
                } 
            else{ //parent process
                wait(NULL);
                close(file_descriptor[1]);
                temp_file_descriptor = file_descriptor[0];
            }
        }
	}
	// pipe doesn't exist, look for ampersands and semi colons
	else{
		if(ampersand_exists){// ampersand exists
			//init 2-d array to store individual commands preceding ampersand
			char** space_separated_command = (char**)malloc(MAX_ARGUMENTS * sizeof(char*));
			for (int j = 0; j < MAX_ARGUMENTS; j++)
				space_separated_command[j] = (char*)malloc(MAX_ARGUMENT_CHARS * sizeof(char*));
			//code block to store and execute whitespace separated commands
			int k = 0;
			while(parsed_command[k] != NULL){
				char* found = strtok(parsed_command[k++], " ");	
				int number_of_arguments = 0;
				while(found != NULL){
					strcpy(space_separated_command[number_of_arguments++], found);
					found = strtok(NULL, " ");
				}
				space_separated_command[number_of_arguments] = NULL;	
				
				int ret = fork();
				if(ret == 0){
					execvp(space_separated_command[0], space_separated_command);
				}
				else{ //background process doesn't wait
					// continue
				}
			}
		}
		else{// ampersand doesn't exist
			int ret = fork();
			if(ret == 0){
				execvp(parsed_command[0], parsed_command);
				exit(0);
			}
			else{
				wait(NULL);
			}
		}
	}
}

void parse_command(char* input_command, int execution_flow){ // parse the command according to the execution flow

	int pipe_exists = 0;
	int ampersand_exists;
	char** parsed_command = (char**)malloc(MAX_ARGUMENTS * sizeof(char*));
	for (int j = 0; j < MAX_ARGUMENTS; j++)
		parsed_command[j] = (char*)malloc(MAX_ARGUMENT_CHARS * sizeof(char*));

	if(execution_flow == 0){// no semi colon or ampersand found, split by whitespace
		// code block to separate command by whitespace and execute it
		ampersand_exists = 0;
		char* found = strtok(input_command, " ");	
		int number_of_arguments = 0;
		while(found != NULL){
			parsed_command[number_of_arguments++] = found;
			found = strtok(NULL, " ");
		}
		parsed_command[number_of_arguments] = NULL;	
		execute_command(input_command, parsed_command, pipe_exists,ampersand_exists);	
	}
	
	else if(execution_flow == 1){// no semi colon found, ampersands found, split by ampersand
		// code block to separate command by ampersand and execute it
		ampersand_exists = 1;
		char* found = strtok(input_command, "&");	
		int number_of_commands = 0;
		while(found != NULL){
			parsed_command[number_of_commands++] = found;
			found = strtok(NULL, "&");
		}
		parsed_command[number_of_commands] = NULL;
		execute_command(input_command, parsed_command, pipe_exists, ampersand_exists);
		ampersand_exists = 0;
	}
	else if(execution_flow == 2){// semi colons found, no ampersands found, split by semi colon
		// code block to separate command by semi colon, and call the same function again but with a different execution flow
		ampersand_exists = 0;
		char* found = strtok(input_command, ";");	
		int number_of_commands = 0;
		while(found != NULL){
			parsed_command[number_of_commands++] = found;
			found = strtok(NULL, ";");
		}
		parsed_command[number_of_commands] = NULL;	
		// send semi-colon separated commands to the same function again to be separated by whitespace
		for(int k = 0; k < number_of_commands; k++){
			if(parsed_command[k] == NULL){
				wait(NULL);
				exit(1);
			}
			parse_command(parsed_command[k], 0);
		}
	}
	else if(execution_flow == 3){// semi colons found, ampersands found, split by semi colon first
		// code block to separate command by semi colon, and call the same function again but with a different execution flowampersand_exists = 1;
		char* index;
		char* found = strtok(input_command, ";");
		int number_of_commands = 0;
		while(found != NULL){
			strcpy(parsed_command[number_of_commands++] ,found);
			found = strtok(NULL, ";");
		}
		for(int k = 0; k < number_of_commands; k++){
			// send semi-colon separated commands to the same function again to be separated by ampersand
			if(*index = strchr(parsed_command[k], '&') != NULL)
				parse_command(parsed_command[k], 1);
			// send semi-colon separated commands to the same function again to be separated by whitespace
			else
				parse_command(parsed_command[k], 0);
		}
	}
	else{// pipe found
		int i = 0;
		char* found;
		pipe_exists = 1;
		char** piped_command = (char**)malloc(MAX_ARGUMENTS * sizeof(char*));
		for (int j = 0; j < MAX_ARGUMENTS; j++)
			piped_command[j] = (char*)malloc(MAX_ARGUMENT_CHARS * sizeof(char*));
		// code block to separate command by pipes and execute it
	    found = strtok(input_command,"|"); 
        while(found != NULL){
            strcpy(piped_command[i++] ,found);
			found = strtok(NULL, "|");
        }
        piped_command[i] = NULL; //end the array with a NULL character
		execute_command(input_command, piped_command, pipe_exists, ampersand_exists);
	}
}

void decide_execution_flow(char* input_command, int pipe_flag, int background_flag, int semi_colon_flag){ //decide the execution flow and forward command for parsing

	char** parsed_command;
	if(pipe_flag == 1)// pipe is found
		parse_command(input_command, 4);

	else{// pipe not found
		if(semi_colon_flag == 0 && background_flag == 0) // no semi colon or background processes found
			parse_command(input_command, 0);

		else if(semi_colon_flag == 0 && background_flag == 1) // no semi colon found but there is at least one background process
			parse_command(input_command, 1);
		
		else if(semi_colon_flag == 1 && background_flag == 0) // at least one semi colon found and no background processes found
			parse_command(input_command, 2);
		
		else// at least one semi colon and background process found
			parse_command(input_command, 3);
	}	
}
	
int main(){
 	while(1){
		int pipe_flag = 0;
		int background_flag = 0;
		int semi_colon_flag = 0;
		char input_command[MAX_INPUT_CHARS];
		print_user_info();
		fgets(input_command, MAX_INPUT_CHARS, stdin);
		input_command[strlen(input_command) - 1] = 0; //remove trailing newline character
		if(strcmp(input_command, "exit") == 0) //if block for exit command
			exit(0);
		if(strcmp(input_command, "reset") == 0)
			continue;
		// code block to set various flags
		pipe_flag = parse_for_char(input_command, '|');
		background_flag = parse_for_char(input_command, '&');
		semi_colon_flag = parse_for_char(input_command, ';');
		decide_execution_flow(input_command, pipe_flag, background_flag, semi_colon_flag);
	}
	return 0;
}