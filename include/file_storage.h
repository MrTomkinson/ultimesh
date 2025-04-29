#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <Arduino.h>

bool initFileSystem();
void listFiles();
String readFile(const char* path);

#endif
