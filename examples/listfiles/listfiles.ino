/*
  SD card basic file example
 
 This example shows how to create and destroy an SD card file 	
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11, pin 7 on Teensy with audio board
 ** MISO - pin 12
 ** CLK - pin 13, pin 14 on Teensy with audio board
 ** CS - pin 4, pin 10 on Teensy with audio board
 
 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 	 
 */
#include <LittleFS.h>
#include <SPI.h>
#include <SPIMemory.h>



SPIFlash * FLASH = NULL;
FS * LittleFS = NULL;

File root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// Teensy audio board: pin 10
// Teensy 3.5 & 3.6 on-board: BUILTIN_SDCARD
// Wiz820+SD board: pin 4
// Teensy 2.0: pin 0
// Teensy++ 2.0: pin 20
#define FLASH_SS 6

void setup()
{;

  //UNCOMMENT THESE TWO LINES FOR TEENSY AUDIO BOARD:
  //SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
  //SPI.setSCK(14);  // Audio shield has SCK on pin 14  
  //SPI.begin();
  //SPI.setFrequency(500000);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.print("Initializing LittleFS...");
  FLASH = new SPIFlash(FLASH_SS);
  FLASH->begin();
  Serial.printf("ID: %x\n",FLASH->getJEDECID());
  LittleFS = new FS(FSImplPtr(new littlefs_impl::LittleFSImpl(FLASH, FS_PHYS_ADDR, 0, /*FS_PHYS_SIZE,*/ FS_PHYS_PAGE, FS_PHYS_BLOCK, FS_MAX_OPEN_FILES)));
  if (!LittleFS->begin()) {
    Serial.println("initialization failed!");
    return;
  }
  //SPI.setClockDivider(SPI_CLOCK_DIV1);
  Serial.println("initialization done.");

  root = LittleFS->open("/", FILE_READ);
  
  printDirectory(root, 0);
  
  Serial.println("done!");
}

void loop()
{
  // nothing happens after setup finishes.
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
