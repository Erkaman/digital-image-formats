#include "deflate.h"
#include "../io.h"
#include "../bits.h"
#include "../huffman/huffman.h"
#include "../lzss/lzss_util.h"
#include "common.h"
#include <cmath>
#include <cstdlib>

using std::vector;

vector<unsigned int> tokensToDeflateCodes(
    const vector<Token> & tokens,
    vector<unsigned int> & litteralAndLengthCodes,
    vector<unsigned int> & distanceCodes);

vector<BYTE> deflate(const vector<BYTE> & data)
{
    const bool useFixedCodes = true;

    /* The maximum size of the window buffer. */
    const unsigned int windowSize = pow(2,15);

    /* the maximum length code. */
    const unsigned int lookAheadSize = 258;

    BitVectorWriter * outBits = new BitVectorWriter(LSBF);

    vector<Token> LZSS_Tokens =
        compress(windowSize,lookAheadSize,data);

    vector<unsigned int> litteralAndLengthCodes;
    vector<unsigned int> distanceCodes;

    vector<unsigned int> deflateCodes =
        tokensToDeflateCodes(
            LZSS_Tokens,
            litteralAndLengthCodes,
            distanceCodes);

    /* Assume fixed codes are used from now on. */

    CodesList distanceCodesList;
    CodesList litteralAndLengthCodesList;

    if(useFixedCodes) {

	/* Last block. */
        outBits->writeBits(1, 1);
        outBits->writeBits(BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES, 2);

        distanceCodesList =  getFixedDistanceCodes();
        litteralAndLengthCodesList =  getFixedHuffmanCodes();
    } else {

        outBits->writeBits(1, 1);
        outBits->writeBits(BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES, 2);

        /* Get the code lengths for the literal and length alphabet. */

	/* Add the single end of block code*/
	litteralAndLengthCodes.push_back(END_OF_BLOCK);


        CodeLengths litteralAndLengthCodeLengths  = makeCodeLengths(
            litteralAndLengthCodes,
            286, /* 288?*/
            15);

        litteralAndLengthCodeLengths =
            cutTrailingZeroCodeLengths(litteralAndLengthCodeLengths,257);

        /* Translate the code lengths to a code list.*/
        litteralAndLengthCodesList = translateCodes(litteralAndLengthCodeLengths);


        CodeLengths codeLengthAlphabetCodes;
        CodeLengths temp;


        /* Compress the litteral code lengths. */
        CodeLengths compressedLitteralAndLengthCodeLengths = compressCodeLengths(
            litteralAndLengthCodeLengths,
            temp);

        /* Gather the frequencies of the length codes. */
        codeLengthAlphabetCodes.insert(
            codeLengthAlphabetCodes.end(),
            temp.begin(),
            temp.end());

        /* Do the same thing for the distance codes.  */

        CodeLengths compressedDistanceCodeLengths;

        CodeLengths distanceCodeLengths  = makeCodeLengths(
            distanceCodes,
            30, /* 32?*/
            15);

        distanceCodeLengths =
            cutTrailingZeroCodeLengths(distanceCodeLengths,1);

        distanceCodesList = translateCodes(distanceCodeLengths);

        temp.clear();

        compressedDistanceCodeLengths = compressCodeLengths(
            distanceCodeLengths,
            temp);

        codeLengthAlphabetCodes.insert(
            codeLengthAlphabetCodes.end(),
            temp.begin(),
            temp.end());

        CodeLengths codeLengthCodeLengths =
            makeCodeLengths(codeLengthAlphabetCodes, 19,7);

        CodesList codeLengthCodes = translateCodes(codeLengthCodeLengths);

        /* Write the header. */

        outBits->writeBits(
	    litteralAndLengthCodeLengths.size() - 257,
	    5);

        outBits->writeBits(distanceCodeLengths.size() - 1, 5);

        /* Write the code lengths alphabet and HCLEN. */
        writeCodeLengthCodeLengths(codeLengthCodeLengths, outBits);

        writeCompressedCodeLengths(
            compressedLitteralAndLengthCodeLengths,
            codeLengthCodes,
            outBits);

        writeCompressedCodeLengths(
            compressedDistanceCodeLengths,
            codeLengthCodes,
            outBits);
    }

    for(size_t i = 0; i < deflateCodes.size(); ++i){
        unsigned int code = deflateCodes[i];

        /* Literal value. */
        if(code <= LITTERAL_VALUES_MAX){
            writeCode(litteralAndLengthCodesList[code], outBits);
        } else{
            /* distance length pair. */

            /* write the length code. */
            writeCode(litteralAndLengthCodesList[code], outBits);

            unsigned int extraBitsLength =
                lengthTable[code-LENGTH_MIN].extraBits;

            /* If the length code has any extra bits at all. */

            if(extraBitsLength > 0){

                /* The extra bits of the length are found directly after
                 * the length code*/
                i++;
                unsigned int extraBits = deflateCodes[i];

                outBits->writeBits(extraBits, extraBitsLength);
            }

            /* Write the distance code*/
            ++i;

            unsigned int distcode = deflateCodes[i];

            writeCode(distanceCodesList[distcode], outBits);

            extraBitsLength = distanceTable[distcode].extraBits;

            if(extraBitsLength > 0){
                ++i;

                unsigned int extraBits = deflateCodes[i];

                outBits->writeBits(extraBits, extraBitsLength);
            }


        }
    }

    writeCode(litteralAndLengthCodesList[END_OF_BLOCK], outBits);

    outBits->writeBits(0,8);

    vector<BYTE> compressed = outBits->getOutputVector();

    delete outBits;

    return compressed;
}

