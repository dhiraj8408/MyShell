#include <stdio.h>
#include <string.h>
#include <stdlib.h> // exit(), malloc(), free()
#include <unistd.h> // getcwd()
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h> // wait()

#define MAX_LEN 60
#define MAX_INPUT 60
#define MAX_COMMANDS 20
int typeOfCommands[5] = {0}; // used to know what type of command is provided as input
int noOfCommands = 0;        // no.of commands the particular input contains
int isCommandExit = 0;       // is the command exit
typedef struct commandData
{
    char parsedCommands[MAX_INPUT][MAX_LEN]; // to store all commands alongside arguments
    int isCommandCD;                         // flag to know whether command is CD
    int isCommandExit;                       // flag to know whether command is exit
    int isRedirection;                       // flag to know whether command is >
    int noOfArguments;                       // to know and have a limit of arguments
} commandData;                               // meta data related to commands

commandData commands[MAX_COMMANDS]; // array to store commands
char *previousDirectory = NULL;

void initialize(commandData *command) // to initialize the array of strcutures with default values
{
    for (int i = 0; i < MAX_INPUT; i++)
    {
        command->parsedCommands[i][0] = '\0';
    }
    command->isCommandCD = 0;
    command->isCommandExit = 0;
    command->isRedirection = 0;
    command->noOfArguments = 0;
}

void parseInput(char *inputCommand)
{
    // initializing all global variables to default values
    for (int i = 0; i < MAX_COMMANDS; i++)
    {
        initialize(&commands[i]);
    }
    for (int i = 0; i < 5; i++)
    {
        typeOfCommands[i] = 0;
    }
    noOfCommands = 0;

    // Determine command type by searching whether a particualr type is present
    if (strstr(inputCommand, "##") != NULL)
        typeOfCommands[1] = 1;
    if (strstr(inputCommand, "&&") != NULL)
        typeOfCommands[2] = 1;
    if (strstr(inputCommand, ">") != NULL)
        typeOfCommands[3] = 1;
    if (strstr(inputCommand, "|") != NULL)
        typeOfCommands[4] = 1;

    char initialCommand[MAX_LEN];
    strcpy(initialCommand, inputCommand);

    if (typeOfCommands[3])
    { // if there is redirection then ensuring there are spaces in between
        int idx = 0;
        int copyIdx = 0;
        while (inputCommand[idx] != '\0')
        {
            if (inputCommand[idx] == '>')
            {
                if (idx > 0)
                    if (inputCommand[idx - 1] != ' ')
                        initialCommand[copyIdx++] = ' ';

                initialCommand[copyIdx++] = '>';
                if (inputCommand[idx + 1] != ' ')
                    initialCommand[copyIdx++] = ' ';
            }
            else
            {
                initialCommand[copyIdx++] = inputCommand[idx];
            }
            idx++;
        }
        initialCommand[copyIdx] = '\0';
    }

    // determining if command is not of above types then giving default values
    int status = 0;
    for (int i = 1; i < 5; i++)
    {
        status = (status || typeOfCommands[i]);
    }
    if (!status)
        typeOfCommands[0] = 1;

    int commandCount = 0; // to keep a track of currentcommand no that is being parsed
    int currIdx = 0;

    char *ip = initialCommand;
    char *currCommand = NULL;
    if (typeOfCommands[1] || typeOfCommands[2] || typeOfCommands[4]) // separating based on "##" & "&&" && "|"
    {
        while ((currCommand = strsep(&ip, "##&&|")) != NULL)
        {
            if (currCommand[0] != '\0' && currCommand[0] != '\n')
            {
                currIdx = 0;
                char *executableCommand;
                while ((executableCommand = strsep(&currCommand, " ")) != NULL)
                {
                    if (executableCommand[0] != '\0' && executableCommand[0] != '\n')
                    {
                        if (strcmp(executableCommand, "cd") == 0)
                            commands[commandCount].isCommandCD = 1;
                        strcpy(commands[commandCount].parsedCommands[currIdx], executableCommand);
                        currIdx++;
                    }
                }
                if (!commands[commandCount].noOfArguments)
                    commands[commandCount].noOfArguments = currIdx;
                commandCount++;
            }
        }
        noOfCommands = commandCount;
    }
    else // seperating based on " "
    {
        currIdx = 0;
        while ((currCommand = strsep(&ip, " ")) != NULL)
        {
            if (*currCommand != '\0')
            {
                if (strcmp(currCommand, "cd") == 0)
                    commands[0].isCommandCD = 1;
                if (strcmp(currCommand, "exit") == 0)
                    commands[0].isCommandExit = 1;
                if (strcmp(currCommand, ">") == 0)
                {
                    commands[0].noOfArguments = currIdx;
                    commands[0].isRedirection = 1;
                }
                else
                {
                    strcpy(commands[0].parsedCommands[currIdx], currCommand);
                    currIdx++;
                }
            }
        }
        if (!commands[0].noOfArguments)
            commands[0].noOfArguments = currIdx;
        noOfCommands = 1;
    }
}

