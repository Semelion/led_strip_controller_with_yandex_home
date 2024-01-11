#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define ssid  "" //write your SSID
#define password  "" //write password
#define mqtt_server "" //url or ip mqtt server
#define mqtt_port 1883 //port for mqtt server

#define LAMP_STATE_TOPIC "kitchen/lamp/state"
#define LAMP_POWER_TOPIC "kitchen/lamp/power"

#define pinLed 5
#define buttonPin 16
#define button_LED_Pin 4

bool state = false;
int power = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String input = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    input += (char)payload[i];
  }
  Serial.println();
  if (String(topic) == LAMP_STATE_TOPIC) {
    state = input.toInt();
    analogWrite(pinLed, power*state);
    digitalWrite(button_LED_Pin, !state);
  }

  if (String(topic) == LAMP_STATE_TOPIC) {
    power = map(input.toInt(), 0, 100, 0, 255);
    analogWrite(pinLed, power * state);
    digitalWrite(button_LED_Pin, !state);
  }

}

void reconnect() {
  // Loop until we're reconnected
  if(WiFi.status() != WL_CONNECTED)
    setup_wifi();

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(LAMP_STATE_TOPIC);
      client.subscribe(LAMP_POWER_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(button_LED_Pin, OUTPUT);

  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (digitalRead(buttonPin) == HIGH) {
    while (digitalRead(buttonPin) == HIGH);
    Serial.print("Rbutton:   ");
    Serial.println(state);
    if (state == true)
      client.publish("semelion/kitchen/lamp/state", "0");
    else
      client.publish("semelion/kitchen/lamp/state", "1");
    delay(300);
  }

}
