

#include "shell.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "termcolors.h"
#include "DTree.h"
#include "LDisk.h"

int SHELL_RUNNING = 1;

int cmdCd(int argc, char **argv) {
  if (argc < 2) {
    printf("cd: requires a [directory]\n");
    return 1;
  }

  if (strcmp(argv[1], "..") == 0) {
    // Go to the parent!
    if (PWD->parent != NULL) {
      // Do nothing if we're at the root
      PWD = PWD->parent;
    }
  } else {
    // Set the PWD to the name if found
    DTNode dir = dtreeGetChildByName(PWD, argv[1]);
    if (dir == NULL) {
      // Not found
      printf("cd: string not in pwd: %s\n", argv[1]);
      return 1;
    }

    if (dir->attrs != NULL) {
      // It's a file
      printf("cd: not a directory: %s\n", argv[1]);
      return 1;
    }

    // Otherwise update dir
    PWD = dir;
  }

  return 0;
}

int cmdMkdir(int argc, char **argv) {
  if (argc < 2) {
    printf("mkdir: requires a [name]\n");
    return 1;
  }

  // Check if already exists
  if (dtreeGetChildByName(PWD, argv[1]) != NULL) {
    printf("mkdir: cannot create directory %s: File exists\n", argv[1]);
    return 1;
  }

  DTNode newDir = makeDTNode(argv[1], NULL, NULL);
  dtreeAdd(PWD, newDir);
  return 0;
}

int cmdDelete(int argc, char **argv) {
  if (argc < 2) {
    printf("delete: requires a [name]\n");
    return 1;
  }

  DTNode dir = dtreeGetChildByName(PWD, argv[1]);
  if (dir == NULL) {
    // Not found
    printf("delete: failed to delete '%s': No such file or directory\n", argv[1]);
    return 1;
  }

  if (dtreeIsFile(dir)) {
    // Update the parent's (PWD) timestamp
    dtreeUpdateTimestamp(PWD);
  } else if (!dtreeIsEmpty(dir)) {
    printf("delete: failed to delete '%s': directory not empty\n", argv[1]);
    return 1;
  }

  // All ok
  dtreeRecursiveRemove(dir);

  return 0;
}

/**
 * Print out entire directory tree
 * @param argc
 * @param argv
 * @return
 */
int cmdDir(int argc, char **argv) {
  dtreePrintBF(ABS_DIR_ROOT);
  printf("\n");
  return 0;
}

void cmdLsPrintHelper(DTNode node) {
  printf("%s ", node->name);
}

/**
 * List all files and subdirectories in current directory
 * @param argc
 * @param argv
 * @return
 */
int cmdLs(int argc, char **argv) {
  llMap(PWD->children, cmdLsPrintHelper);
  printf("\n");
  return 0;
}

void cmdPrfilesHelper(DTNode node) {
  // If it's a file -> print the attributes
  if (node->attrs != NULL) {
    printf(C_YEL "%s " C_NRM, node->absPath);
    dtreePrintFile(node);
    printf("\n");
  }
}

/**
 * print out all file information
 * @param argc
 * @param argv
 * @return
 */
int cmdPrfiles(int argc, char **argv) {
  dtreeTraverseApplyBF(ABS_DIR_ROOT, cmdPrfilesHelper);
  return 0;
}

/**
 * prdisk - print out disk space information
 * @param argc
 * @param argv
 * @return
 */
int cmdPrdisk(int argc, char **argv) {
//  printf("Size of free blocks: %li\n", ldiskSizeOfFreeBlocks(DISK));
  ldiskPrintFootprint(DISK);
  return 0;
}

/**
 * create [name] - create a new file in the current directory
 * @param argc
 * @param argv
 * @return
 */
int cmdCreate(int argc, char **argv) {
  if (argc < 2) {
    printf("create: requires a [name]\n");
    return 1;
  }

  char *filename = argv[1];

  // Check if already exists
  if (dtreeGetChildByName(PWD, filename) != NULL) {
    printf("create: cannot create directory %s: File exists\n", argv[1]);
    return 1;
  }

  DTFileAttrs newFileAttrs = makeDTFileAttrs((size_t) 0, NULL);
  DTNode newFileNode = makeDTNode(filename, NULL, newFileAttrs);

  dtreeAdd(PWD, newFileNode);
  // Update current dirs timestamp
  dtreeUpdateTimestamp(PWD);
  return 0;
}

/**
 * append [name] [bytes] - append a number of bytes to the file
 * @param argc
 * @param argv
 * @return
 */
int cmdAppend(int argc, char **argv) {
  if (argc < 3) {
    printf("append: requires [name] [bytes]\n");
    return 1;
  }

  DTNode fileNode = dtreeGetChildByName(PWD, argv[1]);
  if (fileNode == NULL) {
    // Not found
    printf("append: failed to append to '%s': No such file\n", argv[1]);
    return 1;
  } else if (!dtreeIsFile(fileNode)) {
    printf("append: failed to append to '%s': Can't append to a directory\n", argv[1]);
    return 1;
  }

  size_t bytes = (size_t) atol(argv[2]);

  if (bytes <= 0) {
    printf("append: failed to append '%s' bytes to '%s: Must be a number more than 0\n", argv[2], argv[1]);
    return 1;
  }

  if (dtreeAppendToFile(fileNode, DISK, bytes) == INSUFFICIENT_SPACE) {
    printf("append: failed to append %li bytes to '%s': Out of disk space.\n", bytes, argv[1]);
    return 1;
  }

  return 0;
}