void executeCommand()
{
    if (commands[0].isCommandCD) // if command is cd then directly executing cd without forking
    {
        char currWorkingDirectory[MAX_LEN];
        getcwd(currWorkingDirectory, sizeof(currWorkingDirectory));

        if (strcmp(commands[0].parsedCommands[1], "-") == 0)
        {
            if (previousDirectory == NULL)
            {
                printf("Shell: Incorrect command\n");
            }
            else
            {
                char tempDirectory[MAX_LEN];
                strcpy(tempDirectory, previousDirectory);
                previousDirectory = strdup(currWorkingDirectory);
                if (chdir(tempDirectory) != 0)
                {
                    printf("Shell: Incorrect command\n");
                }
            }
        }
        else
        {
            if (chdir(commands[0].parsedCommands[1]) != 0)
                printf("Shell: Incorrect command\n");
            else
                previousDirectory = strdup(currWorkingDirectory);
        }
    }
    else // forking if command is not cd and then executing the inputcommand in the child process
    {
        int processStatusCode = fork();
        if (processStatusCode == 0)
        {

            int noOfArgsPassed = commands[0].noOfArguments;
            char *commandArgsToExecute[noOfArgsPassed + 1];
            for (int i = 0; i < noOfArgsPassed; i++)
            {
                commandArgsToExecute[i] = strdup(commands[0].parsedCommands[i]);
            }
            commandArgsToExecute[noOfArgsPassed] = NULL;

            if (execvp(commandArgsToExecute[0], commandArgsToExecute) < 0)
            {
                printf("Shell: Incorrect command\n");
            }
        }
        else if (processStatusCode > 0) // waiting for the child to terminate since sequential
        {
            wait(NULL);
        }
        else if (processStatusCode < 0)
        {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    }
}

void executeParallelCommands()
{
    int commandsExecuted = 0;
    for (int i = 0; i < noOfCommands; i++)
    {
        if (commands[i].isCommandCD)
        {
            if (chdir(commands[i].parsedCommands[1]) != 0)
            {
                printf("Shell: Incorrect command\n");
            }
        }
        else
        {
            commandsExecuted++;
            int processStatusCode = fork();
            if (processStatusCode == 0)
            {
                int noOfArgsPassed = commands[i].noOfArguments;
                char *commandArgsToExecute[noOfArgsPassed + 1];
                for (int idx = 0; idx < noOfArgsPassed; idx++)
                {
                    commandArgsToExecute[idx] = strdup(commands[i].parsedCommands[idx]);
                }
                commandArgsToExecute[noOfArgsPassed] = NULL;

                if (execvp(commandArgsToExecute[0], commandArgsToExecute) == -1)
                {
                    printf("Shell: Incorrect command\n");
                }
            }
            else if (processStatusCode > 0)
            {
                // do nothing as commands are to be executed parallely
            }
            else if (processStatusCode < 0)
            {
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        }
    }
    for (int i = 0; i < commandsExecuted; i++)
    {
        wait(NULL);
    }
}

void executeSequentialCommands()
{
    for (int i = 0; i < noOfCommands; i++)
    {
        if (commands[i].isCommandCD)
        {
            if (chdir(commands[i].parsedCommands[1]) != 0)
            {
                printf("Shell: Incorrect command\n");
            }
        }
        else
        {
            int processStatusCode = fork();
            if (processStatusCode == 0)
            {
                int noOfArgsPassed = commands[i].noOfArguments;
                char *commandArgsToExecute[noOfArgsPassed + 1];
                for (int idx = 0; idx < noOfArgsPassed; idx++)
                {
                    commandArgsToExecute[idx] = strdup(commands[i].parsedCommands[idx]);
                }
                commandArgsToExecute[noOfArgsPassed] = NULL;

                if (execvp(commandArgsToExecute[0], commandArgsToExecute) == -1)
                {
                    printf("Shell: Incorrect command\n");
                }
            }
            else if (processStatusCode > 0) // waiting for the child to terminate since sequential execution
            {
                wait(NULL);
            }
            else if (processStatusCode < 0)
            {
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        }
    }
}

void executePipelineCommands()
{
    int noOfPipes = noOfCommands - 1;
    int pipeArrays[2 * noOfPipes];

    // Creating pipes.
    for (int i = 0; i < noOfPipes; i++)
    {
        if (pipe(pipeArrays + i * 2) < 0)
        {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    }

    for (int idx = 0; idx < noOfCommands; idx++)
    {
        int processStatusCode = fork();

        if (processStatusCode == 0)
        {
            // If not the first command, getting input from the previous pipe.
            if (idx > 0)
            {
                if (dup2(pipeArrays[(idx - 1) * 2], STDIN_FILENO) < 0)
                {
                    printf("Shell: Incorrect command\n");
                    exit(1);
                }
            }

            // If not the last command, output to the next pipe.
            if (idx < noOfPipes)
            {
                if (dup2(pipeArrays[idx * 2 + 1], STDOUT_FILENO) < 0)
                {
                    printf("Shell: Incorrect command\n");
                    exit(1);
                }
            }

            // Close all pipes in the child process.
            for (int i = 0; i < 2 * noOfPipes; i++)
            {
                close(pipeArrays[i]);
            }

            // Executing the command.
            int noOfArgsPassed = commands[idx].noOfArguments;
            char *commandArgsToExecute[noOfArgsPassed + 1];
            for (int i = 0; i < noOfArgsPassed; i++)
            {
                commandArgsToExecute[i] = commands[idx].parsedCommands[i];
            }
            commandArgsToExecute[noOfArgsPassed] = NULL;

            if (execvp(commandArgsToExecute[0], commandArgsToExecute) < 0)
            {
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        }
        else if (processStatusCode < 0)
        {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    }

    // Parent process must close all pipes.
    for (int i = 0; i < 2 * noOfPipes; i++)
    {
        close(pipeArrays[i]);
    }

    // Parent must wait for all child processes.
    for (int i = 0; i < noOfCommands; i++)
    {
        wait(NULL);
    }
}

void executeCommandRedirection(int *statusCode)
{
    int processStatusCode = fork();
    if (processStatusCode == 0)
    {
        int noOfArgsPassed = commands[0].noOfArguments;
        close(STDOUT_FILENO);                                                                    // closing STDOUT_FILENO to redirect standrd output
        open(commands[0].parsedCommands[noOfArgsPassed], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // opening new file in place of STDOUT_FILENO to redirect output
        char *commandArgsToExecute[noOfArgsPassed + 1];
        for (int i = 0; i < noOfArgsPassed; i++)
        {
            commandArgsToExecute[i] = strdup(commands[0].parsedCommands[i]);
        }
        commandArgsToExecute[noOfArgsPassed] = NULL;

        execvp(commandArgsToExecute[0], commandArgsToExecute);
    }
    else if (processStatusCode > 0)
    {
        wait(NULL);
    }
    else
    {
        printf("Shell: Incorrect command\n");
        exit(1);
    }
}
void ctrlC_Handler(int signalCode)
{
    // Function Does Nothing just returns Back
}
void ctrlZ_Handler(int signalCode)
{
    // Function Does Nothing just returns Back
}
int main()
{
    while (1)
    {
        signal(SIGINT, ctrlC_Handler);
        signal(SIGTSTP, ctrlZ_Handler);
        char currWorkingDirectory[MAX_LEN];
        getcwd(currWorkingDirectory, sizeof(currWorkingDirectory));
        printf("%s$", currWorkingDirectory);

        char *inputCommand = NULL;
        size_t len = 0;

        getline(&inputCommand, &len, stdin);
        inputCommand[strlen(inputCommand) - 1] = '\0';
        int statusCode = 0;
        parseInput(inputCommand);
        if (noOfCommands == 1 && commands[0].isCommandExit)
        {
            printf("Exiting shell...\n");
            break;
        }

        if (typeOfCommands[3])
        {
            executeCommandRedirection(&statusCode);
            if (statusCode == -1)
            {
                printf("Shell: Incorrect command\n");
            }
        }
        else if (typeOfCommands[2])
        {
            executeParallelCommands();
        }
        else if (typeOfCommands[1])
        {
            executeSequentialCommands();
        }
        else if (typeOfCommands[4])
        {
            executePipelineCommands();
        }
        else
        {
            executeCommand();
        }
    }

    return 0;
}