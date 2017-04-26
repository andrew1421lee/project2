#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <fstream>

enum LOG
{
    DEBUG, INFO, ERROR
};

enum WRITEMODE
{
    APPEND, TRUNCATE
};

void LogPrint(LOG level, std::string msg);
bool WriteFile(WRITEMODE mode, std::string filename, std::string text);

#endif /*UTILS_H*/
