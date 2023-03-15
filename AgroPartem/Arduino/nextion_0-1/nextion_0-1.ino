#include <SoftwareSerial.h>

// текущий режим работы, нажатая кнопка и её приращение
char mode    = ' ';
char button  = ' ';
char signBtn = ' ';

bool pause   = false;// режим обновления

int  wprocessing = 100;// ширина захвата орудия, дециметры
bool updateWProcessing = false;// флаг начального обновления

SoftwareSerial nxtSerial(5, 4);

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

void setup() {
  Serial.begin(9600);
  nxtSerial.begin(38400);

  Serial.println("Serial init");
}

void loop() {
  if (nxtSerial.available()>0) {
    for(int i=0; i<nxtSerial.available(); i++) {
      byte c = nxtSerial.read();
      if (c == '\n' || c == '\r') {
        Serial.println('-');
        return;
      }
      Serial.print(c);
      Serial.print('|');
    }
  }

  ///////////////////
  // PARSE MESSAGE //
  ///////////////////
  // to do

  ////////////////////////////
  // обрабатываем состояние //
  ////////////////////////////
  if (mode == 'M') {//<<<<<<<<<<<<<<<<<<<<<<<<< основное меню
    updateWProcessing = false;
  } else if (mode == 'W') {//<<<<<<<<<<<<<<<<<< режим настройки
    // если страница только открылась, обновляем поле W
    if (!updateWProcessing) {
      toNextion("W.txt=", wprocessing);
      updateWProcessing = true;
    }
    // если нажата кнопка
    if (button == 'W') {
      if (signBtn == '-') {
        wprocessing--;
      }
      if (signBtn == '+') {
        wprocessing++;
      }
      toNextion("W.txt=", wprocessing);
    }
    if (button == 'A') {
      //set point A
      toNextion("vAB.val=", 1);
      toNextion("btAB.pic=", 2);
    }
    if (button == 'B') {
      //set point B
      toNextion("page ", "drive");
    }

    // очищаем кнопки
    button  = ' ';
    signBtn = ' ';
  } else if (mode == 'D') {//<<<<<<<<<<<<<<<<<<< режим вождения
    updateWProcessing = false;
    // если нажата кнопка паузы
    if (button == 'P') {
      pause = !pause;
      if (pause) {//меняем картинку кнопки
        toNextion("p.pic=",33);
      } else {
        toNextion("p.pic=",34);
      }
      button = ' ';// очищаем кнопку
    }
    
    ///////////////////////////////////////////////////
    // основной цикл обновления экрана кусоуказателя //
    ///////////////////////////////////////////////////
    if (!pause) {
      // update direction
      // update velocity
      // update distance
      // update area
      
      // send direction to Nextion 
      // send velocity  to Nextion
      // send distance  to Nextion
      // send area      to Nextion
    }
  }
}
