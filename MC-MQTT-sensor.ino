// ESP8266 sketch sending data from sensor to MQTT
// sensor: DHT22, wifi connection

#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "config.h"


// configuration of MQTT server ==============================================
//#define mqtt_server "mqtt server ip"
// no MQTT security
// #define mqtt_user "your_username"
// #define mqtt_password "your_password"

// =============== configuration of MQTT topics / OpenHAB items setup =======
#define humidity_topic "PI1/humidity"
#define temperature_topic "PI1/temperature"

// =============== configuration of DHT sensor ==============================
#define DHTPIN 2     // what digital pin we're connected to
// pin 1 3,3 V, pin 4 (right) to ground
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// ===================== config end ==========================================


// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("OpenHAB sensor started.");

  dht.begin();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// lastMsg is time when last measurement was done
long lastMsg = millis();

float temp = 0;
float hum = 0;

void loop(){
  //auto reconnect MQTT if needed
  if (!client.connected()) {
    reconnect();
  }
  //call MQTT library internal loop
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    // do measurement and send it
    lastMsg = now;

  
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float hum = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float temp = dht.readTemperature();
  
    // Check if any reads failed and exit early (to try again).
    if (isnan(hum) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    //publish measurement to MQTT
    Serial.print("Temperature:");
    Serial.println(String(temp).c_str());
    client.publish(temperature_topic, String(temp).c_str(), true);
   
    Serial.print("Humidity:");
    Serial.println(String(hum).c_str());
    client.publish(humidity_topic, String(hum).c_str(), true);
    // read the return code?
  }
}
