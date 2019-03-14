#include "sim800.h"

/// utils - begin
byte lcmp(char **cmd,char *c) {
   int l = strlen(c);
  if (memcmp(*cmd,c,l)==0) { 
       c=*cmd; c+=l; while(*c && *c<=32) c++; *cmd=c; return 1;}
  return 0;
}
/// utils -end

   
void sim800modem::init(Stream *s, int reset_pin = 0) {
  com=s; pinReset = reset_pin;
  reset();
}

void sim800modem::reset() {
  pinMode(pinReset,OUTPUT);
  digitalWrite(pinReset,LOW);
  delay(100);
  digitalWrite(pinReset,HIGH);
  delay(100);
}

void sim800modem::_inchar(char ch) {
if (ch == '\r') { // end of line - start check?
   fLineReady = 1;
   return ;
   }
if (fLineReady) return ;// skip till read
if (line_len+1 < sizeof(line)-1) { // add a char
  line[line_len] = ch;
  line_len++;
  line[line_len]=0;  
  }
}

void sim800modem::_doline() {
  if (!fLineReady) return 0;
char *c = line;
while(*c && *c<=32) c++;
if (lcmp(&c,"OK"))  cOK++;
else if (lcmp(&c,"ERROR")) cERR++; 
else if (lcmp(&c,"+HTTPACTION:")) {   // 0,301,333
  cOK++; strcpy(res,c);
} 
else {
  //Serial.println("check line: "+String(c));
  if (!res[0] && c[0] && !lcmp(&c,"AT") && !lcmp(&c,"+HTTP")) { // copy result 
     strcpy(res,c);
     }
  }
fLineReady=0; line_len=0; line[0]=0; // clear a line buffer
}

int sim800modem::wait_res(int ms) {
cOK = cERR = 0; _doline();
while (ms>0) {
    if ( com->available()) {
      char ch = com->read();
      _inchar(ch);
       // Serial.println("CHAR:"+String(ch));
        Serial.print(String(ch));
      if (fLineReady) _doline();
      } else { delay(10); ms-=10;}
    if (cOK) return 1;
    if (cERR) return -1;
    }
return 0;
}

int sim800modem::at4(char *cmd,char *cmd2=0,char *cmd3=0,char *cmd4 = 0) {
  com->print("AT"); 
  if (cmd) com->print(cmd); 
  if (cmd2) com->print(cmd2); 
  if (cmd3) com->print(cmd3); 
  if (cmd4) com->print(cmd4); 
  com->print("\r");
 return wait_res(); 
}

char *sim800modem::atstr(char *cmd,char *def="") {
  res[0]=0; 
  if (at(cmd)) return res;
  return def;
}

int sim800modem::reg() {
  char *s = atstr("+creg?");  
  char *p = strstr(s,"0,"); // has reg
  if (!p) p=strstr(s,"2,"); // has reg with params
  //Serial.println("reg="+String(s));
  if (!p) return 0;
  p+=2;
  if (p[0]=='1' || *p =='5') return 1; // home or roam
  return 0;
}

int sim800modem::set_apn(char *apn) {
  at("+SAPBR=3,1,\"Contype\",\"GPRS\"");
  at4("+SAPBR=3,1,\"APN\",\"",apn,"\"",0);
  at("+SAPBR=1,1");
  return at("+SAPBR=2,1");  
 }

 int sim800modem::wget(char *url,char *params) {
  at("+HTTPINIT"); at("+HTTPPARA=\"CID\",1"); // httpInit  
  at4("+HTTPPARA=\"URL\",\"",url,params,"\"");
  if (!at("+HTTPACTION=0")) return 0;  
  int ok = wait_res(30000); // socket connected, wait +HTTPACTION notification
  if (!ok) return 0; // fail connect
  if (atstr("+httpread")>0) return 1;  // ok read
  return 0; // fail read http responce
 }

 