/**
 * remove [name] [bytes] - delete a number of bytes from the file
 * @param argc
 * @param argv
 * @return
 */
int cmdRemove(int argc, char **argv) {
  if (argc < 3) {
    printf("remove: requires [name] [bytes]\n");
    return 1;
  }

  DTNode fileNode = dtreeGetChildByName(PWD, argv[1]);
  if (fileNode == NULL) {
    // Not found
    printf("remove: failed to remove from '%s': No such file\n", argv[1]);
    return 1;
  } else if (!dtreeIsFile(fileNode)) {
    printf("remove: failed to remove from '%s': Can't remove bytes from a directory\n", argv[1]);
    return 1;
  }

  size_t bytes = (size_t) atol(argv[2]);

  if (bytes <= 0) {
    printf("remove: failed to remove '%s' bytes from '%s: Must be a number more than 0\n", argv[2], argv[1]);
    return 1;
  }

  dtreeRemoveFromFile(fileNode, DISK, bytes);
  return 0;
}

/**
 *
 * @param argc
 * @param argv
 */
int exec_cmd(int argc, char **argv) {

  int retCode;

  if (argc == 0){
    retCode = 0;
  }

  else if (strcmp(argv[0], "exit") == 0) {
    exit_shell();
    retCode = 0;
  } else if (strcmp(argv[0], "cd") == 0) {
    retCode = cmdCd(argc, argv);
  }
  else if (strcmp(argv[0], "mkdir") == 0) {
    retCode = cmdMkdir(argc, argv);
  }

  else if (strcmp(argv[0], "delete") == 0){
    retCode = cmdDelete(argc, argv);
  }

  else if (strcmp(argv[0], "ls") == 0){
    retCode = cmdLs(argc, argv);
  }

  else if (strcmp(argv[0], "create") == 0) {
    retCode = cmdCreate(argc, argv);
  }

  else if (strcmp(argv[0], "append") == 0){
    retCode = cmdAppend(argc, argv);
  }

  else if (strcmp(argv[0], "remove") == 0){
    retCode = cmdRemove(argc, argv);
  }

  else if (strcmp(argv[0], "dir") == 0) {
    retCode = cmdDir(argc, argv);
  }

  else if (strcmp(argv[0], "prfiles") == 0){
    retCode = cmdPrfiles(argc, argv);
  }

  else if (strcmp(argv[0], "prdisk") == 0){
    retCode = cmdPrdisk(argc, argv);
  }

  else {
    printf("filesystem: command not found: %s\n", argv[0]);
    retCode = 1;
  }

  return retCode;
}

char **parse_args(char *argvString) {
  int argc = 0;
  char **argv = (char **) malloc(sizeof(char *) * (SHELL_MAX_ARGS + 1)); // +1 for null terminator

  char *arg = strtok(argvString, " ");

  while (arg && argc < SHELL_MAX_ARGS) {

    argv[argc] = malloc(sizeof(char) * (strlen(arg) + 1));
    strcpy(argv[argc], arg);
    argc += 1;
    arg = strtok(0, " ");

  }

  // End list with NULL
  argv[argc] = NULL;
  return argv;

};

/*
 * argc calculator
 */
int count_args(char **argv) {

  int count = 0;

  while (argv != NULL && argv[count] != NULL)
    count++;

  return count;

}

char **read_input() {

  char inputBuf[SHELL_MAX_INPUT_LEN];
  ssize_t numRead = read(STDIN_FD, inputBuf, SHELL_MAX_INPUT_LEN - 1);

  if (numRead < 0)
    perror("Error reading input!"), exit(1);
  else if (numRead == 1)
    inputBuf[0] = '\0';

  if (inputBuf[numRead - 1] == '\n')
    inputBuf[numRead - 1] = '\0';

  inputBuf[numRead] = '\0';

  return parse_args(inputBuf);

}

void print_prompt() {

  printf("filesystem:%s", PWD->name);
  if (errno)
    printf(C_RED "> " C_NRM);
  else
    printf(C_GRN "> " C_NRM);

  fflush(stdout);

}

void exit_shell() {

  printf("Bye bitch.\n");
  SHELL_RUNNING = 0;

}


void enter_shell() {
  // Only run while
  while (SHELL_RUNNING) {
    print_prompt();
    char **argv = read_input();
    int argc = count_args(argv);
    errno = exec_cmd(argc, argv);
    fflush(stdout);
    // Free allocated memory
    for (int i = 0; i < argc; i++) {
      free(argv[i]);
    }
    free(argv);
  }
}
