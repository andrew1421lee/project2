#include <sstream>
#include "Utils.h"

struct BLOCK
{
    //unsigned index; //Don't need another index, just use array index
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
    //Invalid input check
    if(argc < 3 || argc > 3)
    {
        std::cout << "Usage: " << argv[0] << " [input] [output]" << std::endl;
        return 0;
    }

    //Load file from args
    outfile = argv[2];
    std::ifstream infile(argv[1]);
    //If file exists and opened
    if(infile)
    {
        //Run direct cache on data
        if(!DirectCache(infile)) LogPrint(ERROR, "Failed Direct Cache");
    }
}

bool DirectCache(std::ifstream &infile)
{
    int sizes[4] = {32, 128, 512, 1024};
    int shifts[4] = {5, 7, 9, 10};

    //Loop 4 times for each cache size
    for(int i = 0; i < 4; i++)
    {
        //Allocate memory for cache
        BLOCK* cache = (BLOCK*)malloc(sizeof(BLOCK) * sizes[i]);
        //Initialize cache 
        for(int j = 0; j < sizes[i]; j++)
        {
            cache[j].valid = 0;
            cache[j].tag = 0;
        }

        //Variables for reading
        std::string instr;
        unsigned long addr;
        std::string derp;
        //Reading loop
        while(infile >> instr >> std::hex >> addr)
        {
            //Index is found by Block Address modulo Cache size
            unsigned cacheindex = addr % sizes[i];
            //Tag is the rest of the bits
            unsigned cachetag = addr >> shifts[i];
            LogPrint(INFO, instr + " - " + GetHexString(addr) + " - " + GetHexString(cacheindex) + " - " + GetHexString(cachetag));
            std::cin >> derp;
        }

        //Unallocate memory
        free(cache);
    }
    
    return true;
}

std::string GetHexString(unsigned long addr)
{
        std::stringstream ss;
        ss << std::hex << addr;
        return ss.str();
}
