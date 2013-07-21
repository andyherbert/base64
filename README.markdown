[This article original appeared on andyh.org.](http://andyh.org/Base64.html)

I first became aware of Base64 when I started exchanging messages on Fidonet in the mid-90s, at least I think I did, I only remember seeing a strange variety of letters and numbers that would somehow represent an 8-bit file in a message composed of 7-bit ASCII characters. For a long time I was completely ignorant on how a file could be encoded into Base64 and decoded back again. Then one day, I needed to squeeze a sequence of bytes into a JSON object, and then I wrote a quick and dirty implementation in Javascript. Some time later, as an exercise in C programming, I wanted to see how simple I could make the code by using pointers and data types.

Base64 works simply by combining three 8-bit bytes together to form a 24-bit byte, 24 being the lowest common multiple of both 8 and 6. This conflated byte is then split into four 6-bit bytes. In order to represent these smaller bytes in a string of text, the characters A-Z, a-z, 0-9, +, and /, are conventionally used. If the number of bytes being encoded into Base64 isn't exactly divisible by three, and is missing either one or two, then additional '=' characters are added to the encoded string to represent each missing byte. The number of bytes in the final product is therefore always precisely divisible by four. It's worth mentioning that as a consequence of representing 6-bit data back as an 8-bit byte, an extra third is added to the final length of the encoded data.

The implementation that follows is quite simplistic, and similar to the base64 command found on common \*nix distributions, reads from stdin and outputs to stdout. Similarly a single argument, -d, is used to switch from the default setting of encode, to decode. There's a little pointer magic used when looking up the 6-bit representation, but when decoding, a combination of switch and case commands are used to find the required 8-bit value. This doesn't appear to be very elegant, and I could have used nested if statements instead, but I find the current solution straight-forward, readable, and more likely to compile-down to clean and efficient code. Fortunately most of the work is done with bit shifting and masking, which is something computers can perform very quickly.

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
