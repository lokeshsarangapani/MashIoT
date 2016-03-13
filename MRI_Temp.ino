#include <b64.h>
#include <HttpClient.h>

#include <DHT.h>
#include <LDateTime.h>
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LiquidCrystal.h>
#include <LAudio.h>

#define DHTPIN 2        // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define WIFI_AP "Lokesh"
#define WIFI_PASSWORD "MashIoT2016"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define per 100 //50
#define per1 10 //3
#define DEVICEID "Ddw0R03b" // Input your deviceId
#define DEVICEKEY_TEMP "3PqEjqcWu4h30sAV" // Input your deviceKey
#define DEVICEKEY_RH "3PqEjqcWu4h30sAV"
#define SITE_URL "api.mediatek.com"

LWiFiClient c;
unsigned int rtc;
unsigned int lrtc;
unsigned int rtc1;
unsigned int lrtc1;
char port[4]={0};
char connection_info[21]={0};
char ip[21]={0};             
int portnum;
int val = 0;
String tcpdata = String(DEVICEID) + "," + String(DEVICEKEY_TEMP) + ",0";
String upload_data;
String tcpcmd_led_on = "Temp_LowerLimit_Ctrl,,1";
String tcpcmd_led_off = "Temp_LowerLimit_Ctrl,,0";

// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
float temp = 0.0;
float Rh = 0.0;
float ftemp = 25.0;
float fRh = 0.0;

datetimeInfo t;

LWiFiClient c2;
HttpClient http(c2);

#define   TEMP_PROBE   0
int TempSensorValue;
String DTimeStr;
String TempStr;
String RhStr;

DHT dht(DHTPIN, DHTTYPE);
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(10, 8, 4, 5, 6, 7);

void setup()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Health Care Sol.");
  lcd.setCursor(0, 1);
  lcd.print("By MashIOT Team ");

  //pinMode(TEMP_PROBE, INPUT);
  Serial.begin(115200);

  LAudio.begin();
  LAudio.setVolume(9);
  LAudio.playFile( storageFlash,(char*)"beep1.wav");
  delay(1000);
  LAudio.stop();

  LTask.begin();
  LWiFi.begin();

  while(!Serial) delay(1000); // comment out this line when Serial is not present, ie. run this demo without connect to PC 
  Serial.println("Setup..");
  dht.begin();

  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  Serial.println("calling connection");

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);

  pinMode(13, OUTPUT);
  getconnectInfo();
  connectTCP();
}

void getconnectInfo(){
  //calling RESTful API to get TCP socket connection
  c2.print("GET /mcs/v2/devices/"); 
  c2.print(DEVICEID);
  c2.println("/connections.csv HTTP/1.1");
  //c2.println("GET /mcs/v2/devices/Ddw0R03b/datachannels/Temp_LowerLimit_Ctrl/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY_TEMP);
  c2.println("Connection: close");
  c2.println();
  
  delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    Serial.println("waiting HTTP response: ");
    Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  Serial.println();
  char c;
  int ipcount = 0;
  int count = 0;
  int separater = 0;
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      c = v;
      Serial.print(c);
      connection_info[ipcount]=c;
      if(c==',')
      separater=ipcount;
      ipcount++;    
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }
    
  }
  Serial.print("The connection info: ");
  Serial.println(connection_info);
  int i;
  for(i=0;i<separater;i++)
  {  ip[i]=connection_info[i];
  }
  int j=0;
  separater++;
  for(i=separater;i<21 && j<5;i++)
  {  port[j]=connection_info[i];
     j++;
  }
  Serial.println("The TCP Socket connection instructions:");
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.print("Port: ");
  Serial.println(port);
  portnum = atoi (port);
  Serial.println(portnum);

} //getconnectInfo

