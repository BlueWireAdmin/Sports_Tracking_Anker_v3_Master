#include <SPI.h>
#include <DW1000.h>
#include <WiFi.h>
#include <ArduinoJson.h>

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 4;

const uint16_t NETWORK_ID = 10;
const uint8_t DEVICE_ADDRESS = 1;

const char* server = "192.168.87.69";  // Replace with your server IP
const int port = 5005;

const char* ssid = "Bredesande4";
const char* password = "sommerhus2020";
WiFiClient client;

volatile boolean sentAck = false;
volatile boolean received = false;
volatile boolean error = false;
String receivedString;

struct TDOAData {
  uint8_t tagID;
  DW1000Time arrivalTime;
  float rssi;
};

void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());

  // connectToServer();

  DW1000.begin(PIN_IRQ, PIN_RST);
  DW1000.select(PIN_SS);

  DW1000.newConfiguration();
  DW1000.setDefaults();
  DW1000.setDeviceAddress(DEVICE_ADDRESS);
  DW1000.setNetworkId(NETWORK_ID);

  // DW1000.setPreambleLength(DW1000.TX_PREAMBLE_LEN_256);
  // DW1000.setDataRate(DW1000.TRX_RATE_6800KBPS);
  // DW1000.setPulseFrequency(DW1000.TX_PULSE_FREQ_64MHZ);

  DW1000.commitConfiguration();
  DW1000.attachReceivedHandler(handleReceive);
  DW1000.attachReceiveFailedHandler(handleError);
  DW1000.attachErrorHandler(handleError);
  // DW1000.attachSentHandler(handleSent);
  receiver();
}


void handleReceive() {
  receivedString = "";
  byte dataSize = DW1000.getDataLength();  // Få størrelsen på de modtagne data



  if (dataSize > 255) {
    // Hvis data er for stor, håndter fejlen eller returnér
    Serial.println("Data size exceeds buffer limit.");
    return;
  }

  byte data[255];                  // Allokér en fast størrelse på 255 bytes
  DW1000.getData(data, dataSize);  // Læs data til byte array

  // Konverter byte array til String
  for (int i = 0; i < dataSize; i++) {
    receivedString += (char)data[i];
  }

  received = true;
}


void handleError() {
  error = true;
}

// void handleSent() {
//   sentAck = true;
// }

void receiver() {
  DW1000.newReceive();
  DW1000.setDefaults();
  DW1000.receivePermanently(true);
  DW1000.startReceive();
}


void transmitter() {
  DW1000.newTransmit();
  DW1000Time timestamp;
  DW1000.getSystemTimestamp(timestamp);
  // DW1000.getTransmitTimestamp(timestamp);
  uint64_t timestampValue = timestamp.getTimestamp();  // Tidsstempel i nanoseconds
  String msg = "{\"tagID\":" + String(DEVICE_ADDRESS) + ",\"packNum\":" + String(timestampValue) + "}";
  DW1000.setData(msg);
  DW1000.startTransmit();
  // DW1000.newTransmit();
  // DW1000.setDefaults();

  // // Use a buffer to hold the message data
  // char msg[50];
  // snprintf(msg, sizeof(msg), "{\"tagID\":%d,\"packNum\":%d}", DEVICE_ADDRESS, sentNum);

  // DW1000.setData(reinterpret_cast<uint8_t*>(msg), strlen(msg));
  // DW1000.startTransmit();
}

DW1000Time previousTime;
void loop() {

  // if (sentAck) {
  // sentAck = false;
  // sentNum++;
  // transmitter();
  // delay(1000);  // Tilføj forsinkelse for at sikre korrekt behandling

    // DW1000Time currentTime = DW1000Time((int64_t)micros());
    DW1000Time currentTime;
    DW1000.getSystemTimestamp(currentTime);
    DW1000Time timeDifference = currentTime - previousTime;
    
    DW1000.newTransmit();
    String timeStr = String(currentTime.getTimestamp());
    DW1000.setData(timeStr);
    DW1000.startTransmit();
    
    previousTime = currentTime;

    Serial.print("timeDifference : ");
    Serial.println(timeDifference.getAsMicroSeconds());

    delay(1000); // send every 1 second

  // }
  // Send tidsstempel med jævne mellemrum




  // Hvis der modtages et pakke fra et TAG
  // if (received) {
  //   TDOAData data;

  //   // Print the received string for debugging
  //   Serial.println("Processing received string: " + receivedString);

  //   int tagID = extractValueFromJson(receivedString, "tagID");
  //   uint16_t packNum = extractValueFromJson(receivedString, "packNum");

  //   // Debug output for extracted values
  //   Serial.print("Extracted tagID: ");
  //   Serial.println(tagID);
  //   Serial.print("Extracted packNum: ");
  //   Serial.println(packNum);


  //   DW1000Time timestamp;
  //   try {
  //     DW1000.getReceiveTimestamp(timestamp);
  //     uint64_t timestamp = data.arrivalTime.getTimestamp();  // Få tidsstemplet som en uint64_t værdi
  //   } catch (...) {
  //     Serial.println("Error getting receive timestamp");
  //     received = false;
  //     return;
  //   }

  //   // Store the data and generate the JSON payload
  //   data.tagID = tagID;
  //   data.arrivalTime = timestamp;
  //   data.rssi = DW1000.getReceivePower();

  //   dataBuffer[bufferIndex++] = data;
  //   if (bufferIndex >= bufferSize) {
  //     // Opret JSON-string af alle datapakker i bufferen
  //     String jsonData = create_json_from_buffer(dataBuffer, bufferSize);

  //     // Send samlet JSON-data via UDP
  //     send_udp(&jsonData);

  //     // Nulstil bufferindeks for at starte en ny pakkeopsamling
  //     bufferIndex = 0;
  //   }
  //   received = false;
  // }
}

// Funktion til at sende tidsstempel
void sendTimestamp() {

  DW1000.newTransmit();
  DW1000Time timestamp;
  // DW1000.getSystemTimestamp(timestamp);
  DW1000.getTransmitTimestamp(timestamp);
  uint64_t timestampValue = timestamp.getTimestamp();  // Tidsstempel i nanoseconds
  String msg = "{\"tagID\":" + String(DEVICE_ADDRESS) + ",\"packNum\":" + String(timestampValue) + "}";
  DW1000.setData(msg);
  DW1000.startTransmit();
}
