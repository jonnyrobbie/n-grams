#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include "file.h"

using namespace std;

struct comFlag_t {
// http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
// http://www.ibm.com/developerworks/aix/library/au-unix-getopt.html
    int ngram_dim;
    int gen_txt;
    char *outputFilename;
    char *alphFilename;
    char **inputFiles;
	int numInputFiles;
    int verbosity;
    int help;
} comFlag;

static const char *optString = "n:g:o:a:vh?";

static const struct option longOpts[] = {
    { "ngram", required_argument, NULL, 'n' },
    { "generate", required_argument, NULL, 'g' },
    { "output", required_argument, NULL, 'o' },
    { "alphabet", required_argument, NULL, 'a' },
    { "verbose", required_argument, NULL, 'v' },
    { "help", required_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

int intsize;

float getRandom(float a, float b)
{
    return ((b-a)*((float)rand()/RAND_MAX))+a;
}

void putFreq()
{
    file txtFile;
    txtFile.filename = comFlag.inputFiles[0];
    if (txtFile.load())
    {
        if (comFlag.verbosity>0) cout << "Error loading file!\n";
        return;
    }
    char dimension = comFlag.ngram_dim;
    char * lastChar = NULL;
    if (dimension>0)
    {
        lastChar = new char[dimension-1];
    }
    for (int i=0; i<dimension; i++)
    {
        lastChar[i]=0;
    }
    int * freq;
    //
    char alphSize = 32;
    char * alph;
    alph = new char[alphSize];
    alph = {"abcdefghijklmnopqrstuvwxyz ,.?!'"};
    //
    int dictSize = pow(alphSize, dimension);
    freq = new int[dictSize];
    for (int i=0; i<dictSize; i++)
    {
        freq[i]=0;
    }
    //alph = {"abcdefghijklmnopqrstuvwxyz ,.?!'"}; //english set
    for (int i=0; i<alphSize; i++) freq[i]=0;
    int size = 0;
    int lastB;
    int fOffset = 0;
    for (int i=0; i<txtFile.size; i++) // pro kazdy znak ve zdroji
    {
        for (int c=0; c<26; c++)
        {
            if ((int)txtFile.byte[i] == c+65 || (int)txtFile.byte[i] == c+97)
            {
                freq[c+fOffset]++;
                lastB = c;
                c=26;
                size++;
            }
        }
        if ((int)txtFile.byte[i] == 32)
        {
            freq[26+fOffset]++;
            size++;
            lastB=26;
        }
        if ((int)txtFile.byte[i] == 44)
        {
            freq[27+fOffset]++;
            size++;
            lastB=27;
        }
        if ((int)txtFile.byte[i] == 46)
        {
            freq[28+fOffset]++;
            size++;
            lastB=28;
        }
        if ((int)txtFile.byte[i] == 63)
        {
            freq[29+fOffset]++;
            size++;
            lastB=29;
        }
        if ((int)txtFile.byte[i] == 33)
        {
            freq[30+fOffset]++;
            size++;
            lastB=30;
        }
        if ((int)txtFile.byte[i] == 39)
        {
            freq[31+fOffset]++;
            size++;
            lastB=31;
        }
        fOffset = 0;
        for (int d=0; d<dimension-2; d++)//posunu posledni znaky a pridam ten nejnovejsi
        {
            lastChar[dimension-d-2] = lastChar[dimension-d-3];
        }
        if (dimension>1) lastChar[0] = lastB;
        for (int d=1; d<dimension; d++)
        {
            fOffset = fOffset + pow(alphSize, d)*(lastChar[d-1]);
        }
    }
    if (comFlag.verbosity>0) cout << "size: " << size << endl;
    char * output;
    if (comFlag.verbosity>1) cout << "dictSize: " << dictSize << endl << "alphSize: " << (int)alphSize << endl << "sizeofint: " << intsize << endl << "dicrSOI: " << dictSize*intsize << endl;
    if (comFlag.verbosity>1) cout  << "char[5+(int)alphSize+dictSize*intsize] = " << (5+(int)alphSize+dictSize*intsize) << endl;
    int outSize = (5+(int)alphSize+dictSize*intsize);
    output = new char[outSize];
    output[0] = 76;
    output[1] = 67;
    output[2] = 70;
    output[3] = dimension;
    output[4] = (int)alphSize;
    for (int i=5; i<5+(int)alphSize; i++)
    {
        output[i] = alph[i-5];
    }
    for (int i=5+(int)alphSize; i<5+(int)alphSize+dictSize*intsize; i++)
    {
        output[i] = ((char*)freq)[i-5-(int)alphSize];
    }
    /*for (int i=0; i<5+alphSize+dictSize*sizeof(int); i++)
    {
        cout << (int)output[i] << " ";
    }*/
    file lcfFile;
    lcfFile.size = 5+(int)alphSize+dictSize*intsize;
    lcfFile.filename = comFlag.outputFilename;
    lcfFile.byte = output;
    if (lcfFile.save())
    {
        if (comFlag.verbosity>0) cout << "Error saving LCF file.";
    }
}

void getFreq()
{

    file lcfFile;
    lcfFile.filename = comFlag.inputFiles[0];
    if (lcfFile.load())
    {
        cout << "Error loading file!";
        return;
    }
    if (lcfFile.byte[0]!=76 || lcfFile.byte[1]!=67 || lcfFile.byte[2]!=70)
    {
        cout << "Not an LCF file!" << endl;
        return;
    }
    int dimension = lcfFile.byte[3];
    char * lastChar;
    if (dimension>0)
    {
        lastChar = new char[dimension-1];
    }
    int alphSize = lcfFile.byte[4];
    if (lcfFile.size != 5+alphSize+pow(alphSize, dimension)*sizeof(int))
    {
        cout << "File does not have the correct size!";
        return;
    }
    int * freq;
    int dictSize = pow(alphSize, dimension);
    freq = new int[dictSize];
    int genSize = comFlag.gen_txt;
    char * alph;
    alph = new char[alphSize];
    for (int i=0; i<dictSize*sizeof(int); i++)
    {
        ((char*)freq)[i] = lcfFile.byte[i+5+alphSize];
    }
    for (int i=0; i<alphSize; i++)
    {
        alph[i] = lcfFile.byte[i+5];
    }
    /*
    for (int i=0; i<dictSize; i++)
    {
        cout << freq[i] << " ";
    }
    cout << endl;
    for (int i=0; i<alphSize; i++)
    {
        cout << alph[i];
    }
    */
    file txtFile;
    txtFile.size = genSize;
    char * output;
    output = new char[genSize];
    for (int i=0; i<dimension-1; i++)
    {
        lastChar[i] = 1;
    }
    for (int i=0; i<genSize; i++)
    {
        int * alphFreq;
        alphFreq = new int[alphSize];
        int fOffset = 0;
        for (int d=1; d<dimension; d++)
        {
            fOffset = fOffset + pow(alphSize, d)*(lastChar[d-1]);
        }
        for (int j=0; j<alphSize; j++)
        {
            if (dimension < 1) alphFreq[j] = 1;
            else alphFreq[j] = freq[j+fOffset];
        }
        int length=0;
        for (int j=0; j<alphSize; j++)
        {
            length += alphFreq[j];
        }
        float * cFreq;
        cFreq = new float[alphSize];
        if (length > 0)
        {
            cFreq[0] = (float)alphFreq[0] / (float)length;
            for (int j=1; j<alphSize; j++)
            {
                cFreq[j] = ((float)alphFreq[j] / (float)length) + cFreq[j-1];
            }
        }
        else
        {
            cFreq[0] = 1 / (float)alphSize;
            for (int j=1; j<alphSize; j++)
            {
                cFreq[j] = (1 / (float)alphSize) + cFreq[j-1];
            }
        }
        float random = getRandom(0, 1);
        for (int j=0; j<alphSize; j++)
        {
            if (random < cFreq[j])
            {
                if (comFlag.verbosity>1) cout << alph[j];
                output[i] = alph[j];
                for (int d=0; d<dimension-2; d++)//posunu posledni znaky a pridam ten nejnovejsi
                {
                    lastChar[dimension-d-2] = lastChar[dimension-d-3];
                }
                if (dimension>1) lastChar[0] = j;
                j=alphSize;
            }
        }
        delete alphFreq;
        delete cFreq;
    }
    txtFile.byte = output;
    txtFile.filename = comFlag.outputFilename;
    if (txtFile.save())
    {
        cout << "Error saving generated file.";
    }
}

void display_usage()
{
    cout << "Generating n-grams from corpus and text from n-grams." << endl;
    cout << "N-Grams 0.1" << endl;
    cout << "Usage: n_grams [options] input_file. Example:" << endl << endl;
    cout << "n_grams -n 3 -o trigram.lcf corpus.txt" << endl << endl << endl;
    cout << "-n, --ngram [integer]     Generates n-gram form corpus to a LCF file. Parameter" << endl;
    cout << "                          sets n and has to be a positive integer." << endl;
    cout << "-g, --generate [integer]  Generates random text using LCF file. Parameter sets " << endl;
    cout << "                          the length of the generated file in bytes. Only -n or" << endl;
    cout << "                          -g can be set at a time." << endl;
    cout << "-o, --output [string]     Path to output." << endl;
    cout << "-a, --alphabet [string]   Path to custom alphabet file. If not set, the program" << endl;
    cout << "                          uses default English." << endl;
    cout << "-v, --verbose             Verbosity level." << endl;
    cout << "-h, -?, --help            Display this help." << endl;
}

void printFlags()
{
    cout << "=====\n";
    cout << "FLAGS:" << endl;
    cout << "ngrams: " << comFlag.ngram_dim << endl;
    cout << "generate: " << comFlag.gen_txt << endl;
    if (comFlag.outputFilename != NULL)
    {
        cout << "output filename: " << comFlag.outputFilename << endl;
    }
    if (comFlag.alphFilename != NULL)
    {
        cout << "alphabet filename: " << comFlag.alphFilename << endl;
    }
    cout << "verbosity: " << comFlag.verbosity << endl;
    cout << "help: " << comFlag.help << endl;
    cout << "num input files: " << comFlag.numInputFiles << endl;
    for (int i = 1; i<=comFlag.numInputFiles; i++)
    {
        cout << "File " << i << ": " << comFlag.inputFiles[i-1] << endl;
    }
    cout << "=====\n";
}
int main( int argc, char *argv[] )
{
    srand(time(NULL));
    intsize = sizeof(int);
    //parsing command line args
    int opt = 0;
    int longIndex = 0;
    comFlag.alphFilename = NULL;
    comFlag.help = 0;
    comFlag.inputFiles = NULL;
    comFlag.ngram_dim = -1;
    comFlag.gen_txt = 0;
    comFlag.numInputFiles = 0;
    comFlag.outputFilename = NULL;
    comFlag.verbosity = 0;
    opt = getopt_long ( argc, argv, optString, longOpts, &longIndex );
    while( opt != -1 ) {
		switch( opt ) {
			case 'n':
				comFlag.ngram_dim = atoi(optarg);
				break;
			case 'o':
				comFlag.outputFilename = optarg;
				break;
            case 'g':
				comFlag.gen_txt = atoi(optarg);
				if (comFlag.verbosity>1) cout << "generate size: " << comFlag.gen_txt;
				break;
            case 'a':
				comFlag.alphFilename = optarg;
				break;
			case 'v':
				comFlag.verbosity++;
				break;
			case 'h':
			case '?':
				display_usage();
				return 0;
			default:
				break;
		}
		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	}
	comFlag.inputFiles = argv + optind;
	comFlag.numInputFiles = argc - optind;
	if (comFlag.verbosity>1) printFlags();

    //checking parsed flags
	if (comFlag.outputFilename == NULL)
    {
        cout << "Missing output file.\n";
        return 1;
    }
    if (comFlag.ngram_dim>-1 && comFlag.gen_txt == 0)
    {
        if (comFlag.verbosity>0) cout << "Computing n-gram.\n";
        putFreq();
        if (comFlag.verbosity>0) cout << "Done.\n";
        return 0;
    } else if (comFlag.ngram_dim==-1 && comFlag.gen_txt>0)
    {
        if (comFlag.verbosity>0) cout << "Generating text.\n";
        getFreq();
        if (comFlag.verbosity>0) cout << "Done.\n";
        return 0;
    } else
    {
        cout << "Invalid parameters.\n Parameter of -n and -g has to be greater than zero and only one can be set at a time.";
        return 1;
    }
    return 0;
}
