// Simple speed test for filesystem objects
// Released to the public domain by Earle F. Philhower, III


#include <FastCRC.h>

FastCRC32 CRC32;

#include <LittleFS.h>

#define TESTSIZEKB 512
#define TESTBUF 512
#define NUMBUFSKB 2
#define FLASH_SS 6
SPIFlash * FLASH = NULL;
FS * LittleFS = NULL;

void DoTest(FS *fs) {
  Serial.println("Formatting");
  if (!fs->format()) {
    Serial.printf("Unable to format(), aborting\n");
    return;
  }
  if (!fs->begin()) {
    Serial.printf("Unable to begin(), aborting\n");
    return;
  }

  uint8_t data[TESTBUF +1];
  for (int i = 0; i < TESTBUF; i++) {
    data[i] = (uint8_t) i;
  }

  Serial.printf("Creating %dKB file, may take a while...\n", TESTSIZEKB);
  long start = millis();
  File f = fs->open("/testwrite.bin", "w");
  if (!f) {
    Serial.printf("Unable to open file for writing, aborting\n");
    return;
  }
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < NUMBUFSKB; j++) {
      uint16_t sect = i*NUMBUFSKB + j;
      data[1] = sect >> 8;
      data[2] = sect & 0xff;
      uint32_t crc = CRC32.crc32(data+1, TESTBUF -5);
      memcpy(data + (TESTBUF -4), &crc, 4);
      f.write(data, TESTBUF);
      //Serial.printf("Sect %d CRC %lx\n", sect, crc);
    }
  }
  f.close();
  long stop = millis();
  Serial.printf("==> Time to write %dKB in %db chunks = %ld milliseconds\n", TESTSIZEKB, TESTBUF, stop - start);

  f = fs->open("/testwrite.bin", "r");
  Serial.printf("==> Created file size = %d\n", f.size());
  f.close();

  Serial.printf("Reading %dKB file sequentially in %db chunks\n", TESTSIZEKB, TESTBUF);
  start = millis();
  f = fs->open("/testwrite.bin", "r");
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < NUMBUFSKB; j++) {
       memset(data, 0, TESTBUF+1);
      f.read(data, TESTBUF);
      uint16_t sect = i*NUMBUFSKB + j;
      uint16_t gotsect = (data[1]<<8) | (data[2] & 0xff);
      uint32_t gotcrc;
      uint32_t crc =  CRC32.crc32(data+1, TESTBUF -5);
      memcpy(&gotcrc, data + (TESTBUF -4), 4);
      
      if (sect != gotsect) {
        Serial.printf ("Error sect: %d %d\n", sect, gotsect);
      }
      if(gotcrc != crc) {
         Serial.printf ("CRC Error sect: %lx %lx\n", crc, gotcrc);
      }
    }
  }
  f.close();
  stop = millis();
  Serial.printf("==> Time to read %dKB sequentially in %db chunks = %ld milliseconds = %ld bytes/s\n", TESTSIZEKB, TESTBUF, stop - start, TESTSIZEKB * 1024 / (stop - start) * 1000);

  Serial.printf("Reading %dKB file MISALIGNED in flash and RAM sequentially in %db chunks\n", TESTSIZEKB, TESTBUF);
  start = millis();
  f = fs->open("/testwrite.bin", "r");
  f.read();
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < NUMBUFSKB; j++) {
      memset(data, 0, TESTBUF+1);
      f.read(data + 1, TESTBUF);
      uint16_t sect = i*NUMBUFSKB + j;
      uint16_t gotsect = (data[1]<<8) | (data[2] & 0xff);
      uint32_t gotcrc = 0;
      uint32_t crc =  CRC32.crc32(data+1, TESTBUF -5);
      memcpy(&gotcrc, data + (TESTBUF -4), 4);
      
      if (sect != gotsect) {
        Serial.printf ("Error sect: %d %d\n", sect, gotsect);
      }
      if(gotcrc != crc) {
         Serial.printf ("CRC Error sect: %lx %lx\n", crc, gotcrc);
      }
    }
  }
  f.close();
  stop = millis();
  Serial.printf("==> Time to read %dKB sequentially MISALIGNED in flash and RAM in %db chunks = %ld milliseconds = %ld bytes/s\n", TESTSIZEKB, TESTBUF, stop - start, TESTSIZEKB * 1024 / (stop - start) * 1000);


  Serial.printf("Reading %dKB file in reverse by %db chunks\n", TESTSIZEKB, TESTBUF);
  start = millis();
  f = fs->open("/testwrite.bin", "r");
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < NUMBUFSKB; j++) {
      uint32_t pos = TESTBUF + (TESTBUF * i*NUMBUFSKB)  + (j*TESTBUF);
      //Serial.printf("pos %ld\n", pos);
      if (!f.seek(pos, SeekEnd)) {
        Serial.printf("Unable to seek to %d, aborting\n", -TESTBUF - (TESTBUF * j * i));
        return;
      }
      memset(data, 0, TESTBUF+1);
      if (TESTBUF != f.read(data, TESTBUF)) {
        Serial.printf("Unable to read %d bytes, aborting\n", TESTBUF);
        return;
      }
      uint16_t sect = (TESTSIZEKB * NUMBUFSKB -1) - (i*NUMBUFSKB + j);
      uint16_t gotsect = (data[1]<<8) | (data[2] & 0xff);
      uint32_t gotcrc = 0;
      uint32_t crc =  CRC32.crc32(data+1, TESTBUF -5);
      memcpy(&gotcrc, data + (TESTBUF -4), 4);
      
      if (sect != gotsect) {
        Serial.printf ("Error sect: %d %d\n", sect, gotsect);
      }
      if(gotcrc != crc) {
         Serial.printf ("CRC Error sect: %lx %lx\n", crc, gotcrc);
      }
    }
  }
  f.close();
  stop = millis();
  Serial.printf("==> Time to read %dKB in reverse in %db chunks = %ld milliseconds = %ld bytes/s\n", TESTSIZEKB, TESTBUF, stop - start, TESTSIZEKB * 1024 / (stop - start) * 1000);


  Serial.printf("Writing 64K file in 1-byte chunks\n");
  start = millis();
  f = fs->open("/test1b.bin", "w");
  for (int i = 0; i < 65536; i++) {
    f.write((uint8_t*)&i, 1);
  }
  f.close();
  stop = millis();
  Serial.printf("==> Time to write 64KB in 1b chunks = %ld milliseconds = %ld bytes/s\n", stop - start, 65536 / (stop - start) * 1000);

  Serial.printf("Reading 64K file in 1-byte chunks\n");
  start = millis();
  f = fs->open("/test1b.bin", "r");
  for (int i = 0; i < 65536; i++) {
    char c;
    f.read((uint8_t*)&c, 1);
  }
  f.close();
  stop = millis();
  Serial.printf("==> Time to read 64KB in 1b chunks = %ld milliseconds = %ld bytes/s\n", stop - start, 65536 / (stop - start) * 1000);


}

void setup() {

  Serial.begin(115200);
  Serial.printf("Set Pin Mode\n");
  
  FLASH = new SPIFlash(FLASH_SS);
  FLASH->begin();
  delay(1000);
  LittleFS = new FS(FSImplPtr(new littlefs_impl::LittleFSImpl(FLASH, FS_PHYS_ADDR, 0, /*FS_PHYS_SIZE,*/ FS_PHYS_PAGE, FS_PHYS_BLOCK, FS_MAX_OPEN_FILES)));
  Serial.printf("Beginning LittleFS test\n");
  Serial.flush();
  DoTest(LittleFS);
}

void loop() {
  Serial.println("Done!");
  delay(10000);
}
