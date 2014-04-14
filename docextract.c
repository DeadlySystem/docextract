#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFFER_SIZE 8192
/*
    Use this macro if your compiler does not have min available:
    #define min(x,y) ((x < y) ? (x) : (y))
*/

int main(int argc, char* argv[])
{
    mkdir("PNGs");
    unsigned char PNGHeader[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char DOC_header[4] = {'D', 'O', 'C', ' '};
    unsigned int  DOC_version   = 0x00010000;
    //Open the input file
    FILE* inputfile = fopen("DOCUMENT_DEC.DAT", "rb");
    if(inputfile == NULL)
    {
        printf("Could not open input file for reading.\n");
        return 1;
    }
    
    //Read signature
    unsigned char header[4];
    fread(header, 1, sizeof(DOC_header), inputfile);
    if(memcmp(header, DOC_header, sizeof(DOC_header)) != 0)
    {
        printf("Input file is not a DOC file. Operation aborted.");
        return 2;
    }
    
    //Read versions
    unsigned int version1;
    unsigned int version2;
    fread(&version1, 1, sizeof(DOC_version), inputfile);
    fread(&version2, 1, sizeof(DOC_version), inputfile);
    if(version1 != version2 || version1 != DOC_version)
    {
        printf("Version not supported! %08X", version1);
        return 3;
    }
    
    //Read game code
    unsigned char gamecode[16];
    fread(gamecode, 1, sizeof(gamecode), inputfile);
    printf("Game: %s\n", gamecode);
    
    //Skip unknown portion
    fseek(inputfile, 0x6C, SEEK_CUR);
    
    int i = 1;
    unsigned char* buffer = (unsigned char*) malloc(BUFFER_SIZE);
    if(buffer == NULL)
    {
        printf("Out of memory!");
        fclose(inputfile);
        return 4;
    }
    while(true)
    {
        unsigned int offset;
        unsigned int size;
        fread(&offset, 1, sizeof(offset), inputfile);
        fseek(inputfile, 8, SEEK_CUR);
        fread(&size, 1, sizeof(offset), inputfile);
        if(offset == 0)
        {
            break;
        }
//        printf("Offset: %08X\n", offset);
        fseek(inputfile, 0x70, SEEK_CUR);
        long nextentry = ftell(inputfile);
        unsigned char* filename = "PNGs/XXXXX.png";
        FILE* outputfile;
        sprintf(filename, "PNGs/%05i.png", i);
        printf("Writing %s\n", filename);
        offset += 0x28;
        size -= 0x28;
        fseek(inputfile, offset, SEEK_SET);
        outputfile = fopen(filename, "wb");
        if(outputfile == NULL)
        {
            printf("Could not open output file for writing!");
            return 5;
        }
        fwrite(PNGHeader, 1, sizeof(PNGHeader), outputfile);
        int bytesremaining = size;
//        int skip = -0x60;
//        fseek(inputfile, skip, SEEK_CUR);
//        bytesremaining -= skip;
        while(bytesremaining > 0)
        {
            int bytesread = fread(buffer, 1, min(BUFFER_SIZE, bytesremaining), inputfile);
            fwrite(buffer, 1, bytesread, outputfile); 
            bytesremaining -= bytesread;
            if(bytesread == 0)
            {
                break; //TODO: Ensures termination.
            }
        }
        fclose(outputfile);
        
        fseek(inputfile, nextentry, SEEK_SET);
        ++i;
    }
    
    
    
    
    //Close the file
    fclose(inputfile);
    
    printf("DOC extracted!\n");
    return 0;
}




/*
File header:
    header {'D', 'O', 'C', ' '}
    int unknown 00 00 01 00
    int unknown 00 00 01 00
    gamecode (9 bytes)/16 bytes
    ??? imagelimit (whether >= 100 images)
    0x60 bytes: Unknown
    int unknown {0xFF, 0xFF, 0xFF, 0xFF)
    int # images



Entries start at 0x88.
1 Entry = 0x80 bytes:
    int offset_low (little endian)
    int offset_high (little endian) usually 0
    int unknown
    int size_low (little endian)
    int size_high (little endian) usually 0
    0x6C bytes: Unknown
*/
