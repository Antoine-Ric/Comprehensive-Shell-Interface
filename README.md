[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/wtw9xmrw)
# Shell

## Description

This is a shell project developed by a group of students at Florida State University. The shell is designed to implement various features, including prompt, environment variable handling, tilde expansion, $PATH search, external command execution, I/O redirection, piping, background processing, internal command execution, and an external timeout executable.

## Group Members

- Benjamin Payne: bjp22c@fsu.edu
- Christopher Salas: cms17d@fsu.edu
- Ricardi Antoine: ra20a@fsu.edu

## Division of Labor

- Part 1: Prompt
  - Benjamin Payne
  - Ricardi Antoine

- Part 2: Environment Variables
  - Ricardi Antoine
  - Christopher Salas

- Part 3: Tilde Expansion
  - Christopher Salas
  - Benjamin Payne

- Part 4: $PATH Search
  - Benjamin Payne
  - Christopher Salas

- Part 5: External Command Execution
  - Ricardi Antoine
  - Benjamin Payne

- Part 6: I/O Redirection
  - Christopher Salas
  - Benjamin Payne

- Part 7: Piping
  - Benjamin Payne
  - Ricardi Antoine

- Part 8: Background Processing
  - Benjamin Payne
  - Christopher Salas

- Part 9: Internal Command Execution
  - Christopher Salas
  - Benjamin Payne

- Part 10: External Timeout Executable
  - Benjamin Payne
  - Christopher Salas

- Extra Credit
  - Benjamin Payne
  - Christopher Salas
  - Ricardi Antoine


## File Listing
```
shell/
│
├── src/
│ ├── mytimeout.c
│ └── shell.c
│
├── include/
│ └── shell.h
│
├── README.md
└── Makefile
```
## How to Compile & Execute

### Requirements
- **Compiler**: e.g., `gcc` for C/C++.
- **Dependencies**: No dependencies.

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Bugs
- **Background double pipe**: There was at least one time there was an error caused by background processing with double piping  it was probably a memory allocation problem for the command that is stored in BackPros.

## Extra Credit
- **Extra Credit 1**: Shell-ception

## Considerations
This shell is not nearly as robust at giving good error messages when users try vary interesting command combinations.  If a user happens to type in a command with 10,000,000 arguments and then follows it by a |, < or > the program will most likely not work because of the min variable.  We were also getting a couple of errors about implicitly difining functions setenv and kill which we did not understand but did not effect code execution.
