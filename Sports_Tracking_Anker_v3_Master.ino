#include <SPI.h>
#include <DW1000.h>

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 4;

const uint16_t NETWORK_ID = 10;
const uint8_t DEVICE_ADDRESS = 1;


volatile boolean sentAck = false;
volatile boolean received = false;
volatile boolean error = false;
String receivedString;

void setup() {
  Serial.begin(9600);

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

void receiver() {
  DW1000.newReceive();
  DW1000.setDefaults();
  DW1000.receivePermanently(true);
  DW1000.startReceive();
}

DW1000Time previousTime;
void loop() {
    // DW1000Time currentTime = DW1000Time((int64_t)micros());
    DW1000.newTransmit();
    DW1000Time currentTime;
    DW1000.getSystemTimestamp(currentTime);
    DW1000Time timeDifference = currentTime - previousTime;
    String timeStr = String(currentTime.getTimestamp());
    DW1000.setData(timeStr);
    DW1000.startTransmit();
    
    previousTime = currentTime;

    Serial.print("timeDifference : ");
    Serial.println(timeDifference.getAsMicroSeconds());

    delay(1000); // send every 1 second
}

