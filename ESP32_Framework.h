
#define SETTINGS_FILE   "/settings.ini"
#define CMSG_SIZE        512
#define JSON_BUFF_MAX   1024

#include <ezTime.h>             // https://github.com/ropg/ezTime v0.8.3
#include <TelnetStream.h>       // https://github.com/jandrassy/TelnetStream
#include "Debug.h"
#include "networkStuff.h"
#include "SPIFFS.h"

  
bool      Verbose = false;
char      cMsg[CMSG_SIZE];
char      fChar[10];
String    lastReset   = "";
char      settingHostname[41];
uint32_t  ledTimer;

const char *weekDayName[]  {  "Unknown", "Zondag", "Maandag", "Dinsdag", "Woensdag"
                            , "Donderdag", "Vrijdag", "Zaterdag", "Unknown" };
const char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "FAST READ", "SLOWREAD", "Unknown" };


// eof
