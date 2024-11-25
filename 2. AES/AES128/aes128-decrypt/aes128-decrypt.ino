#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AESLib.h>
#include <Base64.h>

#define SSID "ra.thor.e_"            // Replace with your Wi-Fi SSID
#define PSWD "08080808"              // Replace with your Wi-Fi Password
#define BROKER_IP "192.168.137.120"  // Replace with your MQTT Broker IP
#define MQTT_PORT 1883               // Default MQTT port

WiFiClient espClient;
PubSubClient client(espClient);
AESLib aesLib;

#define INPUT_BUFFER_LIMIT 256
char cleartext[INPUT_BUFFER_LIMIT];   // Buffer for decrypted message
char base64_decoded[INPUT_BUFFER_LIMIT]; // Buffer for Base64-decoded data

byte aes_key[32] = {
    'T', 'h', 'i', 's', 'I', 's', 'A', '3', '2', 'C', 'h', 'a', 'r', 'L', 'o', 'n',
    'g', 'K', 'e', 'y', 'F', 'o', 'r', 'A', 'E', 'S', '2', '5', '6', '!', '!', '!'
};

byte aes_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void setup_wifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSWD);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(1000);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi. Halting...");
    while (true) { delay(1000); }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message received:");
  Serial.print("Topic: ");
  Serial.println(topic);

  // Start the timer
  unsigned long startTime = micros();
  Serial.println("Timer started.");

  // Check payload size
  if (length >= INPUT_BUFFER_LIMIT) {
    Serial.println("Payload too large for buffer!");
    return;
  }

  // Copy the payload into a buffer and null-terminate it
  char encrypted_message[length + 1];
  memcpy(encrypted_message, payload, length);
  encrypted_message[length] = '\0';
  Serial.println("Payload copied.");

  // Decode Base64
  int decoded_len = base64_decode(base64_decoded, encrypted_message, length);
  if (decoded_len <= 0) {
    Serial.println("Base64 decoding failed.");
    return;
  }
  if (decoded_len >= INPUT_BUFFER_LIMIT) {
    Serial.println("Decoded data exceeds buffer size!");
    return;
  }

  Serial.print("Decoded Length: ");
  Serial.println(decoded_len);

  // Decrypt the message
  Serial.println("Decrypting...");
  int decrypted_len = aesLib.decrypt(
      (byte*)base64_decoded, decoded_len, (byte*)cleartext, aes_key, sizeof(aes_key), aes_iv);

  if (decrypted_len > 0) {
    cleartext[decrypted_len] = '\0';
    Serial.print("Decrypted Message: ");
    Serial.println(cleartext);

    // End the timer
    unsigned long elapsedTime = micros() - startTime;
    Serial.print("Time taken (from fetch to decryption): ");
    Serial.print(elapsedTime);
    Serial.println(" microseconds");

    // Publish the decrypted message
    if (client.publish("decrypt_aes256", cleartext)) {
      Serial.println("Published decrypted message.");
    } else {
      Serial.println("Failed to publish decrypted message.");
    }
  } else {
    Serial.println("Decryption failed, but timer will still be calculated.");

    // Timer calculation for failure
    unsigned long elapsedTime = micros() - startTime;
    Serial.print("Fallback - Time taken (from fetch to decryption): ");
    Serial.print(elapsedTime);
    Serial.println(" microseconds");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP8266Subscriber")) {
      Serial.println("MQTT connected");
      client.subscribe("AES256");
      Serial.println("Subscribed to topic AES256");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(BROKER_IP, MQTT_PORT);
  client.setCallback(callback);
  aesLib.set_paddingmode((paddingMode)0); // Ensure consistent padding
  Serial.println("Setup completed.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
