#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DefaultSectorSize 512
#define DefaultSectorsPerCluster 1
#define DefaultReservedSectors 4
#define DefaultMaxRootEntries 512
#define DefaultSectorsPerFat 128
#define DefaultSectorsPerTrack 1
#define DefaultNumberOfHeads 1
#define DefaultBootSignature 0x29
#define DefaultDriveNumber 0x80


#define EntryFree 0xE5
#define EntryAllFree 0x00

#define AttributeReadOnly 0x01
#define AttributeMediaType 0xF8
#define AttributeCleanMask 0x8000
#define AttributeIsFile 0x20
#define AttributeIsVolName 0x08

#define BootSector 0
#define FATcount 1
#define FATType "FAT16"

#define ArbitraryDate 0x6543
#define ArbitraryTime 0x886D

// FAT is Little Endian!

// Format of Disk:
// [Boot Sector (One Sector)] [FAT Table] [ Root Directory with File Entries ] [ Data ]

/*Boot Sector Format
Start   Len     Data
0       11      Ignored
11      2       Bytes per sector
13      1       Sectors per cluster
14      2       Number of Reserved Sectors
16      1       Number of FATs
17      2       Root Entry Count
19      2       Total Sector Count
21      1       Ignore
22      2       Sectors per FAT
24      2       Sectors per Track
26      2       Number of heads
28      4       Ignore
32      4       Total Sector Count (FAT32) 0 for FAT12/16
36      2       Ignore
38      1       Boot Signature
39      4       Volume ID
43      11      Volume Label
54      8       System Type (FAT12/16)
62      -       Rest of Boot Sector
*/

/* FAT ENTRY: (FAT16)
0   2       Cluster Status (0 = Free, otherwise Marks start of next cluster, FFFF = End of file)
*/


//Data table format
/*
Start   Len     Data
0       8       FileName, filled with spaces (0x32) 
8       3       Extension, filled with spaces (0x32)
11      1       Attributes
12      2       Reserved
14      2       Creation Time
16      2       Creation Date
18      2       Last Access Date
20      2       Ignore
22      2       Last Write Time
24      2       Last Write Date
26      2       First Logical Cluster
28      4       File Size (Bytes)

*/


union convertUInt32 {
    uint32_t Integer;
    char buffer[4];
};

union convertUInt16 {
    uint16_t Integer;
    char buffer[2];
};

union convertUInt8 {
    uint8_t Integer;
    char buffer[1];
};

void _cpyTo(char* src, char* dest, int srcLen, int srcOffset, int destOffset){
    for (int index = 0; index < srcLen; index++){
        dest[index+destOffset] = src[index+srcOffset];
    }
}



void makeFatTable(char* buffer){
    union convertUInt16 uint16Conversion;
    int FAT_Start = DefaultReservedSectors * DefaultSectorSize;
    int FAT_End = FAT_Start + (DefaultSectorsPerFat*DefaultSectorSize);
    //printf("FAT START %i\n",FAT_Start);
    //printf("FAT END %i\n",FAT_Start + (DefaultSectorsPerFat*DefaultSectorSize));
    
    //Clear the FAT Table
    for (int byteIndex = FAT_Start; byteIndex < FAT_End; byteIndex++){
        buffer[byteIndex] = '\x00';    
    }

    //First bytes are always 0xfff8 at byte 2048 (sector 4)
    uint16Conversion.Integer = 0xfff8;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,FAT_Start);

    //Mark the end of the FAT with 0xFFFF
    uint16Conversion.Integer = 0xFFFF;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,FAT_Start+2);
    
}


