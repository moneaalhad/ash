#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ASH_RL_BUFFERSIZE 1024
#define ASH_TOKEN_BUFFERSIZE 64
#define ASH_TOKEN_DELIMITER " \t\r\n\a"

void ash_loop(void);
char **ash_split_line(char *line);
int ash_launch(char **args);
int ash_cd(char **args);
int ash_help(char **args);
int ash_exit(char **args);
int ash_num_builtins();
int ash_execute(char **args);
char *ash_read_line(void);


void ash_loop(void)
{
    char *line;
    char **args;
    int status;
    char *host = getlogin();

    do 
    {
        if(host)
        {
            printf("%s@$: ", host);
        }
        
        line = ash_read_line();
        args = ash_split_line(line);
        status = ash_execute(args);


        free(line);
        free(args);
    } while(status);
}

char **ash_split_line(char *line)
{
    int buffersize = ASH_TOKEN_BUFFERSIZE, position = 0;
    char **tokens = malloc(buffersize * sizeof(char*));
    char *token;

    if(!tokens)
    {
        fprintf(stderr, "ash$: memory allocation error. try again with fewer lines/chars.\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, ASH_TOKEN_DELIMITER);
    while (token!=NULL)
    {
        tokens[position] = token;
        position++;

        if(position>=buffersize)
        {
            buffersize += ASH_TOKEN_BUFFERSIZE;
            tokens = realloc(tokens, buffersize * sizeof(char*));

            if(!tokens)
            {
                fprintf(stderr, "ash$: memory allocation error. try again with fewer lines/chars.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ASH_TOKEN_DELIMITER);
    }

    tokens[position]= NULL;
    return tokens;
}

int ash_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid==0)
    {
        if(execvp(args[0], args) == -1)
        {
            perror("ash: PID error\n");
        }
    } else if(pid<0)
    {
        // Forking error, PID can never be less than 0
        perror("ash: PID error\n");
    } else {
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
            
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

char *builtin_str[] = 
{
    "cd", "help", "exit"
};

int (*builtin_func[]) (char **) = {
    &ash_cd, &ash_help, &ash_exit
};

int ash_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int ash_cd(char **args)
{
    if(args[1]==NULL)
    {
        fprintf(stderr, "ash$: expected argument to cd.\n");
    } else {
        if(chdir(args[1]!=0)){
            perror("ash: ERR.\n");
        }
    }
    return 1;
}

int ash_help(char **args)
{
    int i;
    printf("aalhad's shell v1.\ntype program names and hit enter.\nthe following commands are built in:");
    for(i=0; i<ash_num_builtins(); i++)
        printf("   %s\n", builtin_str[i]);

    printf("use man() for information on other programs.\n");

    return 1;
}

int ash_exit(char **args)
{
    return 0;
}

int ash_execute(char **args)
{
    int i;

    if(args[0] == NULL)
    {
        // Empty command 
        return 1;
    }
    for(i=0; i<ash_num_builtins(); i++)
    {
        if(strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return ash_launch(args);
}

char *ash_read_line(void)
{
  int bufsize = ASH_RL_BUFFERSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "ash$: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += ASH_RL_BUFFERSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

int main(int argc, char **argv)
{
    // Load Config Files

    // Run Shell Loop
    ash_loop();


    //

    return EXIT_SUCCESS;
}