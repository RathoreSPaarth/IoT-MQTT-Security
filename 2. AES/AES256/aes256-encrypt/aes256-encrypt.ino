#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <AESLib.h>
#include <Base64.h>  // Include Base64 library for encoding/decoding

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
AESLib aesLib;

#define INPUT_BUFFER_LIMIT 1024
char cleartext[INPUT_BUFFER_LIMIT] = {0};
char ciphertext[2 * INPUT_BUFFER_LIMIT] = {0};
char base64_encoded[3 * INPUT_BUFFER_LIMIT] = {0};  // For base64 encoding

byte aes_key[32] = {
    'T', 'h', 'i', 's', 'I', 's', 'A', '3', '2', 'C', 'h', 'a', 'r', 'L', 'o', 'n',
    'g', 'K', 'e', 'y', 'F', 'o', 'r', 'A', 'E', 'S', '2', '5', '6', '!', '!', '!'
};

byte aes_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
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
      Serial.println(" try again in 10 seconds");
      delay(10000);
    }
  }
}

void aes_init() {
  aesLib.set_paddingmode((paddingMode)0);
}

uint16_t encrypt_to_ciphertext(char* msg, uint16_t msgLen, byte iv[]) {
  Serial.println("Calling encrypt (string)...");
  int cipherlength = aesLib.get_cipher64_length(msgLen);
  aesLib.encrypt((byte*)msg, msgLen, (byte*)ciphertext, aes_key, sizeof(aes_key), iv);
  return cipherlength;
}

void setup_flash_memory() {
  EEPROM.begin(EEPROM_SIZE);
  static unsigned int seqNum;
  if (EEPROM.read(INIT_FLAG_ADDRESS) != MAGIC_NUMBER) {
    EEPROM.write(INIT_FLAG_ADDRESS, MAGIC_NUMBER);
    EEPROM.put(SEQ_NUM_ADDRESS, seqNum);
    EEPROM.commit();
  } else {
    EEPROM.get(SEQ_NUM_ADDRESS, seqNum);
  }
}

String generateRepeatedString(char* output, int n) {
  const char* baseString = "BITS Pilani";
  int baseLength = strlen(baseString);
  int totalLength = n * baseLength;

  for (int i = 0; i < n; i++) {
    strcpy(output + i * baseLength, baseString);
  }
  
  output[totalLength] = '\0';  // Null-terminate the final string
  return String(output);
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(IP, 1883);
  client.setCallback(callback);
  setup_flash_memory();

  aes_init();  // Initialize AES with padding mode
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 15000) {
    lastMsg = now;



    char randomString[RANDOM_STRING_LENGTH + 1];
    static unsigned int seqNum;
    String msg = "/////////////" +String(seqNum) + " // Hello from ESP8266 // " + generateRepeatedString(randomString, RANDOM_STRING_LENGTH);

    Serial.print("Message to be Sent: ");
    Serial.println(msg);
    // Convert the message to cleartext buffer
    sprintf(cleartext, "%s", msg.c_str());

    // Start time measurement
    unsigned long startTime = micros();

    
    // Measure free heap at the start
    // size_t freeHeapStart = ESP.getFreeHeap();
    // Serial.print("Free Heap (Start): ");
    // Serial.println(freeHeapStart);

    // Encrypt the message
    uint16_t cipher_len = encrypt_to_ciphertext(cleartext, msg.length(), aes_iv);

    // Base64 encode the ciphertext
    int encoded_len = base64_encode(base64_encoded, (char*)ciphertext, cipher_len);

    Serial.print("Base64 Encrypted message: ");
    Serial.println(base64_encoded);

    // Publish the base64 encoded encrypted message
    if (client.publish("AES256", base64_encoded, encoded_len)) {
      Serial.println("Message published successfully.");
    } else {
      Serial.println("Message failed to publish. state - " + client.state());
    }

    // Measure free heap at the end
    // size_t freeHeapEnd = ESP.getFreeHeap();
    // Serial.print("Free Heap (End): ");
    // Serial.println(freeHeapEnd);

    // Calculate heap difference
    // Serial.print("Heap Usage (Encryption + Publish): ");
    // Serial.println(freeHeapStart - freeHeapEnd);

    // End time measurement
    unsigned long elapsedTime = micros() - startTime; // this value changes if i include heap calculation, becomes around 12245 microseconds. else it is 4800 microseconds.
    Serial.print("Execution time (Encryption + Publish): ");
    Serial.print(elapsedTime);
    Serial.println(" microseconds");

    seqNum++;
    EEPROM.put(SEQ_NUM_ADDRESS, seqNum);
    EEPROM.commit();
  }
}
