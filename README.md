# MyShell

## Overview

**MyShell** is a custom command-line shell built from scratch using OS system calls in C, designed to execute basic Linux commands. This project was developed as part of the **Operating Systems Lab** assignment. It replicates core shell functionalities such as command execution, directory navigation, signal handling, and more.

## Features

1. **Command Execution**:  
   MyShell runs an infinite loop, accepting and executing user commands. It supports commands like `ls`, `pwd`, etc., and uses system calls like `fork()`, `exec()`, and `wait()` to manage process creation and execution.

2. **Directory Navigation** (`cd` command):  
   - `cd <directory>`: Change to the specified directory.
   - `cd ..`: Move to the parent directory.
   - `cd -` : Switches to the previous working directory.
   
3. **Command Error Handling**:  
   For invalid commands, MyShell returns the message:
   ```bash
   Shell: Incorrect command
   ```

4. **Signal Handling**:  
   MyShell properly handles signals generated from the keyboard (`Ctrl + C`, `Ctrl + Z`). The shell continues running unless explicitly stopped by the `exit` command.

5. **Multiple Command Execution**:  
   - Commands separated by `##` are executed sequentially.
   - Commands separated by `&&` are executed in parallel.

6. **Output Redirection**:  
   MyShell supports output redirection using the `>` symbol. Example:
   ```bash
   ls > output.txt
   ```

7. **Pipelines**:  
   MyShell supports command pipelines (e.g., `cat file.txt | grep "pattern" | wc`).

## Installation and Usage

### Compilation
To compile MyShell, run:
```bash
gcc -o myshell myshell.c
```

### Running MyShell
Once compiled, you can run the shell using:
```bash
./myshell
```

### Example Commands
- `ls`: List files in the current directory.
- `cd <directory>`: Change to the specified directory.
- `pwd`: Print the current working directory.
- `ls > file.txt`: Redirect `ls` output to a file.
- `command1 ## command2`: Execute commands sequentially.
- `command1 && command2`: Execute commands in parallel.
- `command1 | command2 | ... | commandn` : Execute pipeline Commands

### Exiting MyShell
To exit MyShell, use the command:
```bash
exit
```

## Requirements

- Linux-based system
- GCC compiler
