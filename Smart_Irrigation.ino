#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

#define relay D1
#define trigpin D3
#define echopin D4
#define motor D6

int duration,distance;
float t,h;

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN,DHTTYPE);


const char* ssid     = "Your SSID";//Enter the ssid of your router
const char* password = "Your password";//Enter the password of your router Sm@rtEngin33ring

const char* host = "api.thingspeak.com";

const char* host1 = "api.msg91.com";
const int httpsPort1 = 443;
const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";
const char* privateKey = "Your Private Key";
const String talkBackAPIKey = "Your Public Key";
const String talkBackID = "27970";
String talkBackCommand,tbRequest;
int sensorvalue,MoistureValue;
String line1;

void setup() {
  pinMode(relay,OUTPUT);
  pinMode(motor,OUTPUT);
  pinMode(trigpin,OUTPUT);
  pinMode(echopin,INPUT);
  dht.begin();
  
  Serial.begin(115200);
  delay(10);
 

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
  void sensordata()
{  

   t=dht.readHumidity();
   h=dht.readTemperature();
   digitalWrite(trigpin,HIGH);
   delay(100);
   digitalWrite(trigpin,LOW);
   duration=pulseIn(echopin,HIGH);
   distance=(duration/2)*0.0343;
  
  int sensorvalue=analogRead(A0);
  MoistureValue=map(sensorvalue,0,1023,0,100);
  Serial.print("Moisture Value = ");
  Serial.println(MoistureValue);
  if(MoistureValue<40)
  {
   Serial.println("Moisture value is High : ");
  }
  else
  {
    Serial.println("Moisture value is Low : ");
  }
  
  
 }

void Upload_to_Cloud()
{
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/update";
  url += "?key=";
  url += privateKey;
  url += "&field1=";
  url += t;
  url += "&field2=";
  url += h;
  url += "&field3=";
  url += MoistureValue;
  url += "&field4=";
  url += distance;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(1000);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
//message code
if(MoistureValue>40)
{
  //cloud();
}

  
}

WiFiClient client;
void getTalkBack()
{
tbRequest="GET /talkbacks/"+ talkBackID + "/commands/execute?api_key=" + talkBackAPIKey;
Serial.println(tbRequest);

if(!client.connected())
{
  if (client.connect("api.thingspeak.com", 80))
  {
    client.println(tbRequest);
    if (client.connected())
      {
        talkBackCommand="";
        while(client.connected() && !client.available()) delay(10); //waits for data
        while (client.connected() || client.available())
            {
              talkBackCommand = client.readStringUntil('\n');
            }
        Serial.print("Command -> ");
        Serial.println(talkBackCommand);
        Serial.println();
      }
  client.stop();
  Serial.flush();
 }
}
}
  

  

void loop() 
{
  
delay(100);
sensordata();
Upload_to_Cloud();

getTalkBack(); // Get the command issued from mobile app/web app

if(talkBackCommand == "RON")
{
  digitalWrite(relay,LOW);// motor is ON
  Serial.println("Motor is ON");
}
 else if(talkBackCommand == "ROFF") 
{
   digitalWrite(relay,HIGH); // motor is OFF
  Serial.println("Motor is OFF");
}
 else if(talkBackCommand == "MON")
{
  digitalWrite(motor,HIGH);// motor is ON
  Serial.println("Motor is ON");
}
 else if(talkBackCommand == "MOFF") 
{
   digitalWrite(motor,LOW); // motor is OFF
  Serial.println("Motor is OFF");
}
delay(2000);  
}

void cloud(){
   WiFiClientSecure client1;
  Serial.print("connecting to ");
  Serial.println(host1);
  if (!client1.connect(host1, httpsPort1)) {
    Serial.println("connection failed");
    return;
  }

  if (client1.verify(fingerprint, host1)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url1 = "/api/sendhttp.php?sender=MSGIND&route=4&mobiles=8499004200&authkey=201450AvFaNsu5fJTY5a9fe171&country=91&message=Moisture Level is Low";
  Serial.print("requesting URL: ");
Serial.println(url1);

  client1.print(String("GET ") + url1 + " HTTP/1.1\r\n" +
               "Host: " + host1 + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client1.connected()) {
    line1 = client1.readStringUntil('\n');
    if (line1 == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  line1 = client1.readStringUntil('\n');
  if (line1.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line1);
  Serial.println("==========");
  Serial.println("closing connection");
}


 
  
     
  
  
     
  
    
  
  