vector<unsigned int> tokensToDeflateCodes(
    const vector<Token> & tokens,
    vector<unsigned int> & litteralAndLengthCodes,
    vector<unsigned int> & distanceCodes)
{
    vector<unsigned int> deflateCodes;

    for(size_t i = 0; i < tokens.size(); ++i){
        Token token = tokens[i];

        if(token.type == SymbolToken){

            deflateCodes.push_back(token.symbol);
            litteralAndLengthCodes.push_back(token.symbol);

        } else if(token.type == OffsetLengthToken) {

            unsigned int length;
            /* Brute force find the matching length code. */
            for(length = LENGTH_MIN; length <= LENGTH_MAX; ++length){

                LengthTableEntry len = lengthTable[length - LENGTH_MIN];

                unsigned int lower = len.minLength;
                unsigned int upper = len.minLength + pow(2, len.extraBits) - 1;

                if(lower <= token.length && token.length <= upper){
                    deflateCodes.push_back(length);
                    litteralAndLengthCodes.push_back(length);

                    if(len.extraBits > 0){
                        deflateCodes.push_back(token.length - len.minLength);
                    }
                    break;
                }
            }

            if(length > LENGTH_MAX){
                printf("ERROR, found length code with no match\n");
                exit(1);
            }

            /* Brute force find the matching distance/offset code. */
            unsigned int distance;
            for(distance = 0; distance < DISTANCE_CODES; ++distance){

                DistanceTableEntry dist = distanceTable[distance];

                unsigned int lower = dist.minDist;
                unsigned int upper = dist.minDist + pow(2, dist.extraBits) - 1;

                if(lower <= token.offset && token.offset <= upper){
                    deflateCodes.push_back(distance);
                    distanceCodes.push_back(distance);

                    /* TODO: does this work? */
                    if(dist.extraBits > 0){

                        /* Add the extra bits. */

                        deflateCodes.push_back(token.offset - dist.minDist);
                    }

                    break;
                }
            }

            if(distance > DISTANCE_CODES){
                printf("ERROR, found distance code with no match:%d\n", token.offset);
                exit(1);
            }

        }
    }

    return deflateCodes;
}
