#include "common.h"

const LengthTableEntry lengthTable[LENGTH_CODES] = {

    {0,3},
    {0,4},
    {0,5},
    {0,6},
    {0,7},
    {0,8},
    {0,9},
    {0,10},

    {1,11},
    {1,13},
    {1,15},
    {1,17},

    {2,19},
    {2,23},
    {2,27},
    {2,31},

    {3,35},
    {3,43},
    {3,51},
    {3,59},

    {4,67},
    {4,83},
    {4,99},
    {4,115},

    {5,131},
    {5,163},
    {5,195},
    {5,227},

    {0, 258}
};

const DistanceTableEntry distanceTable[DISTANCE_CODES] = {
    {0,1},
    {0,2},
    {0,3},
    {0,4},

    {1,5},
    {1,7},

    {2,9},
    {2,13},

    {3,17},
    {3,25},

    {4,33},
    {4,49},

    {5,65},
    {5,97},

    {6,129},
    {6,193},

    {7,257},
    {7,385},

    {8,513},
    {8,769},

    {9,1025},
    {9,1537},

    {10,2049},
    {10,3073},

    {11,4097},
    {11,6145},

    {12,8193},
    {12,12289},

    {13,16385},
    {13,24577},
};



CodesList getFixedHuffmanCodes(void)
{
    CodesList fixedHuffmanCodes(HUFFMAN_CODES);

    int i;
    HuffmanCode code;

    /* Litteral value 256 is the end code.*/

    /* Literal values 0-143 are given the codes
       00110000(=48=0x30) - 10111111(191=0xbf) of length 8*/
    for(i = 0; i <= 143; ++i){
        code.value = 0x30 + i;
        code.length = 8;
        fixedHuffmanCodes[i] = code;
    }

    /* Literal values 144-255 are given the codes
       1 1001 0000(=400=0x190) - 1 1111 1111(=511=0x1ff) of length 9*/
    for(i = 144; i <= 255; ++i){
        code.value = 0x190 + i - 144;
        code.length = 9;
        fixedHuffmanCodes[i] = code;
    }

    /* Literal values 256-279 are given the codes
       000 0000(=0=0x0) - 001 0111(=23=0x17) of length 7*/
    for(i = 256; i <= 279; ++i){
        code.value = 0x0 + i - 256;
        code.length = 7;
        fixedHuffmanCodes[i] = code;
    }

    /* Literal values 280-287 are given the codes
       1100 0000(=192=0xc0) - 1100 0111(=199=0xc7) of length 8*/
    for(i = 280; i <= 287; ++i){
        code.value = 0xc0 + i - 280;
        code.length = 8;
        fixedHuffmanCodes[i] = code;
    }

    return fixedHuffmanCodes;
}

CodesList getFixedDistanceCodes(void)
{
    int i;
    HuffmanCode code;

    CodesList fixedDistanceCodes(32);

    /*
      "Distance codes 0-31 are represented by (fixed-length) 5-bit codes"
    */
    for(i = 0; i <= 31; ++i){
        code.value = i;
        code.length = 5;
        fixedDistanceCodes[i] = code;
    }

    return fixedDistanceCodes;
}
