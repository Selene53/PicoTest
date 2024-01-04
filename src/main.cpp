#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <SDFS.h>
#include <file.h>
#include <WiFi.h>
#define TIMER_INTERRUPT_DEBUG 0     // 1
#define _TIMERINTERRUPT_LOGLEVEL_ 0 // 4
#include "RPi_Pico_TimerInterrupt.h"
#include <WebServer.h>
#include <ezTime.h>
#include "funcs.h"


// const char *ssid = "SaritaMist_2GEXT";
// const char *password = "53ft.Selene";

const char *ssid = "RANGER";
const char *password = "fortyninefoot";

int port = 8888;
WiFiServer server(port);

WebServer wserver(80);

bool bAllowSave = false;
RPI_PICO_Timer ITimer0(0);
#define TIMER0_INTERVAL_MS 5000

Timezone myTZ;

bool TimerHandler0(struct repeating_timer *t)
{
  bAllowSave = true;
  return true;
}
void Flash(String text = "Flashing")
{
  Serial.println(text);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
}
void FlashOn()
{
  digitalWrite(LED_BUILTIN, HIGH);
}
void FlashOff()
{
  digitalWrite(LED_BUILTIN, LOW);
}


void SaveData()
{
  FlashOn();
  char data[rowDataSize]; // 2023-10-10 10:10:10,Sandy ,96.00,30.88
  int x = sprintf(data, "%-19.19s,%-6.6s,%-5.5s,%-5.5s\n", mynow(myTZ).c_str(), "Sandy", fmtNum((float)cQ.currentRow, 2, 2, 5).c_str(), fmtNum((float)analogReadTemp(), 2, 2, 5).c_str());
  cQ.Add(data); // Dont pass a string or you'll get an unwanted null wich terminates the string but it seens to come befor the /n
  bAllowSave = false;
  FlashOff();
}

void CreateConfig()
{
  String arr[16] = {"SalonTemp=20",
                    "MasterTemp=20.1",
                    "VIPTemp=20.2",
                    "SalonRadTemp=40.1",
                    "MasterRadTemp=40.2",
                    "VIPRadTemp=40.3",
                    "SalonFanSpeed=10",
                    "MasterFanSpeed=11",
                    "VIPFanSpeed=12",
                    "SalonSensorIndex=0",
                    "SalonRadSensorIndex=1",
                    "MasterSensorIndex=2",
                    "MasterRadSensorIndex=3",
                    "VIPSensorIndex=0",
                    "VIPRadSensorIndex=1",
                    "CurrentRow=0"};
  String str = Join(arr, 15);
  writeFile("config.txt", str);
}
int iifIP()
{
  if (WiFi.macAddress() == "28:cd:c1:0c:ca:15")
    return 60;
  else
    return 61;
}

void SetupWiFi()
{
  Flash("Setup WiFi, Connecting to ");
  // if (WiFi.macAddress() ==  "28:cd:c1:0c:ca:15")//"28:cd:c1:0c:c9:a9") //
  //   WiFi.mode(WIFI_AP);
  // else
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
}

#pragma region WEB_STUFF
void handleRoot()
{
  Serial.println("User asked for main page");
  wserver.send(200, "text/plain", "hello from PiPico! " + String(millis()));
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += wserver.uri();
  message += "\nMethod: ";
  message += (wserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += wserver.args();
  message += "\n";
  for (uint8_t i = 0; i < wserver.args(); i++)
  {
    message += " " + wserver.argName(i) + ": " + wserver.arg(i) + "\n";
  }
  wserver.send(404, "text/plain", message);
}

void DumpData()
{
  Serial.println("User asked for data ");
  File download = LFS.open("/Data.log", "r");
  if (download)
  {
    wserver.sendHeader("Content-Type", "text/text");
    wserver.sendHeader("Content-Disposition", "attachment; filename=Data.log");
    wserver.sendHeader("Connection", "close");
    wserver.streamFile(download, "application/octet-stream");
    download.close();
  }
  else
    handleNotFound();
}

void GetRecord()
{ // call like this http://192.168.0.144/GetRecord?Record=10
  String s = cQ.get(wserver.arg(0).toInt());
  Serial.println("User asked for Record " + s + " " + wserver.arg(0));
  wserver.send(200, "text/plain", "hello from PiPico! " + s);
}

#pragma endregion

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BOOTSEL, INPUT_PULLUP);

