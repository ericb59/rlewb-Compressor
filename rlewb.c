/* ________________________________________________________________________________
                                                                           
   RLE-WB Compressor V1.0
   Eric Boez (c) March 2019
   Tool developped for FUSION-C library. But feel free to use for other purposes

This tool will compress any data with RLE-WB algorythm. It's based on classic RLE compression but with the use of a Control Code
RLE WB Format compression : 
control-Code     Number-of-repetition     data to repeat
    0x80               nn                       dd             ; run of n consecutive identical bytes (0x1..0xFA), value dd
    0x80               0x0                                     ; will use 0x80 as data
    0x80               0xFF                                    ; end of data block
    any other value                                            ; raw data used   


 More details about this RLE Compression : http://www.smspower.org/Development/Index
 Decompressions routines for ASM and C at AORANTE's website : http://aorante.blogspot.com/2014/06/compresion-rle-sms-wonder-boy.html

Command line, Usage of this tool :

./rlewb <input file> <outputfile> <offset> <export mode>

input file  : is the file name of any file type to read
output file : is the file name you desire for the final file
offset      : is the start point where compressor will start encoding data. it will leave all data before this sart point
                It usefull to bypass header file. For example if you want to encode a MSX image, the file have a 7 bytes header it is not necessary to encode.
export mode : Choose the export encoding file. 
                If '1' compressor will export datas in a text file formated for ASM include
                If '2' compressor will export datas in a text file formated for C include
                If '3' compressor will export datas as a binary file


RLE-WB compressor is free software; it comes without any warranty. 
You can use, modify, share it under the terms of Creative Commons CC BY_SA 4.0 license.

You are free to :
Share : Copy and redistribute the material in any medium or format
Adapt : remix, transform, and build upon the material for any purpose, even commercially.
The licensor cannot revoke these freedoms as long as you follow the license terms.

You must give appropriate credit, provide a link to the license, and indicate if changes were made.
You may do so in any reasonable manner, but not in any way that suggest the licensor endorse you or your use.
You must provide : the name of the creator and attribution parties, the title of the material, a copyright notice a license notice, 
a disclaimer notice and a link to the material.
If you remix, transform, or build upon the material you must distribute your contributions under the same license as the original.
____________________________________________________________________________________*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char cpt;
char format;
char NumCar;
char flag;

///////// C O N F I G 

unsigned char ControlCode=0x80;  // Control Code. 
char AsmTxt[4]=".db ";           // used at the begining of each line in ASM export mode (o more than 4 chars)
char Numeric='#';               // Character behind each digital number 



// Check file Lenght
long filelength(char *f)
{
    FILE *t = fopen(f, "rb");
    fseek(t, 0, SEEK_END);
    int x = ftell(t);
    fclose(t);
    return x;
}

// Write data to dest file
void WriteToFile(unsigned char mydata, FILE *handler)
{

    unsigned char *putdata = (unsigned char *)malloc(3);

    putdata[0]=mydata;
    fwrite(putdata, 1, 1, handler);
    free(putdata);
    flag=1;
}

// Write Data to dest file in ASM / C mode in hexa format
void WriteToFileASM(unsigned char mydata, FILE *handler)
{

     char putdata[9];

    sprintf(putdata, "0x%02x ", mydata);
    fwrite(putdata, 1, 4, handler);
    flag=1;
}

// Check for no more 16 elements by line in ASM and C format
void checkline(FILE *handler)
{
                if (cpt==0 || cpt>=16)
                {
                    if (format==2)
                    {
                        WriteToFile(',', handler);
                    }
                    WriteToFile('\n', handler);
                    if (format==1)
                    {
                        WriteToFile('.', handler);
                        WriteToFile('d', handler);
                        WriteToFile('b', handler);
                        WriteToFile(' ', handler);
                    }
                    cpt=0;
                }
}

/// MAIN
int main(int argc, char *argv[])
{ 
    

    long f;
    int offset=0;
    int r=0,p=0,e=0;
    uint64_t z=0;

    printf("\n\nFUSION-C :: RLE-WB Compressor :: V1.0 (March 2019 by Eric Boez)\n");
    printf("_________________________________________________________________\n\n");

    if(argc != 5)
    {
        printf("Usage: %s <input file> <output file> <offset> <export mode>\n", argv[0]);
        printf("- input file is the file name to compress\n"); 
        printf("- output file is the name of the compressed file\n");
        printf("- offset is the start byte where to start analysing input file\n"); 
        printf("- mode must be 1,2 or 3\n");
        printf("   1: ASM text format  2:C text Format  3:Binary format\n");
        printf("\n");
        return -1;
    }

    cpt=0;
    flag=0;

    format=atoi(argv[4]);   // 1 : ASM    2 : C    3 BIN
    if (format !=1 && format !=2 && format !=3)
        format=1;

    if (format==1)
        NumCar=Numeric;

    if (format==2)
        NumCar=' ';

    offset=atoi(argv[3]);

    // Open files
    FILE *in = fopen(argv[1], "rb");
    FILE *out_ = fopen(argv[2], "wb");

    
    if(in == NULL)
    {
        printf("ERROR! :: Cannot open File : %s\n", argv[1]); 
        return -1;
    }
    if(out_ == NULL)
    {
        printf("ERROR! :: Cannot open file : %s\n", argv[2]); 
        return -1;
    }

    unsigned char *ind = (unsigned char *)malloc(f= (long)filelength(argv[1]) + 1);

    z = filelength(argv[1]);

    if(ind == NULL)
    {
        printf("ERROR! :: Cannot allocate memory for the input file : %ld KB\n", f/1024);
        return -1;
    }

    p=0;
    // Read the entire origianl file
    while(!feof(in))
    {
        fread(&ind[p++], 1, 1, in);
    }
    fclose(in);

    /// Start  encoding process
    p=0;
    for(r=offset;r<z;r++)
    {
        if(ind[r] == ind[r+1])
        {
            e=0;
            while(ind[r+e] == ind[r])
            {
                e++; 
                if(e > 250)
                {
                        break;
                }
            }
            // Uncheck for verbose
            //printf("Found run at offset (%d-%d): %x [%x]\n", r, r+e, e, ind[r]);
            if (format==1 || format==2)
            {
                if (cpt==0 || cpt>=16)
                {
                    if (format==2 && flag==1)
                    {
                        WriteToFile(',', out_);
                    }
                    WriteToFile('\n', out_);
                    if (format==1)
                    {
                        WriteToFile(AsmTxt[0], out_);
                        WriteToFile(AsmTxt[1], out_);
                        WriteToFile(AsmTxt[2], out_);
                        WriteToFile(AsmTxt[3], out_);
                    }
                    cpt=0;
                }
                cpt++;
                WriteToFile(NumCar, out_);
                WriteToFileASM(ControlCode, out_);
                checkline(out_);
                cpt++;
                if (cpt>1)
                    WriteToFile(',', out_);
                WriteToFile(NumCar, out_);
                WriteToFileASM(e, out_);
                checkline(out_);
                cpt++;
                if (cpt>1)
                WriteToFile(',', out_);
                WriteToFile(NumCar, out_);
                WriteToFileASM(ind[r], out_);
            
                if(cpt<16)
                {
                     WriteToFile(',', out_);
                } 
                else
                {
                    
                    cpt=0;
                }
            }

            if (format==3)
            {
                WriteToFile(ControlCode, out_);
                WriteToFile(e, out_);
                WriteToFile(ind[r], out_);
            }
            r+=(e-1);

        } 
        else 
        {
            if (ind[r]==ControlCode)
            {
                if (format==1 || format==2)
                {
                    if (cpt==0 || cpt>=16)
                    {
                        if (format==2 && flag==1)
                        {
                            WriteToFile(',', out_);
                        }
                        WriteToFile('\n', out_);
                        if (format==1)
                        {
                            WriteToFile(AsmTxt[0], out_);
                            WriteToFile(AsmTxt[1], out_);
                            WriteToFile(AsmTxt[2], out_);
                            WriteToFile(AsmTxt[3], out_);
                        }
                        cpt=0;
                     }
                     cpt++;
                    WriteToFile(NumCar, out_);
                    WriteToFileASM(ControlCode, out_);
                    cpt++;
                    WriteToFile(',', out_);
                    WriteToFile(NumCar, out_);
                    WriteToFileASM(0, out_);
    
                    
                    if(cpt<16)
                    {
                         WriteToFile(',', out_);
                    } 
                    else
                    {
                        
                        cpt=0;
                    }
                }

                if (format==3)
                {
                    WriteToFile(ControlCode, out_);
                    WriteToFile(0, out_);
                }
            } 
            else
            {
                if (format==1 || format==2)
                {
                    if (cpt==0 || cpt>=16)
                    {
                        if (format==2 && flag==1)
                        {
                            WriteToFile(',', out_);
                        }
                        WriteToFile('\n', out_);
                        if (format==1)
                        {
                            WriteToFile(AsmTxt[0], out_);
                            WriteToFile(AsmTxt[1], out_);
                            WriteToFile(AsmTxt[2], out_);
                            WriteToFile(AsmTxt[3], out_);
                        }
                        cpt=0;
                    }
                    cpt++;
                    WriteToFile(NumCar, out_);
                    WriteToFileASM(ind[r], out_);
                
                    if(cpt<16)
                    {
                         WriteToFile(',', out_);
                    } 
                    else
                    {
                     
                        cpt=0;
                    }
                }

                if (format==3)
                {
                    WriteToFile(ind[r], out_);
                }
            }
            
        }
    }
    if (format==1 || format==2)
    {
        if (cpt==0 || cpt>=16)
        {
            
            WriteToFile('\n', out_);
            if (format==1)
            {
                WriteToFile(AsmTxt[0], out_);
                WriteToFile(AsmTxt[1], out_);
                WriteToFile(AsmTxt[2], out_);
                WriteToFile(AsmTxt[3], out_);
            }
        }
        
        WriteToFile(NumCar, out_);
        WriteToFileASM(ControlCode, out_);
        WriteToFile(',', out_);
        WriteToFile(NumCar, out_);
        WriteToFileASM(255, out_);
        WriteToFile(',', out_);
        WriteToFile(NumCar, out_);
        WriteToFileASM(ControlCode, out_);
        WriteToFile(',', out_);
        WriteToFile(NumCar, out_);
        WriteToFileASM(255, out_);
    }
    if (format==3)
    {
        WriteToFile(ControlCode, out_);
        WriteToFile(0xFF, out_);
        WriteToFile(ControlCode, out_);
        WriteToFile(0xFF, out_);
    }
    fclose(out_);

    printf("\nResult >>>>>>>>>>>>>>> :%s \n",argv[2]);
    if (format==3) // binary
    {
         printf("Original file lenght :%ld bytes \n",(long)filelength(argv[1]));
         printf("Final  BIN  file lenght :%ld bytes \n",(long)filelength(argv[2]));
         printf("offset used: %d\n\n",offset);
    }
    if (format==1) // binary
    {
         printf("Original file lenght :%ld bytes \n",(long)filelength(argv[1]));
         printf("Final  ASM  text file lenght :%ld bytes \n",(long)filelength(argv[2]));
         printf("offset used: %d\n\n",offset);
    }
    if (format==2) // c
    {
         printf("Original file lenght :%ld bytes \n",(long)filelength(argv[1]));
         printf("Final  C  text file lenght :%ld bytes \n",(long)filelength(argv[2]));
         printf("offset used: %d\n",offset);
    }
    printf("job done sir !\n\n");
    
    

return 0;

}
