#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
//----------------------------------------

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

const int DHTPin = 4;
  String t;
#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

int z=0;
int y=0;
//void(* resetFunc) (void) = 0;
//----------------------------------------SSID dan Password wifi mu gan.
const char* ssid = "iot"; //--> Nama Wifi / SSID.
const char* password = "zx123456"; //-->  Password wifi .

//----------------------------------------

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

String GAS_ID = "AKfycbwPaVytmt1zhDG9PosYFDpkFxdrX5NTT9potNSxaS8aA9n2grecACi9yBWZ9uNCmEAY"; //--> spreadsheet script ID

//============================================ void setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);
  dht.begin();

  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, HIGH); //--> 

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  //----------------------------------------If successfully connected to the wifi router, the IP Address that will be visited is displayed in the serial monitor
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}
//==============================================================================
//============================================================================== void loop
void loop() {
  // if(z%10==0){
  //   if(y%2==0){
  //   WiFi.disconnect();
  //   WiFi.begin(ssid1, password1);
  //   Serial.println("Widfi1 reconnected");
  //   y++;
  //   }
  //   else{
  //     WiFi.disconnect();
  //     WiFi.begin(ssid, password);
  //     Serial.println("Widfi0 reconnected");
  //     y++;
  //   }
  // }

  now = millis();
  // Publishes new temperature and humidity every 3 seconds
  if (now - lastMeasure > 3000) {
    lastMeasure = now;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Computes temperature values in Celsius
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    
    // Uncomment to compute temperature values in Fahrenheit 
    // float hif = dht.computeHeatIndex(f, h);
    // static char temperatureTemp[7];
    // dtostrf(hif, 6, 2, temperatureTemp);
    
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);

   
    
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");
    // Serial.print(hif);
    // Serial.println(" *F");
     sendData(t,h);
    z++;
    if(z%10==0){
      ESP.restart(); 
    }
    // Serial.print(z);
  }

  
}
//*****
//==============================================================================

void sendData(float value,float value2) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Proses dan kirim data  

  float string_temp = value; 
  float string_humi = value2;
  String url = "/macros/s/" + GAS_ID + "/exec?temp=" + string_temp + "&humi="+string_humi; //  2 variables 
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //---------------------------------------
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp32/Arduino CI successfull!");
  } else {
    Serial.println("esp32/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
}
