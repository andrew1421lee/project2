#include <sstream>
//#include <cmath>
#include "Utils.h"

struct BLOCK
{
    //unsigned index; //Don't need another index, just use array index
    char valid;
    unsigned tag;
    unsigned last;
    unsigned temp;
};

//Global Variables
std::string outfile;
std::string dummy;

//Forward Declarations
bool DirectCache(std::ifstream &infile);
bool SetAssCache(std::ifstream &infile);
bool FullAssCache(std::ifstream &infile);
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
        if(!SetAssCache(infile)) LogPrint(ERROR, "Failed Set Associative Cache");
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
    //Magic numbers!
    int asses[4] = {2, 4, 8, 16};
    int shifts[4] = {8, 7, 6, 5};
    int cachesize = 16384;
    std::stringstream returnstring;

    for(int i = 0; i < 4; i++)
    {
        if(i != 0)
        {
            returnstring << " ";
        }

        unsigned long hits = 0;
        unsigned long accesses = 0;
        
        BLOCK** cache = new BLOCK*[cachesize / (32 * asses[i])];

        //Loops number of sets
        for(int j = 0; j < (cachesize / (32 * asses[i])); j++)
        {
            cache[j] = new BLOCK[asses[i]];
            for(int k = 0; k < asses[i]; k++)
            {
                cache[j][k].valid = 0;
                cache[j][k].tag = 0;
                cache[j][k].last = 0;
            }
            //LogPrint(INFO, std::to_string(asses[i]) + " - " + std::to_string(j));
        }
        //Variables for reading
        std::string instr;
        unsigned long addr;
        //unsigned long time;

        //Reading loop
        while(infile >> instr >> std::hex >> addr)
        {
            //LOL offset
            addr >>= 5;
            //Index is found by Block Address modulo sets in cache
            unsigned cindex = addr % (cachesize / (32 * asses[i]));
            //Tag is the rest of the bits
            unsigned ctag = addr >> shifts[i];

            bool found = false;

            //Loop through each block in the set
            //Check for hit or empty spot
            for(int j = 0; j < asses[i]; j++)
            {
                //valid block and correct tag
                if(cache[cindex][j].valid && cache[cindex][j].tag == ctag)
                {
                    cache[cindex][j].last = accesses; //update time
                    hits++; //inc hits
                    found = true;
                    break; // stop looking
                }
                //Empty spot
                else if(!cache[cindex][j].valid)
                {
                    //empty spot, add to cache
                    cache[cindex][j].valid = 1;
                    cache[cindex][j].tag = ctag;
                    cache[cindex][j].last = accesses;
                    found = true;
                    break;
                }
            }
            //Kick LRU out
            if(!found)
            {
                //Find the LRU block
                unsigned lru = 0;
                for(int j = 1; j < asses[i]; j++)
                {
                    if(cache[cindex][lru].last > cache[cindex][j].last)
                    {
                        lru = j;
                    }
                }
                //Kick it out
                cache[cindex][lru].tag = ctag;
                cache[cindex][lru].last = accesses;
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
    WriteFile(APPEND, outfile, "\n" + returnstring.str());
    return true;
}

bool FullAssCache(std::ifstream &infile)
{
    
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