//Inserts a file entry and returns the byte position of the start of it
uint32_t insertFileEntry(char* Name, char* Extension, uint32_t ContentLength, char* buffer){
    union convertUInt16 uint16Conversion;
    union convertUInt8 uint8Conversion;
    union convertUInt32 uint32Conversion;



    //The Start of the Root Dir
    int ROOT_Start = (DefaultSectorSize * DefaultReservedSectors) + (DefaultSectorSize * DefaultSectorsPerFat);

    //The Data section start
    int DATA_Start = ((DefaultMaxRootEntries * 32) + ROOT_Start) - (2*DefaultSectorSize);

    //Scan the FAT Table to find a free cluster
    int FAT_Start = DefaultReservedSectors * DefaultSectorSize;
    int FAT_End = FAT_Start + (DefaultSectorsPerFat*DefaultSectorSize);
    //printf("Fat Starts %i ends %i\n",FAT_Start,FAT_End);
    

    int byteIndex;
    int startBlock;
    for (byteIndex = 6; byteIndex < FAT_End-FAT_Start; byteIndex += 2){
        _cpyTo(buffer,uint16Conversion.buffer,2,byteIndex+FAT_Start,0);
        //printf("Check Cluster %i (really %i) got %i from %x %x of %x %x\n",byteIndex,byteIndex+FAT_Start,uint16Conversion.Integer,uint16Conversion.buffer[0],uint16Conversion.buffer[1], buffer[byteIndex+FAT_Start], buffer[byteIndex+FAT_Start+1]);

        if (uint16Conversion.Integer == 0){
            startBlock = byteIndex/2;
            break;
        }
    }
    //printf("Found Start Index %i\n",startBlock);


    uint32_t lengthCounter = ContentLength;
    int indexCounter = startBlock + 0;
    while (lengthCounter > DefaultSectorSize){
      uint16Conversion.Integer = indexCounter + 1;
      _cpyTo(uint16Conversion.buffer,buffer,2,0,(indexCounter*2)+FAT_Start);
      indexCounter += 1;
      lengthCounter -= DefaultSectorSize;
    }

    //Claim the last sector
    uint16Conversion.Integer = 0xFFFF;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,(indexCounter*2)+FAT_Start);

    
    //Now the entry in the root dir
    char stringBuffer[8];

    //Locate a free root entry
    int entryBlock = 0;
    for (indexCounter = 0; indexCounter < DefaultMaxRootEntries; indexCounter++){
      if (buffer[ROOT_Start + (indexCounter*32)] == '\x00'){
        entryBlock = indexCounter;
        break;
      }
    }
    
    //The File Name
    memset(stringBuffer, ' ', 8);
    _cpyTo(Name,stringBuffer,strlen(Name),0,0);
    _cpyTo(stringBuffer, buffer, 8, 0, ROOT_Start + (32*entryBlock) + 0);
    //printf("Saved Name at %i\n",ROOT_Start + (32*entryBlock) + 0);

    //The Extension
     memset(stringBuffer, ' ', 3);
    _cpyTo(Extension,stringBuffer,strlen(Extension),0,0);
    _cpyTo(stringBuffer, buffer, 3, 0, ROOT_Start + (32*entryBlock) + 8);
    //printf("Saved Extension at %i\n",ROOT_Start + (32*entryBlock) + 8);

    //The attribute byte
    uint8Conversion.Integer = AttributeIsFile;
    _cpyTo(uint8Conversion.buffer, buffer, 1, 0, ROOT_Start + (32*entryBlock) + 11);
    //printf("Saved Attribute\n");

    //Created Time
    uint16Conversion.Integer = ArbitraryTime;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*entryBlock) + 14);

    //Created Date
    uint16Conversion.Integer = ArbitraryDate;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*entryBlock) + 16);

    //Last Accessed Date
    uint16Conversion.Integer = ArbitraryDate;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*entryBlock) + 18);

    //Written Time
    uint16Conversion.Integer = ArbitraryTime;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*entryBlock) + 22);

    //Written Date
    uint16Conversion.Integer = ArbitraryDate;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*entryBlock) + 24);

    //First Logical Sector
    uint16Conversion.Integer = startBlock;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*entryBlock) + 26);
    //printf("Saved Start Index\n");

    //File Size
    uint32Conversion.Integer = ContentLength;
    _cpyTo(uint32Conversion.buffer,buffer,2,0,ROOT_Start + (32*entryBlock) + 28);
    //printf("Saved Length\n");



    //Now write the data at the specified cluster
    //_cpyTo(Content, buffer, ContentLength, 0, DATA_Start + (startBlock*DefaultSectorSize));
    return DATA_Start + (startBlock*DefaultSectorSize);
}


