#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <tinyECC.h>
#include <Base64.h>

#define SSID "ra.thor.e_"
#define PSWD "08080808"
#define IP "192.168.137.120"
#define RANDOM_STRING_LENGTH 5
#define EEPROM_SIZE 64
#define MAGIC_NUMBER 123
#define INIT_FLAG_ADDRESS 0
#define SEQ_NUM_ADDRESS 4

WiFiClient espClient;
PubSubClient client(espClient);
tinyECC ecc;

void setup_wifi() {
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("Connected to WiFi.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("test/topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(IP, 1883);
  client.setCallback(callback);
  client.setBufferSize(1024);  // Increase buffer size
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();

  if (now - lastMsg > 30000) {
    lastMsg = now;

    // Monitor free heap before encryption
    Serial.print("Free Heap (Before): ");
    Serial.println(ESP.getFreeHeap());

    // Encrypt the message
    ecc.plaintext = "Sample message for encryption";
    ecc.encrypt();

    // Yield to prevent watchdog resets
    yield();

    // Encode ciphertext as Base64
    String encodedCiphertext = base64::encode((const uint8_t*)ecc.ciphertext.c_str(), ecc.ciphertext.length(), false);

    // Monitor free heap after encryption
    Serial.print("Free Heap (After): ");
    Serial.println(ESP.getFreeHeap());

    // Check payload length
    if (encodedCiphertext.length() > 256) {
      Serial.println("Encoded message too long to publish.");
      return;
    }

    // Publish the message
    if (client.publish("test/topic", encodedCiphertext.c_str())) {
      Serial.println("Message published successfully.");
    } else {
      Serial.println("Failed to publish message.");
    }

    // Yield again if needed
    yield();
  }
}
