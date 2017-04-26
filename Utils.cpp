/*
 *  Anchu's C++ Utilities
 *  To make my life easier
 */
#include "Utils.h"

//DEBUG LEVEL
char DebugMode = 0;

void LogPrint(LOG level, std::string msg)
{
    if(level < DebugMode)
    {
        return;
    }
    std::string serv;
    switch(level)
    {
        case ERROR: serv = "ERROR";
                    break;
        case INFO: serv = "INFO";
                   break;
        case DEBUG: serv = "DEBUG";
                    break;
    }
    std::cout << serv << ": " << msg << std::endl;
}

bool WriteFile(WRITEMODE mode, std::string filename, std::string text)
{
    std::ofstream outfile;
    if(mode == 0)
    {
        outfile.open(filename, std::ios_base::app);
    }
    else
    {
        outfile.open(filename, std::ios_base::trunc);
    }
    if(outfile)
    {
        LogPrint(DEBUG, "writeFile - Writing: " + text);
        outfile << text;
        outfile.close();
        return true;
    }
    else
    {
        LogPrint(ERROR, "writeFile - ERROR writing file");
        return false;
    }
}

