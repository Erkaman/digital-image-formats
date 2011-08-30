#include <stdio.h>

void encode(char * fp)
{
    FILE * in,out;

    in = fopen(file,"rb");
    out = fopen(file,"wb");
}

int main(int argc, char *argv[])
{
    if(argc != 1)
	encode(argv[1]);

    return 0;
}