void uploadstatus(byte DevKey){
  //calling RESTful API to upload datapoint to MCS to report LED status
  Serial.println("calling connection");
  LWiFiClient c2;  

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);

  
  if(DevKey == 0)
    upload_data = "MashIoT_Temp,," + String(ftemp);
  else 
    upload_data = "MashIoT_Rh,," + String((int)fRh);
  
  int thislength = upload_data.length();
  HttpClient http(c2);
  c2.print("POST /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  if(DevKey == 0)
    c2.println(DEVICEKEY_TEMP);
  else
    c2.println(DEVICEKEY_RH);
  c2.print("Content-Length: ");
  c2.println(thislength);
  c2.println("Content-Type: text/csv");
  c2.println("Connection: close");
  c2.println();
  c2.println(upload_data);
  
  delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    Serial.print("waiting HTTP response: ");
    Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  Serial.println();
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      Serial.print(char(v));
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }   
  }
}

void connectTCP()
{
  //establish TCP connection with TCP Server with designate IP and Port
  c.stop();
  Serial.println("Connecting to TCP");
  Serial.println(ip);
  Serial.println(portnum);
  while (0 == c.connect(ip, portnum))
  {
    Serial.println("Re-Connecting to TCP");    
    delay(1000);
  }  
  Serial.println("send TCP connect");
  c.println(tcpdata);
  c.println();
  Serial.println("waiting TCP response:");
} //connectTCP

void heartBeat()
{
  Serial.println("send TCP heartBeat");
  c.println(tcpdata);
  c.println();
    
} //heartBeat

boolean disconnectedMsg = false;


void loop()
{
    //DTimeStr = String(t.day) + "/" + String(t.mon) + "/" + String(t.year) + "  " + String(t.hour) + ":" + String(t.mon);
    // read the value from the sensor:
    //TempStr = "Temp " + String(ftemp) + "C  Rh " + String((int)fRh) + "%";

    if((ftemp > 30 ) || (ftemp < 22))
    {
      LAudio.playFile( storageFlash,(char*)"beep1.wav");
    }
    else
    {
      LAudio.stop();
    }

    Serial.println("Loop.."); 
    getTempRh();

    if(temp > 0)
    {
      ftemp = ((ftemp * 6) + temp) / 7;
      fRh = ((fRh * 6) + Rh) / 7;
      
      TempStr = "Temp  : " + String(ftemp) + " C";
      lcd.setCursor(0, 0);
      lcd.print(TempStr);
      RhStr = "R.humidity " + String((int)fRh) + " %";
      lcd.setCursor(0, 1);
      lcd.print(RhStr);
    }

    //Check for TCP socket command from MCS Server 
    //String tcpcmd="";
    //while (c.available())
    //{
    //    int v = c.read();
    //    if (v != -1)
    //    {
    //      //Serial.print((char)v);
    //      tcpcmd += (char)v;
    //      if (tcpcmd.substring(40).equals(tcpcmd_led_on)){
    //        digitalWrite(13, HIGH);
    //        Serial.print("Switch LED ON ");
    //        tcpcmd="";
    //      }else if(tcpcmd.substring(40).equals(tcpcmd_led_off)){  
    //        digitalWrite(13, LOW);
    //        Serial.print("Switch LED OFF");
    //        tcpcmd="";
    //      }
    //    }
    //}
  
    //LDateTime.getRtc(&rtc);
    //if ((rtc - lrtc) >= per) 
    //{
    //  heartBeat();
    //  lrtc = rtc;
    //}
    
    //Check for report datapoint status interval
    LDateTime.getRtc(&rtc1);
    if ((rtc1 - lrtc1) >= per1) 
    {
      uploadstatus(0);
      uploadstatus(1);
      lrtc1 = rtc1;
    }

    //delay(1000);
}

void getTempRh()
{
    if(dht.readHT(&temp, &Rh))
    {
        Serial.println("------------------------------");
        Serial.print("temperature = ");
        Serial.println(temp);
        
        Serial.print("humidity = ");
        Serial.println(Rh);
    }
}

void printdatetime()
{
  LDateTime.getTime(&t); 
  Serial.print("Date ");
  Serial.print(t.day);
  Serial.print("/");
  Serial.print(t.mon);
  Serial.print("/");
  Serial.print(t.year);
  Serial.print(" Time ");
  Serial.print(t.hour);
  Serial.print(":");
  Serial.println(t.min);
}

