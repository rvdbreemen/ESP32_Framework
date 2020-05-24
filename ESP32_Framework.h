
#define SETTINGS_FILE   "/settings.ini"
#define CMSG_SIZE        512
#define JSON_BUFF_MAX   1024

#define ESP_RESET()             ESP.restart()
#define ESP_RESET_REASON()      ((String)esp_reset_reason()).c_str()
#define ESP_GET_FREE_BLOCK()    ESP.getMaxAllocHeap()
#define ESP_GET_CHIPID()        ((uint32_t)ESP.getEfuseMac()) //The chipID is essentially its MAC address (length: 6 bytes) 

//#include <Timezone.h>           // https://github.com/JChristensen/Timezone
//#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time
#include <ezTime.h>             // https://github.com/ropg/ezTime
#include <TelnetStream.h>       // https://github.com/jandrassy/TelnetStream/commit/1294a9ee5cc9b1f7e51005091e351d60c8cddecf
#include "Debug.h"
#include "networkStuff.h"
#include "SPIFFS.h"

  
bool      Verbose = false;
char      cMsg[CMSG_SIZE];
char      fChar[10];
String    lastReset   = "";
char      settingHostname[41];

const char *weekDayName[]  {  "Unknown", "Zondag", "Maandag", "Dinsdag", "Woensdag"
                            , "Donderdag", "Vrijdag", "Zaterdag", "Unknown" };
const char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "FAST READ", "SLOWREAD", "Unknown" };


// eof
