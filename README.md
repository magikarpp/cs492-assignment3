# HW3 Filesystem #

A file system

## Structure

`shell.c/.h`:  
Handles reading input, parsing args, and executing commands.  

TODO:  
* commands
	* append
	* remove
	* prdisk

`LinkedList.c/.h`:  
A generic linked list.  

`DTree.c/.h`:  
A directory tree that allows for infinite files/subdirs.

`main.c`:  
Handles argument parsing and initial setup.

TODO:  
* Ldisk and Lfile functionality

* `PWD`: Process working directory
* `ABS_DIR_ROOT`: The absolute root of the current file system.

## Building / Running
`make`:  Builds to `filesystem` executable.

`make clean`: Removes executable.
