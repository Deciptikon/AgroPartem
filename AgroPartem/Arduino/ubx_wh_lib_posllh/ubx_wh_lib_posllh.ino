#include <SoftwareSerial.h>

SoftwareSerial gpsSerial(5, 4); // RX --> D4 | TX --> D5
const unsigned char UBX_HEADER[] = { 0xB5, 0x62 };

//type data yang menentukan panjang data
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
  for (int i = 0; i < (int)sizeof(NAV_POSLLH); i++) {
    CK[0] += ((unsigned char*)(&posllh))[i];
    CK[1] += CK[0];
  }
}

bool processGPS() {
  static int fpos = 0;
  static unsigned char checksum[2];
 
  while ( gpsSerial.available() > 0 )
  {
    
    byte c = gpsSerial.read();
    if ( fpos < 2 ) {
      if ( c == UBX_HEADER[fpos] ) {// Check Header apakah sesuai
        fpos++;
      } else {
        fpos = 0;
      }
    }
    
    else
    {
      if ( (fpos - 2) < payloadSize ) {
        ((unsigned char*)(&posllh))[fpos - 2] = c;
      }

      fpos++;

      if ( fpos == (payloadSize + 2) ) //CheckSum check
      {
        calcChecksum(checksum);
      }
      else if ( fpos == (payloadSize + 3) )
      {
        if ( c != checksum[0] )
          fpos = 0;
      }
      else if ( fpos == (payloadSize + 4) ) {
        fpos = 0;
        if ( c == checksum[1] ) {
          return true;
        }
      }
      else if ( fpos > (payloadSize + 4) )
      {
        fpos = 0;
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
    Serial.print(posllh.cls, 7);
    Serial.print(',');
    Serial.print(posllh.id, 7);
    Serial.print(',');
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
  delay(10);
  //Serial.println("...");
}
