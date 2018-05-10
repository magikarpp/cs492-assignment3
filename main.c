#define _XOPEN_SOURCE
//#define  _DEFAULT_SOURCE

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <sys/time.h>

#include <limits.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include "shell.h"
#include "DTree.h"
#include "LDisk.h"

#define UNDEFINED 0

const char *argp_program_version = "1.0.0";
static char doc[] = "A simulated filesystem.";
static char args_doc[] = "";

static struct argp_option options[] = {
    {"disk-size", 's', "INT", 0, "REQUIRED: Disk size of the file system.", 0},
    {"block-size", 'b', "INT", 0, "REQUIRED: Block size of the file system.", 0},
    {"files-file", 'f', "FILE", 0, "REQUIRED: Input file storing information on files.", 1},
    {"dir-file", 'd', "FILE", 0, "REQUIRED: Input file storing information on directories.", 1},

    {0}
};

struct arguments {
  size_t diskSize;
  size_t blockSize;
  char *inputFilesPath;
  char *inputDirsPath;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {

  struct arguments *args = state->input;

  switch (key) {

    case 'f':
      // Input files
      args->inputFilesPath = arg;
      break;

    case 'd':args->inputDirsPath = arg;
      // Input directories
      break;

    case 's':
      // Disk Size
      args->diskSize = (size_t) atoi(arg);
      break;

    case 'b':
      // Block size
      args->blockSize = (size_t) atoi(arg);
      break;

    case 'h':argp_usage(state);
      break;

    case ARGP_KEY_END:
      // Check to make sure everything is filled
      if (args->blockSize == UNDEFINED)
        argp_failure(state, 1, 0, "Need to provide block size.");

      if (args->diskSize == UNDEFINED)
        argp_failure(state, 1, 0, "Need to provide disk size.");

      if (args->inputFilesPath == NULL)
        argp_failure(state, 1, 0, "Need to provide files file.");

      if (args->inputDirsPath == NULL)
        argp_failure(state, 1, 0, "Need to provide directory file.");

      break;

    default:return ARGP_ERR_UNKNOWN;

  }

  return 0;

};

static struct argp parser = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

/**
 * ex
 * @param path
 * @return
 */
char *getParentName(char *path) {

  char *name = (char *) malloc(sizeof(char) * strlen(path) + 1);
  char *startName = name;
  char *parentName = (char *) malloc(sizeof(char) * strlen(path) + 1);
  strcpy(name, path);
  name = strtok(path, PATH_SEPARATOR_PTR);
  char *temp;

  while ((temp = strtok(NULL, PATH_SEPARATOR_PTR)) != NULL) {

    strcpy(parentName, name);
    name = temp;

  }

  free(startName);

  char *toReturn = (char *) malloc(sizeof(char) * strlen(parentName) + 1);
  strcpy(toReturn, parentName);
  free(parentName);
  return toReturn;

}

/**
 *
 *  • -f [input files storing information on files]
    • -d [input files storing information on directories]
    • -s [disk size]
    • -b [block size]
 * @param argc
 * @param argv
 */

int main(int argc, char **argv) {
  struct arguments args;
  args.blockSize = UNDEFINED;
  args.diskSize = UNDEFINED;
  args.inputFilesPath = NULL;
  args.inputDirsPath = NULL;
  argp_parse(&parser, argc, argv, 0, 0, &args);

  // Initialize File System
  FILE *inputDirFile = fopen(args.inputDirsPath, "r");

  if (inputDirFile == NULL) {

    fprintf(stderr, "Can't open input dir file: %s \n", args.inputDirsPath);
    exit(1);

  }

  // Initialize the globals
  // Initialize disk
  DISK = makeLDisk(args.blockSize, args.diskSize);
  // Root is initially none until directory is loaded
  ABS_DIR_ROOT = NULL;

  DTNode prevDir = NULL;
  DTNode curRoot = NULL;
  char *dirnamePath = (char *) malloc(sizeof(char) * NAME_MAX);

  while (!feof(inputDirFile)) {

    fscanf(inputDirFile, "%s\n", dirnamePath);

    //get basename from dirname
    char *name = basename(dirnamePath);

    DTNode dir = makeDTNode(name, NULL, NULL);
    // First entry is the root
    if (ABS_DIR_ROOT == NULL) {

      dtreeInitRoot(dir);
      curRoot = dir;

    } else {

      char *parentName = getParentName(dirnamePath);
      // Check if dirname is subdir of preDirName

      if (strcmp(prevDir->name, parentName) == 0) {
        curRoot = prevDir;
      }

      else if (strcmp(curRoot->name, parentName) == 0) {

      }

      else{
        curRoot = curRoot->parent;


      dtreeAdd(curRoot, dir);

      free(parentName);
      }

    }

    prevDir = dir;

  }
  free(dirnamePath);
  fclose(inputDirFile);

  FILE *inputFilesFile = fopen(args.inputFilesPath, "r");

  if (inputFilesFile == NULL) {

    fprintf(stderr, "Can't open input files file: %s \n", args.inputFilesPath);
    exit(1);

  }

  size_t size = 0;
  char month[4] = {0};
  int day = 0;
  char dayStr[3] = {0};
  char hoursMin[6] = {0};
  char absPath[PATH_MAX] = {0};
  char timestampStr[32] = {0};
  struct tm time = {0};

  while (!feof(inputFilesFile)) {

    fscanf(inputFilesFile, "%*i %*i %*s %*i %*s %*s %li %s %i %s %s\n", &size, month, &day, hoursMin, absPath);

    strcpy(timestampStr, "");
    sprintf(dayStr, "%i", day);
    strcat(timestampStr, month);
    strcat(timestampStr, " ");
    strcat(timestampStr, dayStr);
    strcat(timestampStr, " ");
    strcat(timestampStr, hoursMin);

    if (strptime(timestampStr, "%b %d %H:%M", &time) == NULL) {

      printf("Malformed file timestamp: %s\n", timestampStr);
      exit(1);

    }

    DTFileAttrs attrs = makeDTFileAttrs(size, NULL);

    if (size > 0 && lfileAddBytes(attrs->file, DISK, size) == INSUFFICIENT_SPACE) {

      printf("Not enough disk space for: %s\n", absPath);
      exit(1);

    }

    char *filename = basename(absPath);
    DTNode fileNode = makeDTNode(filename, &time, attrs);

    DTNode parent = dtreeGetParentByAbsPath(ABS_DIR_ROOT, absPath);

    if (parent == NULL) {

      printf("No directory exists for file: %s\n", absPath);
      exit(1);
    }

    dtreeAdd(parent, fileNode);

  }

  fclose(inputFilesFile);

  // Set the initial PWD to be the root
  PWD = ABS_DIR_ROOT;

  enter_shell();

  // Cleanup
  dtreeRecursiveRemove(ABS_DIR_ROOT);
  freeLDisk(DISK);
  return 0;
}
