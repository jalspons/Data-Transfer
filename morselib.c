#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "morselib.h"

static const char *morses = " *ETIANMSURWDKGOHVF*L*PJBXCYZQ**54*3***2*******16*******7***8*90";

static const int morseIndexAlphas[] = {
    5,  // A
    24, // B
    26, // C
    12, // D
    2,  // E
    18, // F
    14, // G
    16, // H
    4,  // I
    23, // J
    13, // K
    20, // L
    7,  // M
    6,  // N
    15, // O
    22, // P
    29, // Q
    10, // R
    8,  // S
    3,  // T
    9,  // U
    17, // V
    11, // W
    25, // X
    27, // Y
    28,  // Z
};

static const short morseIndexNumbers[] = {
    63, // 0
    47, // 1
    39, // 2
    35, // 3
    33, // 4
    32, // 5
    48, // 6
    56, // 7
    60, // 8
    62, // 9
};

static const short space = {
    0, /* space character */
};

int encodeMorse(char c)
{
    int cIndex;
    
    if (isspace(c)) {
        cIndex = space;
    }
    else if (isdigit(c)) {
        cIndex = morseIndexNumbers[(int)c - 48];
    }
    else if (isalpha(c)) {
        cIndex = morseIndexAlphas[(int)tolower(c) -  97];
    }
    
    return cIndex;
}

char decodeMorse(int val)
{
    return morses[val];
}

int codeShifts(int morseCode)
{
    int n;
    
    if (morseCode < 4) n = 2;
    else if (morseCode < 8) n = 3;
    else if (morseCode < 16) n = 4;
    else if (morseCode < 32) n = 5;

    return n;
}
