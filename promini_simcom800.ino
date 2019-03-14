#include <SoftwareSerial.h>
#include "sim800.h"

char *url =  ;// place here URL -> 

// begin promini+simcom800 board
SoftwareSerial modem(8,7) ; // RX,TX of sim800 connected to promini
sim800modem    m(&modem,A6); // -> and RST of modem
int pinTermo = A7; // termo NTC-res 10K, pulled UP
// done promini+simcom800 board

// local cash
char imei[22]; // copy of
char buf[200]; // for arg...

double Term() {  //Function to perform the fancy math of the Steinhart-Hart equation
 double Temp;
 int RawADC =  analogRead(pinTermo);
 Temp =log(10000.0/(1024.0/RawADC-1)) ; // Pull-up
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;              // Convert Kelvin to Celsius
    Serial.println("Temp="+String(Temp));
 return Temp;
}

void gsm_modem_write(char *par="size=info") {
  Serial.println("Write params:"+String(par));
  int a = m.wget(url,par);
     Serial.println("wget = "+String(a));
  if (a >0 ) {
    Serial.println("RESP:<"+String(m.res)+">");
  }
}

void report_temp() {
  sprintf(buf,"imei=%s&temp=%d",imei,(int)(100*Term()));
  
 gsm_modem_write(buf);
  
}

void setup() {
  // put your setup code here, to run once:
 
  Serial.begin(9600);
  modem.begin(9600); // me love spped
  m.reset();

  pinMode(pinTermo,INPUT);


  //gsmodem_reset(A6); // aready done by
  //modem.print("AT\r");
  int i = m.at("");
  Serial.println("Hello at="+String(i));
  m.at("+creg=2");
  m.res[0]=0;
  char *imsi = m.atstr("+cimi");
  Serial.println("imsi="+String(m.res));
  char *im = m.atstr("+cgsn");
  Serial.println("im="+String(m.res));
  strcpy(imei,m.res);

  int reg = m.reg();
  Serial.println("reg = "+String(reg));

  int a = m.set_apn("internet.mts.ru");
    Serial.println("set_apn = "+String(a));


  //gsm_modem_write();
  //Term();

  report_temp();

}

int sec;
int timer;

void every_second() {
  timer++;
  //Serial.println("Second "+String(sec));
 // delay(10);
  if (timer == 600) { // every 600 seconds
    report_temp();
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {byte ch = Serial.read();  
     if (ch == 27) m.reset();
      else modem.write(ch); } 
  if (modem.available())  {byte ch = modem.read();   Serial.write(ch);}
  
 int the_sec =  millis()/1000 ;
 
 if (the_sec !=sec) {
    sec=the_sec;
    every_second();
    }

}
