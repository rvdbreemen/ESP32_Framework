static const char indexPage[] =
R"(
<!DOCTYPE html>
<html charset="UTF-8">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">  
    <script src="/index.js"></script>
    <link rel="stylesheet" type="text/css" href="/index.css">
    <title>ESP32_Framework</title>
  </head>
  <body>
    <font face="Arial">
    <!-- <div class="dev-header"> -->
    <div class="header">
      <h1>
        <span id="sysName">ESP32_Framework</span> &nbsp; &nbsp; &nbsp;
        <span id="devName"    style='font-size: small;'>devName</span> &nbsp;
        <span id="devVersion" style='font-size: small;'>[devVersion]</span>
        <span id='theTime' class='nav-item nav-clock'>theTime</span>
      </h1>
    </div>
    </font>
    <div id="displayMainPage"      style="display:none">
      <div class="nav-container">
        <div class='nav-left'>
          <a id='saveMsg' class='nav-item tabButton' style="background: lightblue;">opslaan</a>
          <a id='M_FSexplorer'    class='nav-img'>
                      <img src='/FSexplorer.png' alt='FSexplorer'></a>
          <a id='Settings'      class='nav-img'>
                      <img src='/settings.png' alt='Settings'></a>
        </div>
      </div>
      <br/>
      <div id="mainPage">
        <div id="waiting">Wait! loading page .....</div>
      </div>
    </div>

    <div id="displaySettingsPage"  style="display:none">
      <div class="nav-container">
        <div class='nav-left'>
          <a id='back' class='nav-item tabButton' style="background: lightblue;">Terug</a>
          <a id='saveSettings' class='nav-item tabButton' style="background: lightblue;">opslaan</a>
          <a id='S_FSexplorer' class='nav-img'>
                      <img src='/FSexplorer.png' alt='FSexplorer'></a>
        </div>
      </div>
      <br/>
      <div id="settingsPage"></div>
    </div>
  
    <!-- KEEP THIS --->

    <!-- Pin to bottom right corner -->
    <div class="bottom right-0">2020 &copy; Willem Aandewiel</div>

    <!-- Pin to bottom left corner -->
    <div id="message" class="bottom left-0">-</div>

    <script>
      console.log("now continue with the bootstrapMain");
      /*****
      if (typeof bootsTrapMain !== "function") 
      { 
        console.log("bootsTrapMain() does NOT exist ;-) ");
        var mainJaveScript = '/index.js';
        var el = document.createElement('script');
        el.async = false;
        el.src = mainJaveScript;
        el.type = 'text/javascript';
        (document.getElementsByTagName('HEAD')[0]||document.body).appendChild(el);
      }
      document.getElementById("M_FSexplorer").src="/FSexplorer.png";
      document.getElementById("Settings").src="/settings.png";
      document.getElementById("S_FSexplorer").src="/FSexplorer.png";
      *****/
      window.onload=bootsTrapMain;
    </script>

  </body>

</html>
)";

void sendIndexPage()
{
  httpServer.send(200, "text/html", indexPage);

} // sendIndexPage()

// eof
