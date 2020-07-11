// ESP8266 sketch sending data from sensor to MQTT
// sensor: button with led

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"


// configuration f MQTT server ==============================================
//#define mqtt_server "your mqtt server IP address"
// no MQTT security
// #define mqtt_user "your_username"
// #define mqtt_password "your_password"

// =============== configuration of MQTT topics / OpenHAB items setup =======
#define button_topic "PI1/button"

const int buttonPin = D2;   // the number of the pushbutton pin
const int ledPin = D1;      // the number of the LED pin

// ===================== config end ==========================================



WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("Minecraft MQTT sensor started.");

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);  
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
    if (client.connect("MC-MQTT-sensor-button")) {
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


// output status
int Active = 0;

byte currState = LOW;         // variable for reading the pushbutton status
// variable to remember the last state of the button
byte lastState = HIGH;


void loop(){
  //auto reconnect MQTT if needed
  if (!client.connected()) {
    reconnect();
  }
  //call MQTT library internal loop
  client.loop();

  // read the state of the pushbutton value:
  currState = digitalRead(buttonPin);

  // if it changed
  if (currState != lastState)
  {
    if (currState == LOW)
    {
      // change selected mode
      if (Active == 1)
      {
        Active = 0;
        // turn LED off:
        digitalWrite(ledPin, LOW);
        Serial.print("Button OFF");
        client.publish(button_topic, "off", true);
      } else {
        Active = 1;
        // turn LED on:
        digitalWrite(ledPin, HIGH);
        Serial.print("Button ON");
        client.publish(button_topic, "on", true);
      }
    }

    // remember last state
    lastState = currState;
  }
}
