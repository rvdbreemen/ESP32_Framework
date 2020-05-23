/* 
***************************************************************************  
**  Program : FSexplorer
**  Version : 3.2   15-05-202
**
**  Mostly stolen from https://www.arduinoforum.de/User-Fips
**  For more information visit: https://fipsok.de
**  See also https://www.arduinoforum.de/arduino-Thread-SPIFFS-DOWNLOAD-UPLOAD-DELETE-ESP32-NodeMCU
**
***************************************************************************      
  Copyright (c) 2018 Jens Fleischer. All rights reserved.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This file is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*******************************************************************
**      Usage:
**      
**      setup()
**      {
**        setupFSexplorer();
**        httpServer.serveStatic("/FSexplorer.png",   SPIFFS, "/FSexplorer.png");
**        httpServer.on("/",          sendIndexPage);
**        httpServer.on("/index",     sendIndexPage);
**        httpServer.on("/index.html",sendIndexPage);
**        httpServer.begin();
**      }
**      
**      loop()
**      {
**        httpServer.handleClient();
**        .
**        .
**      }
*/

#define MAX_FILES_IN_LIST   25

const char Helper[] = R"(
  <br>You first need to upload these two files:
  <ul>
    <li>FSexplorer.html</li>
    <li>FSexplorer.css</li>
  </ul>
  <form method="POST" action="/upload" enctype="multipart/form-data">
    <input type="file" name="upload">
    <input type="submit" value="Upload">
  </form>
  <br/><b>or</b> you can use the <i>Flash Utility</i> to flash firmware or SPIFFS!
  <form action='/update' method='GET'>
    <input type='submit' name='SUBMIT' value='Flash Utility'/>
  </form>
)";
const char Header[] = "HTTP/1.1 303 OK\r\nLocation:FSexplorer.html\r\nCache-Control: no-cache\r\n";

//=====================================================================================
void setupFSexplorer()    // Funktionsaufruf "spiffs();" muss im Setup eingebunden werden
{    
  SPIFFS.begin();
  
  if (SPIFFS.exists("/FSexplorer.html")) 
  {
    httpServer.serveStatic("/FSexplorer.html", SPIFFS, "/FSexplorer.html");
    httpServer.serveStatic("/FSexplorer",      SPIFFS, "/FSexplorer.html");
  }
  else 
  {
    httpServer.send(200, "text/html", Helper); //Upload the FSexplorer.html
  }
  httpServer.on("/api/listfiles", APIlistFiles);
  httpServer.on("/SPIFFSformat", formatSpiffs);
  httpServer.on("/upload", HTTP_POST, []() {}, handleFileUpload);
  httpServer.on("/ReBoot", reBootESP);
  httpServer.on("/update", updateFirmware);
  httpServer.onNotFound([]() 
  {
    if (Verbose) DebugTf("in 'onNotFound()'!! [%s] => \r\n", String(httpServer.uri()).c_str());
    if (httpServer.uri().indexOf("/api/") == 0) 
    {
      if (Verbose) DebugTf("next: processAPI(%s)\r\n", String(httpServer.uri()).c_str());
      processAPI();
    }
    else if (httpServer.uri() == "/")
    {
      DebugTln("index requested..");
      sendIndexPage();
    }
    else
    {
      DebugTf("next: handleFile(%s)\r\n"
                      , String(httpServer.urlDecode(httpServer.uri())).c_str());
      if (!handleFile(httpServer.urlDecode(httpServer.uri())))
      {
        httpServer.send(404, "text/plain", "FileNotFound\r\n");
      }
    }
  });
  
} // setupFSexplorer()


