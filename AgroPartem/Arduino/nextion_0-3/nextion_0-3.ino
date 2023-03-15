#include <SoftwareSerial.h>

const byte iPage    = 0x4a;
const byte iButton  = 0x4b;

const byte signM    = 'M';
const byte signP    = 'P';

const byte modeM    = 'M';
const byte modeW    = 'W';
const byte modeD    = 'D';

const byte btA      = 'A';
const byte btB      = 'B';
const byte btW      = 'W';
const byte btP      = 'P';
    
// текущий режим работы, нажатая кнопка и её приращение
byte mode    = ' ';
byte button  = ' ';
byte signBtn = ' ';

bool pause   = false;// режим обновления

int  wprocessing = 128;// ширина захвата орудия, дециметры
bool updateWProcessing = false;// флаг начального обновления

SoftwareSerial nxtSerial(5, 4);//(Ж)RX --> 4 | (С)TX --> 5

String msgNxt = "";

unsigned long t1 = 0;//////////////////////////////////////////
int drv = 20;//////////////////////////////////////////////////
int kdrv = 1;//////////////////////////////////////////////////
int countd = 0;////////////////////////////////////////////////


void toNextionQ(String device, String message) {
  // send string message to Nextion
  nxtSerial.print(device); 
  
  nxtSerial.write(0x22); 
  nxtSerial.print(message); 
  nxtSerial.write(0x22); 
  
  nxtSerial.write(0xff); 
  nxtSerial.write(0xff); 
  nxtSerial.write(0xff);
}
void toNextionQ(String device, int value) {
  // send integer message to Nextion
  toNextionQ(device, String(value));
}

void toNextion(String device, String message) {
  // send string message to Nextion
  nxtSerial.print(device); 
  
  nxtSerial.print(message); 
  
  nxtSerial.write(0xff); 
  nxtSerial.write(0xff); 
  nxtSerial.write(0xff);
}
void toNextion(String device, int value) {
  // send integer message to Nextion
  toNextion(device, String(value));
}

void parser(String msg) {
  Serial.println("Parse ....");
  
  if (msg.length() < 4) {
    return;
  }

  if (msg.length() == 4) {
    Serial.println("msg.length() == 4");
    
    if ((byte)(msg[0]) == iPage) {
      Serial.println("msg[0] == page");
      mode = msg[1];
      return;   
    }
    if ((byte)(msg[0]) == iButton) {
      Serial.println("msg[0] == button");
      button = msg[1];
      return;
    }
    return;
  }
  if (msg.length() == 5) {
    Serial.println("msg.length() == 5");
    if ((byte)(msg[0]) == iButton) {
      Serial.println("msg[0] == button");
      button  = msg[1];
      signBtn = msg[2];
      Serial.print("signBtn = ");
      Serial.println(signBtn);
      return;
    }
    return;
  }
}

void setup() {
  Serial.begin(9600);
  nxtSerial.begin(38400);

  Serial.println("Serial init");
}

void loop() {
  if (nxtSerial.available()>0) {
    for(int i=0; i<nxtSerial.available(); i++) {
      char c = nxtSerial.read();
      Serial.print((byte)c);Serial.print('|');
      msgNxt += c;
      if (c == '\n') {// || c == '\r'
        Serial.println(" ");
        Serial.print(msgNxt);
        Serial.print("   ");
        Serial.println(msgNxt.length());
        parser(msgNxt);
        Serial.println("____________________");
        msgNxt = "";
        return;
      }
    }
  }

  ////////////////////////////
  // обрабатываем состояние //
  ////////////////////////////
  if (mode == modeM) {//<<<<<<<<<<<<<<<<<<<<<<<<< основное меню
    //Serial.println("MODE = M");
    updateWProcessing = false;
    pause = false;
  } else if (mode == modeW) {//<<<<<<<<<<<<<<<<<< режим настройки
    //Serial.println("MODE = W");
    // если страница только открылась, обновляем поле W
    if (!updateWProcessing) {
      toNextionQ("W.txt=", wprocessing);
      updateWProcessing = true;
    }
    // если нажата кнопка
    if (button == btW) {
      if (signBtn == signM) {
        wprocessing--;
      }
      if (signBtn == signP) {
        wprocessing++;
      }
      toNextionQ("W.txt=", wprocessing);
    }
    if (button == btA) {
      //set point A
      Serial.println("BUTTON = A");
      toNextion("vAB.val=", 1);
      toNextion("btAB.pic=", 2);
    }
    if (button == btB) {
      //set point B
      Serial.println("BUTTON = B");
      toNextion("page ", "drive");
    }

    // очищаем кнопки
    button  = ' ';
    signBtn = ' ';
  } else if (mode == modeD) {//<<<<<<<<<<<<<<<<<<< режим вождения
    //Serial.println("MODE = D");
    updateWProcessing = false;
    // если нажата кнопка паузы
    if (button == btP) {
      Serial.println("BUTTON = P");
      pause = !pause;
      if (pause) {//меняем картинку кнопки
        toNextion("p.pic=",34);
        Serial.println("current pause ist true");
      } else {
        toNextion("p.pic=",33);
        Serial.println("current pause ist false");
      }
      button = ' ';// очищаем кнопку
    }
    
    ///////////////////////////////////////////////////
    // основной цикл обновления экрана кусоуказателя //
    ///////////////////////////////////////////////////
    if (!pause) {
//////<<test code>>//////////////////////////////////////
      if (millis() - t1 > 500) {
        t1 = millis();
        drv += kdrv;
        countd++;
        if(drv > 30) {
          drv = 30;
          kdrv = -1;
        }
        if (drv < 10) {
          drv = 10;
          kdrv = 1;
        }
        toNextion("drv.pic=", drv);
        
        int v = 8 + drv/8 - 2;
        toNextionQ("v.txt=", v);

        toNextionQ("d.txt=", countd);
        toNextionQ("s.txt=", 2*countd/9);
      }
/////////////////////////////////////////////////////////////

      
      // update direction
      // update velocity
      // update distance
      // update area
      
      // send direction to Nextion 
      // toNextion("drv.pic=", ..);
      // send velocity  to Nextion
      // toNextionQ("v.txt=", ..);
      // send distance  to Nextion
      // toNextionQ("d.txt=", ..);
      // send area      to Nextion
      // toNextionQ("s.txt=", ..);
    }
  }
}
