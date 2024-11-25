#include<ESP8266WiFi.h>
#include <EEPROM.h>
#include <PubSubClient.h>

#define SSID "ra.thor.e_"
#define PSWD "08080808"
#define IP "192.168.25.120"
#define RANDOM_STRING_LENGTH 10
#define EEPROM_SIZE 64 //size in bytes
#define MAGIC_NUMBER 123
#define INIT_FLAG_ADDRESS 0
#define SEQ_NUM_ADDRESS 4

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  // put your setup code here, to run once:
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID,PSWD);
  Serial.begin(115200);
  delay(500);
  while(WiFi.status() != WL_CONNECTED){
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
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe to a topic
      client.subscribe("test/topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 10 seconds");
      delay(10000);
    }
  }
}

static unsigned int seqNum;


void setup_flash_memory(){

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  // static unsigned int seqNum = 0;

  // Check if EEPROM is initialized
  if (EEPROM.read(INIT_FLAG_ADDRESS) != MAGIC_NUMBER) {
    // EEPROM is not initialized; set sequence number to 0
    EEPROM.write(INIT_FLAG_ADDRESS, MAGIC_NUMBER); // Write the magic number to indicate initialization
    EEPROM.put(SEQ_NUM_ADDRESS, seqNum); // Write the initial sequence number
    EEPROM.commit(); // Commit changes
  } else {
    // EEPROM is initialized; read the sequence number
    EEPROM.get(SEQ_NUM_ADDRESS, seqNum); // Retrieve the sequence number
  }

}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(IP, 1883);
  client.setCallback(callback);
  setup_flash_memory();
}


String generateRandomString(char *output, int length) {
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  for (int i = 0; i < length; i++) {
    int index = random(0, sizeof(charset) - 1);  // Pick a random index
    output[i] = charset[index];
  }
  output[length] = '\0';  // Null-terminate the string
  return output;
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long startTime = micros();

  // Publish a message every 60 seconds

  char randomString[RANDOM_STRING_LENGTH + 1];  // +1 for the null terminator

  static unsigned long lastMsg = 0;
  
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    

    String msg = (String)seqNum+" // Hello from ESP8266 // Random text starts from here -  "+generateRandomString(randomString, RANDOM_STRING_LENGTH);

    seqNum++;
    EEPROM.put(SEQ_NUM_ADDRESS, seqNum); // Store the updated sequence number
    EEPROM.commit(); // Commit changes

    unsigned long elapsedTime = micros() - startTime;
    
    Serial.print("Publishing message: ");
    Serial.println(msg);
    client.publish("test/topic", msg.c_str());

    

    // Print the execution time
    Serial.print("Execution time: ");
    Serial.print(elapsedTime);
    Serial.println(" microseconds");
  }
}