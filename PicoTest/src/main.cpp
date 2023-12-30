#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <SDFS.h>
#include <file.h>
#include <WiFi.h>
#define TIMER_INTERRUPT_DEBUG 0   //1
#define _TIMERINTERRUPT_LOGLEVEL_ 0 //4
#include "RPi_Pico_TimerInterrupt.h"
#include <WebServer.h>

const int rowDataSize = 42;
class CircleQ
{
public:
  String fileName;
  int maxRows;
  int rowSize;
  int currentRow;
  File tbl;

  void begin(String filename, int maxrows, int rowsize, int currentrow = -1)
  {
    fileName = filename;
    maxRows = maxrows;
    rowSize = rowsize;
    if (!LFS.exists("/" + filename))
    {
      Serial.println(filename + " Does not exist");
      tbl = LFS.open("/" + filename, "w");
      tbl.flush();
      tbl.close();
    }
    if (currentrow == -1)
      currentRow = getRows();
    else
      currentRow = currentrow;
  }

  String get(int record)
  {
    int moveTo = (record * rowSize-(1*record));  //(1*record) account for newline being 2 chars (i think)
    String buff = "";
    tbl = LFS.open("/" + fileName, "r");
    delay(10);
    if (tbl.available())
    {
      tbl.seek(moveTo, SeekSet); 
      buff = tbl.readStringUntil('\n');
    }
    else
    {
      Serial.println("Cant seek");
    }
    tbl.close();
    return String(buff);
  }
  void put(int record, String s)
  {
    tbl = LFS.open("/" + fileName, "r+");
    tbl.seek(record * rowSize, SeekSet);
    unsigned char buff[rowSize];
    s.getBytes(buff, rowSize);
    tbl.write(buff, rowSize);
    tbl.flush();
    tbl.close();
  }

  int Add(String s)
  {
    int mils = millis();

    if (currentRow > maxRows)
      currentRow = 0;

    tbl = LFS.open("/" + fileName, "r+");
    tbl.seek((currentRow * rowSize) - currentRow, SeekSet);
    unsigned char buff[rowSize];
    s.getBytes(buff, rowSize);
    tbl.write(buff, rowSize);
    tbl.flush();
    tbl.close();

    currentRow += 1;
    setConfigItem("CurrentRow", String(currentRow));
    Serial.println("CurrentRow=" + String(currentRow) + " " + String(millis() - mils) + "mils");
    return currentRow;
  }

  long getRows()
  {
    tbl = LFS.open("/" + fileName, "r+");
    int tblSize = tbl.size();
    tbl.close();
    return tblSize / rowSize;
  }

private:
};
CircleQ cQ;

const char *ssid = "SaritaMist";
const char *password = "53ft.Selene";

int port = 8888;
WiFiServer server(port);

WebServer wserver(80);

bool bAllowSave = false;
RPI_PICO_Timer ITimer0(0);
#define TIMER0_INTERVAL_MS 5000

bool TimerHandler0(struct repeating_timer *t)
{
// #if (TIMER_INTERRUPT_DEBUG > 0)
//   Serial.print("ITimer0: millis() = ");
//   Serial.println(millis());
// #endif
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

void FlashOn(){
  digitalWrite(LED_BUILTIN, HIGH);
}
void FlashOff(){
  digitalWrite(LED_BUILTIN, LOW);
}
void SaveData()
{
  FlashOn();
  char data[rowDataSize];
  int x = sprintf(data, "%-19.19s,%-6.6s,%06.2f,%06.2f\n", "2023-10-10 10:10:10", "Sandy", (float)cQ.currentRow, (float)cQ.getRows());
  String s(data);
  cQ.Add(s);
  Serial.println("Saving to CircleQ: " + s + " " + String(cQ.currentRow));
  bAllowSave = false;
  FlashOff();
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
  if (WiFi.macAddress() == "28:cd:c1:0c:ca:15")
    WiFi.mode(WIFI_AP);
  else
    WiFi.mode(WIFI_STA);
  // IPAddress ip(10, 10, 10, iifIP());
  // IPAddress gateway(10, 10, 10, 60);
    IPAddress ip(192, 168, 4, 10);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);
  Serial.println(ssid);

  //  IPAddress ip(10,10,10,55);
  //  IPAddress gateway(10,10,10,2);
  //  IPAddress dns(8,8,8,8);
  //  IPAddress dns1(8,8,4,4);

  // IPAddress ip(getValue(MachineIP, '.', 0).toInt(), getValue(MachineIP, '.', 1).toInt(), getValue(MachineIP, '.', 2).toInt(), getValue(MachineIP, '.', 3).toInt());
  //  IPAddress gateway(getValue(GatewayIP, '.', 0).toInt(), getValue(GatewayIP, '.', 1).toInt(), getValue(GatewayIP, '.', 2).toInt(), getValue(GatewayIP, '.', 3).toInt());
  //  IPAddress dns1(getValue(dns1IP, '.', 0).toInt(), getValue(dns1IP, '.', 1).toInt(), getValue(dns1IP, '.', 2).toInt(), getValue(dns1IP, '.', 3).toInt());
  //  IPAddress dns2(getValue(dns2IP, '.', 0).toInt(), getValue(dns2IP, '.', 1).toInt(), getValue(dns2IP, '.', 2).toInt(), getValue(dns2IP, '.', 3).toInt());
  //  IPAddress subnet(255, 255, 255, 0);
  // WiFi.config(ip, gateway, subnet,dns1,dns2);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.print(WiFi.macAddress());
  Serial.println();
}