void addVolumeLabelToTable(char* label, char* buffer){
    union convertUInt16 uint16Conversion;
    union convertUInt8 uint8Conversion;
    union convertUInt32 uint32Conversion;

    //The Start of the Root Dir
    int ROOT_Start = (DefaultSectorSize * DefaultReservedSectors) + (DefaultSectorSize * DefaultSectorsPerFat);

    //Scan the FAT Table to find a free cluster
    int FAT_Start = DefaultReservedSectors * DefaultSectorSize;
    int FAT_End = FAT_Start + (DefaultSectorsPerFat*DefaultSectorSize);
    //printf("Fat Starts %i ends %i\n",FAT_Start,FAT_End);
    
    int startBlock = 2;


    //Now the entry in the root dir
    char stringBuffer[11];
    
    //The root label
    memset(stringBuffer, ' ', 11);
    _cpyTo(label,stringBuffer,strlen(label),0,0);
    _cpyTo(stringBuffer, buffer, 11, 0, ROOT_Start + (32*startBlock) + 0);
    //printf("Saved Name at %i\n",ROOT_Start + (32*startBlock) + 0);

    //The File type
    buffer[ROOT_Start + (32*startBlock) + 11] = AttributeIsVolName;

     //Written Time
    uint16Conversion.Integer = ArbitraryTime;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*startBlock) + 22);

    //Written Date
    uint16Conversion.Integer = ArbitraryDate;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,ROOT_Start + (32*startBlock) + 24);
    

}


void formatDriveF16(char* buffer, int sectorCount, char* label){
    //Clear the First sector
    memset(buffer, 0, DefaultSectorSize);

    //Set some magic stuff inthe MBR
    const char firstEleven[11] = {0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30};
    _cpyTo((char*)firstEleven,buffer,11,0,0);


    //Create the Conversion union
    union convertUInt32 uint32Conversion;
    union convertUInt16 uint16Conversion;
    union convertUInt8 uint8Conversion;

    //Bytes per Sector
    uint16Conversion.Integer = DefaultSectorSize;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,11);

    //Sectors per Cluster
    uint8Conversion.Integer = DefaultSectorsPerCluster;
    _cpyTo(uint8Conversion.buffer,buffer,1,0,13);

    //Number of Reserved Sectors
    uint16Conversion.Integer = DefaultReservedSectors;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,14);

    //Number of FATs
    uint8Conversion.Integer = FATcount;
    _cpyTo(uint8Conversion.buffer,buffer,1,0,16);

    //Maximum number of root directory entries
    uint16Conversion.Integer = DefaultMaxRootEntries;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,17);

    //Total sector count
    uint16Conversion.Integer = sectorCount;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,19);

    //The media type
    uint8Conversion.Integer = AttributeMediaType;
    _cpyTo(uint8Conversion.buffer,buffer,1,0,21);

    //Sectors per FAT
    uint16Conversion.Integer = DefaultSectorsPerFat;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,22);

    //Sectors per Track
    uint16Conversion.Integer = DefaultSectorsPerFat;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,24);

    //Number of Heads
    uint16Conversion.Integer = DefaultNumberOfHeads;
    _cpyTo(uint16Conversion.buffer,buffer,2,0,26);

    //The Drive Number
    uint8Conversion.Integer = DefaultDriveNumber;
    _cpyTo(uint8Conversion.buffer,buffer,1,0,37);


    //The Boot Signature
    uint8Conversion.Integer = DefaultBootSignature;
    _cpyTo(uint8Conversion.buffer,buffer,1,0,38);

    //The Volume ID
    //Generate on the Fly by hashing the Label
    uint32_t volumeID = 0;
    const unsigned long long int FNV_prime = 1099511628211L;
    for (int index = 0; index < strlen(label); index++){
        volumeID = volumeID * FNV_prime;
        volumeID = volumeID ^ label[index];
    }
    uint32Conversion.Integer = volumeID;
    _cpyTo(uint32Conversion.buffer,buffer,4,0,39);

    //The Label
    char tmpLabel[11];
    memset(tmpLabel,' ',11);
    _cpyTo(label,tmpLabel,strlen(label),0,0);
    _cpyTo(tmpLabel,buffer,11,0,43);

    //The Partition Type
    memset(tmpLabel,' ',8);
    _cpyTo(FATType,tmpLabel,strlen(FATType),0,0);
    _cpyTo(tmpLabel,buffer,8,0,54);  

    //The Boot signature
    buffer[510] = 0x55;
    buffer[511] = 0xAA;


    //Format it
    makeFatTable(buffer);

    //Add the label
    addVolumeLabelToTable(label, buffer);


}