//=====================================================================================
void APIlistFiles()             // Senden aller Daten an den Client
{   
typedef struct _fileMeta {
    char    Name[30];     
    int32_t Size;
  } fileMeta;

  _fileMeta dirMap[30];
  int fileNr = 0;
  
  File root = SPIFFS.open("/");         // List files on SPIFFS
  if(!root){
      DebugTln("- failed to open directory");
      return;
  }
  if(!root.isDirectory()){
      DebugTln(" - not a directory");
      return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
        DebugT("  DIR : ");
        DebugTln(file.name());
        // directory is skipped
    } else {
      DebugT("  FILE: ");
      DebugT(file.name());
      DebugT("\tSIZE: ");
      DebugTln(file.size());
      dirMap[fileNr].Name[0] = '\0';
      strncat(dirMap[fileNr].Name, file.name(), 29); // first copy file.name() to dirMap
      memmove(dirMap[fileNr].Name, dirMap[fileNr].Name+1, strlen(dirMap[fileNr].Name)); // remove leading '/'
      dirMap[fileNr].Size = file.size();
    }
    file = root.openNextFile();
    fileNr++;
  }

  // -- bubble sort dirMap op .Name--
  for (int8_t y = 0; y < fileNr; y++) {
    yield();
    for (int8_t x = y + 1; x < fileNr; x++)  {
      //DebugTf("y[%d], x[%d] => seq[y][%s] / seq[x][%s] ", y, x, dirMap[y].Name, dirMap[x].Name);
      if (compare(String(dirMap[x].Name), String(dirMap[y].Name)))  
      {
        //Debug(" !switch!");
        fileMeta temp = dirMap[y];
        dirMap[y] = dirMap[x];
        dirMap[x] = temp;
      } /* end if */
      //Debugln();
    } /* end for */
  } /* end for */

  DebugTln(F("\r\n"));
  for(int f=0; f<fileNr; f++)
  {
    Debugf("%-25s %6d bytes \r\n", dirMap[f].Name, dirMap[f].Size);
    yield();
  }
  
  String temp = "[";
  for (int f=0; f < fileNr; f++)  
  {
    if (temp != "[") temp += ",";
    temp += R"({"name":")" + String(dirMap[f].Name) + R"(","size":")" + formatBytes(dirMap[f].Size) + R"("})";
  }
  //SPIFFS.info(SPIFFSinfo);
  temp += R"(,{"usedBytes":")" + formatBytes(SPIFFS.usedBytes() * 1.05) + R"(",)" +       // Berechnet den verwendeten Speicherplatz + 5% Sicherheitsaufschlag
          R"("totalBytes":")" + formatBytes(SPIFFS.totalBytes()) + R"(","freeBytes":")" + // Zeigt die Größe des Speichers
          (SPIFFS.totalBytes() - (SPIFFS.usedBytes() * 1.05)) + R"("}])";               // Berechnet den freien Speicherplatz + 5% Sicherheitsaufschlag
  
  httpServer.send(200, "application/json", temp);
  
} // APIlistFiles()


//=====================================================================================
bool handleFile(String&& path) 
{
  if (httpServer.hasArg("delete")) 
  {
    DebugTf("Delete -> [%s]\n\r",  httpServer.arg("delete").c_str());
    SPIFFS.remove(httpServer.arg("delete"));    // Datei löschen
    httpServer.sendContent(Header);
    return true;
  }
  if (!SPIFFS.exists("/FSexplorer.html")) httpServer.send(200, "text/html", Helper); //Upload the FSexplorer.html
  if (path.endsWith("/")) path += "index.html";
  return SPIFFS.exists(path) ? ({File f = SPIFFS.open(path, "r"); httpServer.streamFile(f, contentType(path)); f.close(); true;}) : false;

} // handleFile()


