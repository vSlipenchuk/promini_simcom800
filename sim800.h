#include <Arduino.h>

#define LINEBUF 30

class sim800modem {
  public:
   Stream *com; // comport
   byte pinReset; //
   char line[LINEBUF]; byte line_len = 0; byte fLineReady = 0; //  collecting a line buffer
   char res[LINEBUF]; // first non-empty answer not started with AT
   byte cOK,cERR; // counters for
   sim800modem(Stream *s=0, int reset_pin = 0) { com=s, pinReset = reset_pin;};
   void init(Stream *s, int reset_pin = 0);
   void reset();
   // commands

   void _inchar(char ch); // when reading a char from a stream -> collect a line
   void _doline();  // process collected line

   int sim800modem::wait_res(int ms=5000); // default - 5sec answer
   int at4(char *cmd,char *cmd2=0,char *cmd3=0,char *cmd4=0);   
   int at(char *cmd) { return at4(cmd,0,0,0);}  

   // 
   char *atstr(char *cmd,char *def=""); // +CIMI, +CGSN -- one line output

   // at+creg? ->  1 or 5 is home and roaming and is OK
   int reg();

   int set_apn(char *apn);
   int wget(char *url,char *params);
   
   
};
