
#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class file
{
public:
    int size;
    char * byte;
    //char *
    file() {};
    int load();
    int save();
    char * filename;
    ~file();
private:
    streampos s_size;
};

#endif // FILE_H
