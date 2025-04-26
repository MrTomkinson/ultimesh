#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

void listFiles();
void showFreeSpace();
bool initFileSystem();
bool writeFile(const char* path, const char* content);
String readFile(const char* path);

#endif
