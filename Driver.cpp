#include <sstream>
//#include <cmath>
#include "Utils.h"

struct BLOCK
{
    //unsigned index; //Don't need another index, just use array index
    char valid;
    unsigned tag;
    unsigned last;
    //unsigned temp; Lol dont need
};

struct NODE
{
    unsigned lrupath; //0 for left, 1 for right, 2 for there!
    int index; // the thingy
};

//Global Variables
std::string outfile;
std::string dummy;

//Forward Declarations
bool DirectCache(std::ifstream &infile);
bool SetAssCache(std::ifstream &infile);
bool FullAssCache(std::ifstream &infile);
bool NoAssCache(std::ifstream &infile);
bool PreAssCache(std::ifstream &infile);
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
        //if(!DirectCache(infile)) LogPrint(ERROR, "Failed Direct Cache");
        //if(!SetAssCache(infile)) LogPrint(ERROR, "Failed Set Associative Cache");
        //if(!FullAssCache(infile)) LogPrint(ERROR, "Failed Full Associative Cache");
        //if(!NoAssCache(infile)) LogPrint(ERROR, "Failed Write Miss Associative Cache");
        if(!PreAssCache(infile)) LogPrint(ERROR, "Failed Prefetch Associative Cache");
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
    //Magic numbers!
    int numblocks = 512;
    
    //Output string
    std::stringstream returnstring;

    //Loop twice, once for LRU and once for PLRU
    for(int i = 0; i < 2; i++)
    {
        NODE* bst;
        if(i != 0)
        {
            returnstring << "\n";
            bst = new NODE[1023];
            for(int j = 0; j < 1023; j++)
            {
                //Init leaves
                if(j * 2 + 1 > 1022)
                {
                    bst[j].lrupath = 2;
                }
                else //nodes
                {
                    bst[j].lrupath = 0;
                }
                bst[j].index = -1;
                //LogPrint(DEBUG, "DOES THIS WORK?" + std::to_string(j));
            }
        }

        //important variables
        unsigned long hits = 0;
        unsigned long accesses = 0;

        //Allocate memory for cache
        BLOCK* cache = new BLOCK[numblocks];
        
        //Init blocks
        for(int j = 0; j < numblocks; j++)
        {
            cache[j].valid = 0;
            cache[j].tag = 0;
            cache[j].last = 0;
        }

        //Variables for reading
        std::string instr;
        unsigned long addr;

        //Reading loop
        while(infile >> instr >> std::hex >> addr)
        {
            //if(i!=0)
                //LogPrint(DEBUG, "STATS " + std::to_string(hits) + "," + std::to_string(accesses));
            //LOL offset
            addr >>= 5;
            //Index
            //unsigned cindex = addr % numblocks; //lol we dont need this
            unsigned ctag = addr;
            
            bool found = false;

            //Loop through every block looking for hit or empty spot
            for(int j = 0; j < numblocks; j++)
            {
                if(cache[j].valid && cache[j].tag == ctag)
                {
                    if(i == 0)
                    {
                        cache[j].last = accesses; //update time
                    }
                    else
                    {
                        //Find which item in the BST corresponds to the cache
                        float bst_index;
                        for(int k = 0; k < 1023; k++)
                        {
                            if(bst[k].index == j)
                            {
                                bst_index = k;
                                //LogPrint(DEBUG, "I found the BST to cache");
                                break;
                            }
                        }
                        //Loop through all parents
                        while(bst_index > 0)
                        {
                            //fliperino
                            int temp = (int)bst_index;
                            if(bst_index > (float)temp)
                            {
                                if(bst[temp].lrupath == 1)
                                {
                                    bst[temp].lrupath = 0;
                                }
                            }
                            else
                            {
                                if(bst[temp].lrupath == 0)
                                {
                                    bst[temp].lrupath == 1;
                                }
                            }

                            bst_index = (bst_index - 1.0) / 2.0;
                            //LogPrint(DEBUG, "Fixing parents on access");
                        }
                        //LogPrint(DEBUG, "OK fixed parents");;
                    }
                    hits++;
                    found = true;
                    break;
                }
                else if(!cache[j].valid)
                {
                    //empty spot
                    cache[j].valid = 1;
                    cache[j].tag = ctag;
                    if(i == 0)
                    {
                        cache[j].last = accesses;
                    }
                    else
                    {
                        //traverse BST
                        unsigned end = 0;
                        while(bst[end].lrupath != 2)
                        {
                            if(bst[end].lrupath == 0) // turn left
                            {
                                bst[end].lrupath = 1;
                                end = end * 2 + 1;
                            }
                            else if(bst[end].lrupath == 1)//turn right
                            {
                                bst[end].lrupath = 0;
                                end = end * 2 + 2;
                            }
                            //LogPrint(DEBUG, "Looking for spot " + std::to_string(bst[end].lrupath) + " -  " + std::to_string(j));
                        }
                        bst[end].index = j;
                        //LogPrint(DEBUG, "OK filled empty space");
                    }
                    found = true;
                    break;
                }
            }
            //Kick LRU out
            if(!found)
            {
                if(i == 0)
                {
                    unsigned lru = 0;
                    for(int j = 1; j < numblocks; j++)
                    {
                        if(cache[lru].last > cache[j].last)
                        {
                            lru = j;
                        }
                    }
                    //Kick 'em
                    cache[lru].tag = ctag;
                    cache[lru].last = accesses;               
                }
                else //HOT COLD
                {
                    //Traverse BST
                    unsigned end = 0;
                    while(bst[end].lrupath != 2)
                    {
                        if(bst[end].lrupath == 0) // turn left
                        {
                            bst[end].lrupath = 1;
                            end = end * 2 + 1;
                        }
                        else if (bst[end].lrupath == 1)//turn right
                        {
                            bst[end].lrupath = 0;
                            end = end * 2 + 2;
                        }
                        //LogPrint(DEBUG, "Looking for something to kick out");
                    }
                    cache[bst[end].index].tag = ctag;
                    //cache[bst[end].index].last = accesses;
                    //LogPrint(DEBUG, "OK kicked something out");
                }
            }
            accesses++;
        }

        returnstring << std::to_string(hits) << "," << std::to_string(accesses) << ";";
        BeKindRewind(infile);
        //LogPrint(DEBUG, "Where am I?");
        //delete [] cache; //This causes a segfault for unknown reasons LOL ignore
        //delete [] bst;

        //LogPrint(DEBUG, returnstring.str());
        //std::cin >> dummy;
        //WriteFile(APPEND, outfile, returnstring.str());
    }
    WriteFile(APPEND, outfile, "\n"+ returnstring.str());
    return false;
}

