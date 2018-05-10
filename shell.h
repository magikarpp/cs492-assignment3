

#ifndef FILESYSTEM_SHELL_H
#define FILESYSTEM_SHELL_H

#define SHELL_MAX_INPUT_LEN 2048
#define SHELL_MAX_ARGS 3
#define STDIN_FD 0

int exec_cmd(int argc, char** argv);

void enter_shell();

void exit_shell();

#endif //FILESYSTEM_SHELL_H
