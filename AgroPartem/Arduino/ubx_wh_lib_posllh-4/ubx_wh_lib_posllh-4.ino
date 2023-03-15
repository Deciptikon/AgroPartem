#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

SoftwareSerial gpsSerial(5, 4); // RX --> D4 | TX --> D5
const unsigned char UBX_HEADER[] = { 0xB5, 0x62, 0x01, 0x02 };
const int lenhead = 2;


struct NAV_POSLLH {
  unsigned char cls;
  unsigned char id;
  unsigned short len;
  unsigned long iTOW;
  long lon;
  long lat;
  long height;
  long hMSL;
  unsigned long hAcc;
  unsigned long vAcc;
};

NAV_POSLLH posllh;
const int payloadSize = sizeof(NAV_POSLLH); //32 byte

void calcChecksum(unsigned char* CK) {
  memset(CK, 0, 2);
//  CK[0] = 0;
//  CK[1] = 0;
//Serial.println(" ");
  for (int i = 0; i < (int)sizeof(NAV_POSLLH); i++) {
    CK[0] += ((unsigned char*)(&posllh))[i];
    CK[1] += CK[0];
    //Serial.print(CK[0]);Serial.print(" \t");
    //Serial.print(CK[1]);Serial.println(" ");
  }
}

bool processGPS() {
  static int fpos = 0;
  static unsigned char checksum[2] = {0,0};

  while ( gpsSerial.available() )
  {    
    
    Serial.print(gpsSerial.available());Serial.println(" ");
    byte c = gpsSerial.read();
    if ( fpos < lenhead + 2 ) {
      if ( c == UBX_HEADER[fpos] ) {
//        if(fpos > lenhead - 1) {
//          ((unsigned char*)(&posllh))[fpos - lenhead] = c;
//        }
        fpos++;
      } else {
        fpos = 0;
      }
    }
    
    else
    { 
      if ( (fpos - lenhead) < payloadSize ) {
        ((unsigned char*)(&posllh))[fpos - lenhead] = c;
        if((fpos - lenhead) == 2) {
          ((unsigned char*)(&posllh))[0] = 0x01;
          ((unsigned char*)(&posllh))[1] = 0x02;
          //Serial.print("_");Serial.print((int)0x01);Serial.print(" ");
          //Serial.print("_");Serial.print((int)0x02);Serial.print(" ");
        }
        //Serial.print((int)c);Serial.print(" ");
        //Serial.print("_");
      }

      fpos++;
      //Serial.print((int)c);Serial.print(" ");
      
      if ( fpos == (payloadSize + lenhead) )
      {
        calcChecksum(checksum);
      }
      else if ( fpos == (payloadSize + lenhead + 1) )
      {
        if ( c != checksum[0] ) {
          Serial.print(fpos);Serial.println(" ");
          fpos = 0;
          //Serial.println("!checksum[0]");
          }
          //Serial.print("c = ");Serial.print((int)c);
          //Serial.print("\tCK = ");Serial.println((int)(checksum[0]));
        
      }
      else if ( fpos == (payloadSize + lenhead + 2) ) {
        fpos = 0;
        //Serial.println("checksum[1]");
        //Serial.print("c = ");Serial.print((int)c);
        //Serial.print("\tCK = ");Serial.println((int)(checksum[1]));
        if ( c == checksum[1] ) {
          return true;
        }
      }
      else if ( fpos > (payloadSize + lenhead + 2) )
      {
        fpos = 0;
        //Serial.println("");
      }
    }
  }
  return false;
}

void setup()
{
  Serial.begin(9600);
  gpsSerial.begin(38400);//115200//

  Serial.print("Serial init   ");
  Serial.println((int)sizeof(NAV_POSLLH));
}

void loop() {
  if ( processGPS() ) {
    Serial.print(posllh.lon / 10000000.0, 7);
    Serial.print(',');
    Serial.print(posllh.lat / 10000000.0, 7);
//    Serial.print(',');
//    Serial.print(posllh.height / 1000.0, 3);
//    Serial.print(',');
//    Serial.print(posllh.hMSL / 1000.0, 3);
//    Serial.print(',');
//    Serial.print(posllh.hAcc / 1000.0, 3);
//    Serial.print(',');
//    Serial.print(posllh.vAcc / 1000.0, 3);
    Serial.println();
  }
  delay(100);
  //Serial.println("...");
}
