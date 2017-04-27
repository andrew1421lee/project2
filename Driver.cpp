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
std::string dummy;

//Forward Declarations
bool DirectCache(std::ifstream &infile);
std::string GetHexString(unsigned long hex);
void BeKindRewind(std::ifstream &infile);

int main(int argc, char** argv)
{
    //Invalid input check
    if(argc < 3 || argc > 3)
    {
        std::cout << "Usage: " << argv[0] << " [input] [output]" << std::endl;
        return 0;
    }

    //save output file
    outfile = argv[2];
    //Load input file
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
    //Numbers for cache sizes
    int sizes[4] = {32, 128, 512, 1024};
    int shifts[4] = {5, 7, 9, 10};

    //String being written to file
    std::stringstream returnstring;

    //Loop 4 times for each cache size
    for(int i = 0; i < 4; i++)
    {
        //Space between runs
        if(i != 0)
        {
            returnstring << " ";
        }
        //Important variables
        unsigned long accesses = 0;
        unsigned long hits = 0;

        //Allocate memory for cache
        BLOCK* cache = new BLOCK[sizes[i]];
        //Initialize cache 
        for(int j = 0; j < sizes[i]; j++)
        {
            cache[j].valid = 0;
            cache[j].tag = 0;
        }

        //Variables for reading
        std::string instr;
        unsigned long addr;

        //Reading loop
        while(infile >> instr >> std::hex >> addr)
        {
            //LOL offset
            addr >>= 5;
            //Index is found by Block Address modulo Cache size
            unsigned cindex = addr % sizes[i];
            //Tag is the rest of the bits
            unsigned ctag = addr >> shifts[i];
            //Check if cache location is valid
            if(cache[cindex].valid)
            {
                //Check cache tag
                if(cache[cindex].tag == ctag)
                {
                    //LogPrint(DEBUG, "HIT");
                    hits++;
                }
                else
                {
                    cache[cindex].tag = ctag;
                    //hits++;
                }
            }
            else
            {
                cache[cindex].valid = 1;
                cache[cindex].tag = ctag;
            }
            accesses++;
        }
        
        //Add values to string
        returnstring << std::to_string(hits) << "," << std::to_string(accesses) << ";";

        //Be kind, rewind!
        BeKindRewind(infile);
        //Free allocated memory
        delete [] cache;
    }
    
    //Write to file
    WriteFile(TRUNCATE, outfile, returnstring.str());
    return true;
}

bool SetAssCache(std::ifstream &infile)
{
    int asses[4] = {2, 4, 8, 16};
}

std::string GetHexString(unsigned long addr)
{
    std::stringstream ss;
    ss << std::hex << addr;
    return ss.str();
}

void BeKindRewind(std::ifstream &infile)
{
    infile.clear();
    infile.seekg(0, std::ios::beg);
}