#pragma region WEB_STUFF
void handleRoot() {
  wserver.send(200, "text/plain", "hello from PiPico!");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += wserver.uri();
  message += "\nMethod: ";
  message += (wserver.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += wserver.args();
  message += "\n";
  for (uint8_t i=0; i<wserver.args(); i++){
    message += " " + wserver.argName(i) + ": " + wserver.arg(i) + "\n";
  }
  wserver.send(404, "text/plain", message);
}

  void DumpData()
  {
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

#pragma endregion


void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BOOTSEL, INPUT_PULLUP);

  // String s = "Now is the time for all good men";
  // String n[2];
  // Split(s,' ',n,1);
  // for (int i=0;i<2;i++)
  //    Flash(n[i]);

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
  Serial.println(readAllLines("config.txt"));

  int CurrentLine = getConfigItem("CurrentRow", "0").toInt();
  Serial.println("Current CircleQ Row is:" + String(CurrentLine));
#pragma endregion 

  // LFS.remove("/Data.log");
  // setConfigItem("CurrentRow", "0");
  // CurrentLine = 0;
  cQ.begin("Data.log", 100, rowDataSize, CurrentLine); // 10080 is 1 week of data for Channel 1 only saved at a reate of once a minute 1x60x24x7=10080
  // for (int i=0;i<100;i++)
  //    SaveData();
  Serial.println("cQ rows=" + String(cQ.getRows()));
  Serial.println(readAllLines("Data.log"));
  // for(int i=0;i<99;i++)
  //   Serial.println("READING:"+cQ.get(i));
//  delay(30);

  SetupWiFi();

  #pragma region SETUP_WEB_SERVER
  wserver.on("/", handleRoot);
  wserver.on("/DumpData", DumpData);
  wserver.on("/inline", [](){
    wserver.send(200, "text/plain", "this works as well");
  });
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
  {
    Serial.print(F("Starting ITimer0 OK, millis() = "));
    Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer0. Select another Timer, freq. or timer"));
#pragma endregion
    
}


void setup1()
{
 Serial.begin(115200);
  //pinMode(LED_BUILTIN, OUTPUT); 
  //pinMode(BOOTSEL, INPUT_PULLUP);

}

void loop()
{

  wserver.handleClient();
  #pragma region CLIENTSERVER

  char myOwnBigBuffer[rowDataSize];
  int index = 0;
  String s = "";

  if(!digitalRead(BOOTSEL)){
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Resettting to Defaults");
    LFS.remove("/config.txt");
    LFS.remove("/Data.log");
    setConfigItem("CurrentRow", "0");
    setup();
  }

  // if (bAllowSave)
  //   SaveData();

  // put your main code here, to run repeatedly:

  if (WiFi.macAddress() == "28:cd:c1:0c:ca:15")
  {
    //Flash("In Server Mode");
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
          Split(s,' ',getR);
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
    if(WiFi.status() != WL_CONNECTED)
      SetupWiFi();
    Flash("In Client Mode @ "+ WiFi.localIP().toString() );
    WiFiClient client;
    IPAddress ip(192,168,4,1);
    if (client.connect(ip, port))
    {
      Flash("Connected...");
      while (client.connected())
      {
        const char *s = String("getRecord "+ String(random(100))+"\n").c_str();
        client.write(s);
        Serial.println(s);
        String s1 = client.readStringUntil('\n');
        Serial.println("Server Returned: "+s1);
      }
    }
    client.stop();
  }
  
  #pragma endregion
  
}

void loop1(){
  Serial.println("In Loop 1");
  delay(1000);
  // if(digitalRead(BOOTSEL))
  //  digitalWrite(LED_BUILTIN, LOW);
  // else
  //  digitalWrite(LED_BUILTIN, HIGH);
  // delay(1000);
  
}
