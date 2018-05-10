#ifndef FILESYSTEM_LFILE_H
#define FILESYSTEM_LFILE_H

#include "datatypes.h"

Lfile makeLfile();

void freeLfile(Lfile file);

LFnode makeLFnode(size_t start);

void freeLFnode(LFnode node);

void lfileAddBlock(Lfile file, LDisk disk, size_t physicalAddress);

int lfileAddBytes(Lfile file, LDisk disk, size_t bytes);

void lfileRemoveBytes(Lfile file, LDisk disk, size_t bytes);

void lfilePrint(Lfile file);

#endif // FILESYSTEM_LFILE_H