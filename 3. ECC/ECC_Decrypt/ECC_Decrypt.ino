#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <tinyECC.h>

#define SSID "ra.thor.e_"          // Replace with your WiFi SSID
#define PSWD "08080808"            // Replace with your WiFi password
#define MQTT_SERVER "192.168.137.120" // Replace with your MQTT broker IP
#define RANDOM_STRING_LENGTH 10
#define EEPROM_SIZE 64             // Size in bytes
#define MAGIC_NUMBER 123
#define INIT_FLAG_ADDRESS 0
#define SEQ_NUM_ADDRESS 4

WiFiClient espClient;
PubSubClient client(espClient);
tinyECC ecc;

static unsigned int seqNum;

void setup_wifi() {
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSWD);
  Serial.begin(115200);
  delay(500);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  Serial.println("");
  Serial.print("Connected to - ");
  Serial.println(SSID);
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 10 seconds");
      delay(10000);
    }
  }
}

void setup_flash_memory() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(INIT_FLAG_ADDRESS) != MAGIC_NUMBER) {
    EEPROM.write(INIT_FLAG_ADDRESS, MAGIC_NUMBER);
    EEPROM.put(SEQ_NUM_ADDRESS, seqNum);
    EEPROM.commit();
  } else {
    EEPROM.get(SEQ_NUM_ADDRESS, seqNum);
  }
}

String generateRandomString(char *output, int length) {
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  for (int i = 0; i < length; i++) {
    int index = random(0, sizeof(charset) - 1);
    output[i] = charset[index];
  }
  output[length] = '\0';
  return output;
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setBufferSize(1024);
  setup_flash_memory();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  static unsigned long lastMsg = 0;

  if (now - lastMsg > 30000) {
    lastMsg = now;

    char randomString[RANDOM_STRING_LENGTH + 1];
    String plaintext = String(seqNum) + " // Hello from ESP8266 // " + generateRandomString(randomString, RANDOM_STRING_LENGTH);

    // Encrypt the message
    ecc.plaintext = plaintext;
    ecc.encrypt();
    String encryptedMessage = ecc.ciphertext;

    // Publish to the "ecc" topic
    Serial.print("Publishing encrypted message to 'ecc': ");
    Serial.println(encryptedMessage);

    if (client.publish("ecc", encryptedMessage.c_str())) {
      Serial.println("Message published successfully.");
    } else {
      Serial.println("Message failed to publish.");
    }

    seqNum++;
    EEPROM.put(SEQ_NUM_ADDRESS, seqNum);
    EEPROM.commit();
  }
}
