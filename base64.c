#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

char *base64_encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode (FILE *in, FILE *out)
{
    size_t i;
    uint8_t byte[3], l;
    for(i = 0; (l = fread(&byte, 1, 3, in));)
    {
        putc(base64_encoding[byte[0] >> 2], out);
        putc(base64_encoding[((byte[0] & 3) << 4) | ((l == 1) ? 0 : (byte[1] >> 4))], out);
        if(l > 1)
            putc(base64_encoding[((byte[1] & 15) << 2) | ((l == 2) ? 0 : (byte[2] >> 6))], out);
        if(l == 3)
            putc(base64_encoding[byte[2] & 63], out);
        else
            break;
    }
    switch(l)
    {
        case 1: putc('=', out);
        case 2: putc('=', out);
    }
}

int16_t base64_value (FILE *in)
{
    uint8_t value = getc(in);
    if(feof(in))
        exit(0);
    switch(value)
    {
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        return value - 'A';
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        return value - 'a' + 26;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
        return value - '0' + 52;
        case '+': return 62;
        case '/': return 63;
        case '=': exit(0);
        default: exit(-1);
    }
}

void base64_decode (FILE *in, FILE *out)
{
    uint32_t byte;
    do
    {
        byte = base64_value(in) << 18;
        putc(((byte += base64_value(in) << 12)) >> 16, out);
        putc(((byte += base64_value(in) << 6) >> 8) & 255, out);
        putc((byte += base64_value(in)) & 255, out);
    } while(1);
}

int main (int argc, char const *argv[])
{
    if(argc == 2)
    {
        if(!strcmp(argv[1], "-d"))
            base64_decode(stdin, stdout);
        else
            return -1;
    }
    else
    {
        base64_encode(stdin, stdout);
    }
    return 0;
}