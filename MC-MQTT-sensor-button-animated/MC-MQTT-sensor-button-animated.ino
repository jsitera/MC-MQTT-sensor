// ESP8266 sketch sending data from sensor to MQTT
// sensor: button with led / animated version
// TNT button - when pushed sends MQTT message and starts animation

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// library for button management using callbacks 
// https://github.com/r89m/Button https://github.com/r89m/PushButton
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce-Arduino-Wiring

// callback scheduling via SimpleTimer.h not widely used on ESP, why
// build-in Ticker.h (for ESP only)?
#include <Ticker.h>

// configuration f MQTT server ==============================================
//#define mqtt_server "your mqtt server IP address"
// no MQTT security
// #define mqtt_user "your_username"
// #define mqtt_password "your_password"

// =============== configuration of MQTT topics / OpenHAB items setup =======
#define button_topic "PI1/button-animated"

const int buttonPin = 32;   // D2 the number of the pushbutton pin
const int ledPin = 33;      // D1 the number of the LED pin

// ===================== config end ==========================================

// button
PushButton button = PushButton(buttonPin, ENABLE_INTERNAL_PULLUP);

Ticker animation; // Ticker.h object to call a callback 
int animationPhase = 0;

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("Minecraft MQTT sensor started.");

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // button setup via callbacks - each callback can handle multiple buttons
  // When the button is released
  button.onRelease(onButtonReleased);
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


void loop(){
  //auto reconnect MQTT if needed
  if (!client.connected()) {
    reconnect();
  }

  //call MQTT library internal loop
  client.loop();
  //call button library internal loop
  button.update();

  // Ticker.h doesn't need to call (it is attached to hardware interupt)
}

// some button released, duration reports back the total time that the button was held down
void onButtonReleased(Button& btn, uint16_t duration){
  
  Serial.print("Button pressed");
  client.publish(button_topic, "on", true);

  animationPhase=16;
  animation.attach_ms(250, animation1); //start calling animation procedure
  

}

void animation1() {
  Serial.print("Animation phase:");
  Serial.println(animationPhase);
  if (animationPhase > 3) {
    if (animationPhase % 2 == 0) { //even
      digitalWrite(ledPin, HIGH);
      Serial.println("ON");
    } else {
      digitalWrite(ledPin, LOW);
      Serial.println("OFF");
    }
  } else if (animationPhase == 1) {
    digitalWrite(ledPin, LOW);
    animation.detach(); //stop calling this callback
    Serial.println("Animation end");
  }
  animationPhase--;
}