#pragma region MOUNT_FILE_SYSTEM
  delay(5000);
  if (!LFS.begin())
    Serial.println("An Error has occurred while mounting File system");
  else
    Serial.println("Mounted File System");

  FSInfo info;
  LFS.info(info);
  size_t totalBytes = info.totalBytes;
  size_t usedBytes = info.usedBytes;

  listDir(&LFS, "/", 3);

  float percent = (float)usedBytes / (float)totalBytes;
  Serial.println("Total Bytes=" + String(totalBytes) + " Used Bytes:" + String(usedBytes) + " File space usage %=" + String(percent * 100));

#pragma endregion

#pragma region CONFIG_SETUP
  // LFS.remove("/config.txt");
  if (!LFS.exists("/config.txt"))
    CreateConfig();
  Serial.println(readAllLines("config.txt"));

  int CurrentLine = getConfigItem("CurrentRow", "0").toInt();
  Serial.println("Current CircleQ Row is:" + String(CurrentLine));
#pragma endregion

  // LFS.remove("/Data.log");
  // setConfigItem("CurrentRow", "0");
  // CurrentLine = 0;
  cQ.begin("Data.log", 100, rowDataSize, CurrentLine); // 10080 is 1 week of data for Channel 1 only saved at a reate of once a minute 1x60x24x7=10080
  Serial.println("cQ rows=" + String(cQ.getRows()));
  Serial.println(readAllLines("Data.log"));

  SetupWiFi();

#pragma region SETUP_WEB_SERVER
  wserver.on("/", handleRoot);
  wserver.on("/DumpData", DumpData);
  wserver.on("/GetRecord", GetRecord);
  wserver.onNotFound(handleNotFound);
  wserver.begin(80);
  Serial.println("HTTP server started");
#pragma endregion

  Serial.println("==========================");
  Serial.println(cQ.get(36));
  Serial.println("==========================");

  server.begin(port);
  Flash("Server Started");

#pragma region INTERUPTS
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
    Serial.println("Starting ITimer0 OK, millis() = "+String(millis()));
  else
    Serial.println(F("Can't set ITimer0. Select another Timer, freq. or timer"));
#pragma endregion

  waitForSync();
  myTZ.setLocation(F("America/Vancouver"));
  Serial.println(mynow(myTZ));
}
void setup1()
{
  Serial.begin(115200);
  // pinMode(LED_BUILTIN, OUTPUT);
  // pinMode(BOOTSEL, INPUT_PULLUP);
}

void loop()
{

  wserver.handleClient();
  if (!digitalRead(BOOTSEL))
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Resettting to Defaults");
    LFS.remove("/config.txt");
    CreateConfig();
    LFS.remove("/Data.log");
    cQ.begin("Data.log", 100, rowDataSize, 0);
    setConfigItem("CurrentRow", "0");
  }

  if (bAllowSave)
    SaveData();

#pragma region CLIENTSERVER

  char myOwnBigBuffer[rowDataSize];
  int index = 0;
  String s = "";

  if (WiFi.macAddress() == "28:cd:c1:0c:ca:15") //"28:cd:c1:0c:c9:a9" ) //
  {
    // Flash("In Server Mode @ " + WiFi.localIP().toString() + " " + String(millis()));
    WiFiClient client = server.accept();

    if (client)
    {
      if (client.connected())
        Serial.println("Client Connected");

      while (client.connected())
      {
        while (client.available() > 0)
        {
          myOwnBigBuffer[index] = client.read(); // read it
          index++;
        }
        if (index > 0)
        {
          Flash("Incomming message");
          s = String(myOwnBigBuffer);
          s.trim();
          Flash(s);
          String getR[10];
          Split(s, ' ', getR);
          if (getR[0] == "getRecord")
          {
            s = cQ.get(getR[1].toInt());
            const char *c = s.c_str();
            client.write(c);
            client.stop();
          }
          index = 0;
        }
      }

      Serial.println("Client disconnected");
      Flash("Client sent [" + s + "]");
      s = "";
    }
  }
  else
  {
    if (WiFi.status() != WL_CONNECTED)
      SetupWiFi();
    Flash("In Client Mode @ " + WiFi.localIP().toString());
    WiFiClient client;
    IPAddress ip(192, 168, 0, 164);
    if (client.connect(ip, port))
    {
      Flash("Connected...");
      while (client.connected())
      {
        const char *s = String("getRecord " + String(random(100)) + "\n").c_str();
        client.write(s);
        Serial.println(s);
        String s1 = client.readStringUntil('\n');
        Serial.println("Server Returned: " + s1);
      }
    }
    client.stop();
  }

#pragma endregion
}

void loop1()
{
  Serial.println("In Loop 1");
  delay(10000);
  // if(digitalRead(BOOTSEL))
  //  digitalWrite(LED_BUILTIN, LOW);
  // else
  //  digitalWrite(LED_BUILTIN, HIGH);
  // delay(1000);
}
