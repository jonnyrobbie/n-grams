#include "file.h"

int file::load()
{
    ifstream s_file (filename, ios::in|ios::binary|ios::ate);
    if (s_file.is_open())
    {
        s_size = s_file.tellg();
        byte = new char [s_size];
        s_file.seekg (0, ios::beg);
        s_file.read (byte, s_size);
        s_file.close();
        //cout << "File succesfully loaded. Size " << s_size << " B" << endl;
        size = s_size;
        return 0;
    }
    else
    {
        //cout << "Unable to open file.";
        return 1;
    }
}

int file::save()
{
    ofstream s_file (filename, ios::out|ios::binary);
    if (s_file.is_open())
    {
        s_file.write (byte, size);
        s_file.close();
        //cout << "File succesfully saved. Size " << size << " B" << endl;
        return 0;
    }
    else
    {
        //cout << "Unable to save file.";
        return 1;
    }
}

file::~file()
{
    delete[] byte;
}
