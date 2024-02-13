#include <Adafruit_TinyUSB.h>
#include "FAT_Utilities.h"
#include <LittleFS.h>

#define SectorCount  32*1024 // Produces a 16 megabyte virtual file. In truth, only relevant data is filled

#define PartitionAreaLength ((DefaultMaxRootEntries * 32) + (DefaultSectorSize * DefaultReservedSectors) + (DefaultSectorSize * DefaultSectorsPerFat)) - (2*DefaultSectorSize)
#define FileStartMappingsLength DefaultMaxRootEntries

#define maxPathLength 

char PartitionArea[PartitionAreaLength];
uint32_t FileStartMappings[FileStartMappingsLength][2];
char* FileNameMappings[FileStartMappingsLength];

Adafruit_USBD_MSC usb_msc;


void rebuildFatTable(){
  //Zero the disk
  //Serial.println("Zeroing Disk");
  for (uint32_t index = 0; index < PartitionAreaLength; index++){
    //Serial.printf("%i\n",index);
    PartitionArea[index] = 0;
  }
  //Serial.println("Zero Disk");

  //Clear file mappings
  for (uint32_t index = 0; index < FileStartMappingsLength; index++){
    FileStartMappings[index][0] = 0;
    FileStartMappings[index][1] = 0;
    
    if (FileNameMappings[index] != NULL){
      //Serial.printf("Freeing Index %i\n",index);
      free(FileNameMappings[index]);
      FileNameMappings[index] = NULL;
    }
  }
  //Serial.println("Clear Mappings");


  //Create the partition
  formatDriveF16(PartitionArea, SectorCount, "NO NAME");
  //Serial.println("Format Partition");


  //For all local files, add them as entries
  Dir dir = LittleFS.openDir("/");
  uint32_t position;
  char FATname[9];
  char FATextension[4];
  int index;
  int dotIndex;
  char workingChar;

  memset(FATname,0,9);
  memset(FATextension,0,4);

  int fileCounter = 0;
  while (dir.next()) {
    if (dir.isFile()){
      //Serial.printf("Register file %s to FAT Table\n",dir.fileName().c_str());
      
      //Extract the Extension and Name
      //First clear
      memset(FATname,' ',8);
      memset(FATextension,' ',3);

      //Then copy the Name
      index = 0;
      while (1){
        if (index > dir.fileName().length()){
          break;
        }
        if (index == 8){
          break;
        }
        
        workingChar = dir.fileName().charAt(index);
        if (workingChar == '.'){
          break;
        }
        else {
          FATname[index] = workingChar;
        }
        index += 1;
      }
      //Serial.printf("Got Name %s\n",FATname);

      //Locate the Dot
      dotIndex = 0;
      for (index = 0; index < dir.fileName().length(); index++){
        workingChar = dir.fileName().charAt(index);
        if (workingChar == '.'){
          dotIndex = index;
          break;
        }
      }

      //Copy the Extension
      index = 0;
      while (1){
        if (index + dotIndex + 1 < dir.fileName().length()){
          if (index < 3){
            FATextension[index] = dir.fileName().charAt(index + dotIndex + 1);
          }
          else {
            break;
          }
        }
        else {
          break;
        }
        index += 1;
      }
      //Serial.printf("Got Extension %s\n",FATextension);

      position = insertFileEntry(FATname,FATextension,dir.fileSize(),PartitionArea);
      FileStartMappings[fileCounter][0] = position;
      FileStartMappings[fileCounter][1] = position + dir.fileSize();

      char* newName = (char*)malloc(sizeof(char)*(dir.fileName().length() + 1));
      memset(newName,0,sizeof(char)*(dir.fileName().length() + 1));
      _cpyTo((char*)dir.fileName().c_str(),newName,dir.fileName().length(),0,0);
      FileNameMappings[fileCounter] = newName;
      //Serial.printf("Mapping %ul %ul %s\n",position, position + dir.fileSize(), newName);

      fileCounter += 1;
    }
  }

}

int32_t expose_writeCallback(uint32_t lba, uint8_t* buffer, uint32_t bufsize){
  //Serial.printf("Error, write Partition block %i\n", lba);
  ; //Do nothing
  return 0;
}

//Input: logical block address (block number), buffer, 
//Returns number of read bytes as multiple of Sector size
int32_t expose_readCallback(uint32_t lba, void* buffer, uint32_t bufsize){
  
  uint32_t wantsBytesFrom = lba * DefaultSectorSize;
  int32_t readCount = 0;
  //If the lba is within the partition area, we return data from RAM
  if (wantsBytesFrom < PartitionAreaLength){
    //Serial.printf("Read Partition block %i\n", lba);
    _cpyTo(PartitionArea,(char*)buffer,DefaultSectorSize,wantsBytesFrom,0);
    return bufsize;
  }

  //Otherwise we scan for the file they want
  for (int index = 0; index < FileStartMappingsLength; index++){
    if (((FileStartMappings[index][0] <= wantsBytesFrom) && (FileStartMappings[index][1] > wantsBytesFrom)) && (FileNameMappings[index] != NULL)){
      //Serial.printf("Read File %s block %i\n", FileNameMappings[index] , lba);
      //Determine the data from the File
      File myFile = LittleFS.open(FileNameMappings[index], "r");
      myFile.seek(wantsBytesFrom - FileStartMappings[index][0]);
      myFile.readBytes((char*)buffer,bufsize);
      myFile.close();
      return bufsize;
    }
  }
  

  //Otherwise, just return zeros
  //Serial.printf("Read Empty block %i\n", lba);
  memset(buffer,0,bufsize);
  return bufsize;
}


void expose_flushCallback(){
  //Serial.println("Flush");
  ; //Do nothing
}

bool expose_writableCallback(){
  return 0; // Not Writeable
}





void initExposedFS(){
  //Clear the Tables
  for (uint32_t index = 0; index < FileStartMappingsLength; index++){
    FileStartMappings[index][0] = 0;
    FileStartMappings[index][1] = 0;
    
    FileNameMappings[index] = NULL;
    
  }
  //Serial.println("Clear Mappings");

  //Vendor ID, Product ID and Version
  usb_msc.setID("Adafruit", "Mass Storage", "1.0");
  //Serial.println("Set ID");

  //The Capacity
  usb_msc.setCapacity(SectorCount, DefaultSectorSize);
  //Serial.println("Set Capacity");

  //Set callback when read / write / flush called
  usb_msc.setReadWriteCallback(expose_readCallback, expose_writeCallback, expose_flushCallback);
  //Serial.println("Set Callbacks");
  usb_msc.setWritableCallback(expose_writableCallback);
  

  //Rebuild the FAT table
  rebuildFatTable();
  //Serial.println("Built Fat Table!\n");

  //Set the disk to ready
  usb_msc.setUnitReady(true);

  //Begin
  usb_msc.begin();
  //Serial.println("USB Ready");
}

//Used to signal device status
void setReadyState(bool state){
  usb_msc.setUnitReady(state);
}