#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE 8192
#define MAX_ENTROPY_FILE_SIZE 30 * 1024 * 1024 //30 MBytes
#define N_SAMPLES 4

void usage(char* progname)
{
    std::cout << "Usage: " << progname << " filename" << std::endl;
}

unsigned   syms[256];
unsigned   total;

double calc_entropy(FILE* fp, size_t sz)
{
    unsigned char buffer[BUFFER_SIZE];

    int count = total;
    while (total - count < sz)
    {
        int sz = fread(buffer, 1, BUFFER_SIZE, fp);
        if (sz <= 0)
            break;

        for (int i = 0; i < sz; i++)
            syms[buffer[i]]++;            
        
        total += sz;
    }

    double entropy = 0;
	for(int i=0; i < 256; i++){
		if (syms[i])
        {
			double p = double(syms[i])/double(total);
#ifdef DEBUG
            std::cout << syms[i] << ' ' << std::setprecision(4) << p << ' ' << entropy << std::endl;
#endif
			entropy -= p * log2(p);
		}
	}

    return entropy;
}


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        usage(argv[0]);
        return -1;
    }

    for (int i = 0; i < 256; i++)
        syms[i] = 0;

    total = 0;

    std::string fname(argv[1]);
    FILE* fp = fopen(fname.c_str(), "rb");
    if (fp == NULL)
    {
        std::cerr << "couldn't open the file " << fname << std::endl;
        return -2;
    }

    struct stat b_stat;
    if (fstat(fileno(fp), &b_stat) != 0)
        std::cerr << "error at getting file stats " << errno << std::endl;


    double entropy = 0;
    if (b_stat.st_size < MAX_ENTROPY_FILE_SIZE)
        entropy =  calc_entropy(fp, b_stat.st_size);
    else
    {
        off_t offset = 0;
        for (int i = 0; i < N_SAMPLES; i++, offset += b_stat.st_size/N_SAMPLES)
        {
            lseek(fileno(fp), offset, SEEK_SET);
            entropy =  calc_entropy(fp, MAX_ENTROPY_FILE_SIZE/N_SAMPLES);
#ifdef DEBUG
            std::cout << total << ' '  << std::setprecision(4) << entropy/8.0 << std::endl;
#endif
        }
    }

    std::cout << total << ' '  << std::setprecision(4) << entropy/8.0 << std::endl;
    fclose(fp);

	return 0;    
}