//=====================================================================================
void handleFileUpload() 
{
  static File fsUploadFile;
  HTTPUpload& upload = httpServer.upload();
  if (upload.status == UPLOAD_FILE_START) 
  {
    if (upload.filename.length() > 30) 
    {
      upload.filename = upload.filename.substring(upload.filename.length() - 30, upload.filename.length());  // Dateinamen auf 30 Zeichen kürzen
    }
    Debugln("FileUpload Name: " + upload.filename);
    fsUploadFile = SPIFFS.open("/" + httpServer.urlDecode(upload.filename), "w");
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    Debugln("FileUpload Data: " + (String)upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } 
  else if (upload.status == UPLOAD_FILE_END) 
  {
    if (fsUploadFile)
      fsUploadFile.close();
    Debugln("FileUpload Size: " + (String)upload.totalSize);
    httpServer.sendContent(Header);
  }
  
} // handleFileUpload() 


//=====================================================================================
void formatSpiffs() 
{       //Formatiert den Speicher
  if (!SPIFFS.exists("/!format")) return;
  DebugTln(F("Format SPIFFS"));
  SPIFFS.format();
  httpServer.sendContent(Header);
  
} // formatSpiffs()

//=====================================================================================
const String formatBytes(size_t const& bytes) 
{ 
  return (bytes < 1024) ? String(bytes) + " Byte" : (bytes < (1024 * 1024)) ? String(bytes / 1024.0) + " KB" : String(bytes / 1024.0 / 1024.0) + " MB";

} //formatBytes()

//=====================================================================================
const String &contentType(String& filename) 
{       
  if (filename.endsWith(".htm") || filename.endsWith(".html")) filename = "text/html";
  else if (filename.endsWith(".css")) filename = "text/css";
  else if (filename.endsWith(".js")) filename = "application/javascript";
  else if (filename.endsWith(".json")) filename = "application/json";
  else if (filename.endsWith(".png")) filename = "image/png";
  else if (filename.endsWith(".gif")) filename = "image/gif";
  else if (filename.endsWith(".jpg")) filename = "image/jpeg";
  else if (filename.endsWith(".ico")) filename = "image/x-icon";
  else if (filename.endsWith(".xml")) filename = "text/xml";
  else if (filename.endsWith(".pdf")) filename = "application/x-pdf";
  else if (filename.endsWith(".zip")) filename = "application/x-zip";
  else if (filename.endsWith(".gz")) filename = "application/x-gzip";
  else filename = "text/plain";
  return filename;
  
} // &contentType()

//=====================================================================================
bool freeSpace(uint16_t const& printsize) 
{    
  return (SPIFFS.totalBytes() - (SPIFFS.usedBytes()* 1.05) > printsize) ? true : false;
  
} // freeSpace()


//=====================================================================================
void updateFirmware()
{
  DebugTln(F("Redirect to updateIndex .."));
  doRedirect("wait ... ", 1, "/updateIndex", false);
      
} // updateFirmware()

//=====================================================================================
void reBootESP()
{
  DebugTln(F("Redirect and ReBoot .."));
  doRedirect("Reboot ESP32_Framework ..", 60, "/", true);
      
} // reBootESP()

//=====================================================================================
void doRedirect(String msg, int wait, const char* URL, bool reboot)
{
  String redirectHTML = 
  "<!DOCTYPE HTML><html lang='en-US'>"
  "<head>"
  "<meta charset='UTF-8'>"
  "<style type='text/css'>"
  "body {background-color: lightblue;}"
  "</style>"
  "<title>Redirect to Main Program</title>"
  "</head>"
  "<body><h1>FSexplorer</h1>"
  "<h3>"+msg+"</h3>"
  "<br><div style='width: 500px; position: relative; font-size: 25px;'>"
  "  <div style='float: left;'>Redirect over &nbsp;</div>"
  "  <div style='float: left;' id='counter'>"+String(wait)+"</div>"
  "  <div style='float: left;'>&nbsp; seconden ...</div>"
  "  <div style='float: right;'>&nbsp;</div>"
  "</div>"
  "<!-- Note: don't tell people to `click` the link, just tell them that it is a link. -->"
  "<br><br><hr>If you are not redirected automatically, click this <a href='/'>Main Program</a>."
  "  <script>"
  "      setInterval(function() {"
  "          var div = document.querySelector('#counter');"
  "          var count = div.textContent * 1 - 1;"
  "          div.textContent = count;"
  "          if (count <= 0) {"
  "              window.location.replace('"+String(URL)+"'); "
  "          } "
  "      }, 1000); "
  "  </script> "
  "</body></html>\r\n";
  
  DebugTln(msg);
  httpServer.send(200, "text/html", redirectHTML);
  if (reboot) 
  {
    delay(5000);
    ESP.restart();
    delay(5000);
  }
  
} // doRedirect()
