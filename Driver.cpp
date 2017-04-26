#include <sstream>
#include "Utils.h"

struct BLOCK
{
    unsigned index;
    char valid;
    unsigned tag;
};

//Global Variables
std::string outfile;

//Forward Declarations
bool DirectCache(std::ifstream &infile);
std::string GetHexString(unsigned long hex);

int main(int argc, char** argv)
{
    if(argc < 3 || argc > 3)
    {
        std::cout << "Usage: " << argv[0] << " [input] [output]" << std::endl;
        return 0;
    }

    outfile = argv[2];
    
    std::ifstream infile(argv[1]);
    if(infile)
    {
        LogPrint(DEBUG, "Testing");
        if(!DirectCache(infile)) LogPrint(ERROR, "Failed Direct Cache");
    }
}

bool DirectCache(std::ifstream &infile)
{
    int* sizes = [32, 128, 512, 1024];

    //Loop 4 times for each cache size
    for(int i = 0; i < 4; i++)
    {
        //Allocate memory for cache
        BLOCK* cache = malloc(sizeof(BLOCK) * sizes[i]);

        //Variables for reading
        std::string instr;
        unsigned long addr;
        //Reading loop
        while(infile >> instr >> std::hex >> addr)
        {
            LogPrint(INFO, instr + " - " + GetHexString(addr));
        }
    }
    
    return true;
}

std::string GetHexString(unsigned long addr)
{
        std::stringstream ss;
        ss << std::hex << addr;
        return ss.str();
}
