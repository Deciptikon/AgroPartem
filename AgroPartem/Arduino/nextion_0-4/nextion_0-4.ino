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

class Point {
public:
  Point(): x(0), y(0) {
    }
  Point(long xVal, long yVal) {
    x = xVal;
    y = yVal;
  }
//  Point(const Point &pnt)
//  {      
//    x = pnt.getX();
//    y = pnt.getY();
//  }
  long getX() {
    return x;
  }
  long getY() {
    return y;
  }
  void setX(long xVal) {
    x = xVal;
  }
  void setY(long yVal) {
    y = yVal;
  }
  
  long squareLenght() {
    return (x*x + y*y);
  }
  long simpleLength() {
    return sqrt(squareLenght());
  }
  void normal() {
    if (simpleLength() == 0) {
      return;
    }
    x /= simpleLength();
    y /= simpleLength();
  }
  Point normalized() {
    if (simpleLength() == 0) {
      return Point();
    }
    return Point(x/simpleLength(), y/simpleLength());
  }
  
  static long dotProduct(Point p1, Point p2) {
    return (p1.getX() * p2.getX() + p1.getY() * p2.getY());
  }
  
  friend const Point operator-(const Point& pnt);
  friend const Point operator+(const Point& left, const Point& right);
  friend const Point operator-(const Point& left, const Point& right);
private:  
  long x;  
  long y;
};

const Point operator-(const Point& pnt) {
  return Point( -pnt.x, -pnt.y);
}
const Point operator+(const Point& left, const Point& right) {
  return Point( left.x + right.x, left.y + right.y);
}
const Point operator-(const Point& left, const Point& right) {
  return Point( left.x - right.x, left.y - right.y);
}

Point xyCurr, xyPre, pA, pB;

unsigned long maxTimeMs = 500;

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
  if (msg.length() < 4) {
    return;
  }

  if (msg.length() == 4) {
    if ((byte)(msg[0]) == iPage) {
      mode = msg[1];
      return;   
    }
    if ((byte)(msg[0]) == iButton) {
      button = msg[1];
      return;
    }
    return;
  }
  if (msg.length() == 5) {
    if ((byte)(msg[0]) == iButton) {
      button  = msg[1];
      signBtn = msg[2];
      return;
    }
    return;
  }
}

void setup() {
  nxtSerial.begin(38400);
}

void loop() {
  if (nxtSerial.available()>0) {
    for(int i=0; i<nxtSerial.available(); i++) {
      char c = nxtSerial.read();
      msgNxt += c;
      if (c == '\n') {// || c == '\r'
        parser(msgNxt);
        msgNxt = "";
        return;
      }
    }
  }

  /////////////////////////////////
  // Обновление положения по GPS //
  /////////////////////////////////
  //if (gps.available() > 0) {
  //  //xyPre = xyCurr;
  //  //xyCurr = Point(gps.x(), gps.y());
  //}

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
      pA = xyCurr;
      toNextion("vAB.val=", 1);
      toNextion("btAB.pic=", 2);
    }
    if (button == btB) {
      //set point B
      pB = xyCurr;
      toNextion("page ", "drive");
    }

    // очищаем кнопки
    button  = ' ';
    signBtn = ' ';
  } else if (mode == modeD) {//<<<<<<<<<<<<<<<<<<< режим вождения
    //Serial.println("MODE = D");
    // если нажата кнопка паузы
    if (button == btP) {
      pause = !pause;
      if (pause) {//меняем картинку кнопки
        toNextion("p.pic=",34);
      } else {
        toNextion("p.pic=",33);
      }
      button = ' ';// очищаем кнопку
    }
    
    ///////////////////////////////////////////////////
    // основной цикл обновления экрана кусоуказателя //
    ///////////////////////////////////////////////////
    if (!pause) {
//////<<test code>>//////////////////////////////////////
      if (millis() - t1 > maxTimeMs) {
        t1 = millis();

        //pilot-------------------------------------------
        Point dir = xyCurr - xyPre;// направление перемещения
        Point pointOnLine;// точка на линии
        
        
        Point directionToPoint = pointOnLine - xyCurr;//направление на ближайшую точку на линию
        Point orthogonal(dir.getY(), -dir.getX());//ортогональный к направлению вектор

        long projection = Point::dotProduct(
          directionToPoint.normalized(), 
          orthogonal.normalized());
          
        // угол между ближайшей линией и направлением перемещения
        long angle = acos(projection);
        long k = 1.0;
        
        drv = k*angle;
        if (drv > 10) {
           drv = 10;
        }
        if (drv < -10) {
           drv = -10;
        }
        drv += 20;
        // Send to Nextion
        //------------------------------------------------


        
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