bool NoAssCache(std::ifstream &infile)
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
                else if(!cache[cindex][j].valid && instr.compare("S") != 0)
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
            if(!found && instr.compare("S") != 0)
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

bool PreAssCache(std::ifstream &infile)
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
            
            unsigned preindex = (addr + 1) % (cachesize / (32 * asses[i]));
            unsigned pretag = (addr + 1) >> shifts[i]; 
            bool prefound = false;

            //Loop through each block in the set
            //Check for hit or empty spot
            for(int j = 0; j < asses[i]; j++)
            {
                //valid block and correct tag
                if(cache[preindex][j].valid && cache[preindex][j].tag == pretag)
                {
                    cache[preindex][j].last = accesses; //update time
                    //hits++; //inc hits
                    prefound = true;
                    break; // stop looking
                }
                //Empty spot
                else if(!cache[preindex][j].valid)
                {
                    //empty spot, add to cache
                    cache[preindex][j].valid = 1;
                    cache[preindex][j].tag = pretag;
                    cache[preindex][j].last = accesses;
                    prefound = true;
                    break;
                }
            }
            //Kick LRU out
            if(!prefound)
            {
                //Find the LRU block
                unsigned lru = 0;
                for(int j = 1; j < asses[i]; j++)
                {
                    if(cache[preindex][lru].last > cache[preindex][j].last)
                    {
                        lru = j;
                    }
                }
                //Kick it out
                cache[preindex][lru].tag = pretag;
                cache[preindex][lru].last = accesses;
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
