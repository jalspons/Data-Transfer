#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "morselib.h"

static const char *morses = " \nETIANMSURWDKGOHVF*L*PJBXCYZQ**"\
                             "54*3***2*******16*******7***8*90";

static const int morsecode_index_alphas[] = {
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
    28, // Z
};

static const short morsecode_index_nums[] = {
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

static const short control_chars[] = {
    0, /* Space */
    1, /* Newline */
};

int encode_morse(char c)
{
    int index;
    
    if (c == ' ')
        index = control_chars[0];
    else if (c == '\n')
        index = control_chars[1];
    else if (isdigit(c))
        index = morsecode_index_nums[(int)c - 48];
    else if (isalpha(c))
        index = morsecode_index_alphas[(int)tolower(c) -  97];
    
    return index;
}

char decode_morse(int val)
{
    return morses[val];
}

int morsecode_len(int morseCode)
{
    int n;
    
    if (morseCode < 4) 
        n = 2;
    else if (morseCode < 8) 
        n = 3;
    else if (morseCode < 16) 
        n = 4;
    else if (morseCode < 32) 
        n = 5;

    return n;
}
