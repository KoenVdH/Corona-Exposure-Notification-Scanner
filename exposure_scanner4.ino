/*
 * 
 * Koen Van den Heuvel
 * 20 November 2020
 * 
 * This radar app shows "covid trace" beacons transmitted around you.
 * The distance from the centre are related to the strength of the received signal.
 * 
 * THESE ARE NOT infections but people that have the corona alert app installed.
 * 
 * https://covid19-static.cdn-apple.com/applications/covid19/current/static/contact-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf?1
 *
*/

#include <M5Stack.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 10; //In seconds
BLEScan* pBLEScan;
int numberofParticipants = 0;
bool bParticipants[256];
bool all = true;
int rotation_angle = 0;
float sx = 0, sy = 0;
float sxx = 0, syy = 0;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

    // Function based on code from Alexey Shkurko: https://habr.com/ru/post/500208/
    uint8_t* findServiceData(uint8_t* data, size_t length, uint8_t* foundBlockLength) {
        uint8_t* rightBorder = data + length;
        while (data < rightBorder) {
            uint8_t blockLength = *data;
            if (blockLength < 5) {
                data += (blockLength+1);
                continue;
            }
            uint8_t blockType = *(data+1);
            uint16_t serviceType = *(uint16_t*)(data + 2);
            // 0xfd6f is the Service UUID for the Exposure Notification Service 
            if (blockType == 0x16 && serviceType == 0xfd6f) {
                *foundBlockLength = blockLength-3;
                return data+4;
            }
            data += (blockLength+1);
        }   
        return nullptr;
    }
  
    void onResult(BLEAdvertisedDevice advertisedDevice) {  

      String sAdvert = advertisedDevice.toString().c_str();
      
      // 0xfd6f is the Service UUID for the Exposure Notification Service 
      if (sAdvert.indexOf("fd6f") > 0){  
        
        //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
        //Serial.printf("Device Name: %s, TX: %d, RSSI: %d \n", advertisedDevice.getName().c_str(), advertisedDevice.getTXPower(), advertisedDevice.getRSSI());

        // draw black wiper beam
        sx = cos(rotation_angle*0.0174532925);
        sy = sin(rotation_angle*0.0174532925);
        sxx = sx*120+120;
        syy = sy*120+120;
        M5.Lcd.drawLine(120, 120, sxx, syy, TFT_BLACK);
        M5.Lcd.drawLine(120, 121, sxx, syy+1, TFT_BLACK);
        M5.Lcd.drawLine(121, 120, sxx+1, syy, TFT_BLACK);
        M5.Lcd.drawLine(121, 121, sxx+1, syy+1, TFT_BLACK);
        
        // draw black wiper dot
        //sx = cos(rotation_angle*0.0174532925);
        //sy = sin(rotation_angle*0.0174532925);
        //sxx = sx*-advertisedDevice.getRSSI()+120;
        //syy = sy*-advertisedDevice.getRSSI()+120;
        //M5.Lcd.fillCircle(sxx, syy, 5, TFT_BLACK);
        
        rotation_angle = rotation_angle + 1;
        if (rotation_angle == 360) {
          rotation_angle = 0;
        }

        // draw bull
        //M5.Lcd.fillCircle(120, 120, 10, TFT_GREEN);
        M5.Lcd.drawCircle(120, 120, 20, TFT_GREEN);
        M5.Lcd.drawCircle(120, 120, 50, TFT_GREEN);
        M5.Lcd.drawCircle(120, 120, 80, TFT_GREEN);
        M5.Lcd.drawCircle(120, 120, 110, TFT_GREEN);
        
        M5.Lcd.drawLine(120, 0, 120, 240, TFT_GREEN);
        M5.Lcd.drawLine(0, 120, 240, 120, TFT_GREEN);
        
        //M5.Speaker.tone(100-(advertisedDevice.getRSSI()*-4), 200);
        M5.Lcd.drawCircle(120, 120, (-advertisedDevice.getRSSI()-10)*1.3, TFT_GREEN);

        sxx = sx*((-advertisedDevice.getRSSI()-10)*1.3)+120;
        syy = sy*((-advertisedDevice.getRSSI()-10)*1.3)+120;
        M5.Lcd.fillCircle(sxx, syy, 5, TFT_RED);
        
        //if (advertisedDevice.haveServiceUUID()) Serial.printf("UUID: %s\n", advertisedDevice.getServiceUUID().toString().c_str());

        uint8_t* payload = advertisedDevice.getPayload();
        size_t payloadLength = advertisedDevice.getPayloadLength();

        uint8_t serviceDataLength=0;
        uint8_t* serviceData = findServiceData(payload, payloadLength, &serviceDataLength);

        if (serviceData == nullptr) {
            return;
        }
        
        // store the first byte of the rolling identifier into an array
        // this is a simplification to count the number of unique participants since only looking at the first byte
        bParticipants[serviceData[0]] = true;

        /*
        Serial.printf("Rolling Proximity Identifier: ");
        for (int index = 0; index < 16; index++) {
          Serial.printf("%02X ", serviceData[index]);
        }
        Serial.println("");

        Serial.printf("Associated Encrypted Metadata: ");
        for (int index = 16; index < 20; index++) {
          Serial.printf("%02X ", serviceData[index]);
        }
        Serial.println("");
        Serial.printf("---------------------------------------------------------------------------------------\n");
        */
    }     
   }
};

void setup() {
  M5.begin();
  Wire.begin();
  M5.Power.begin();

  // text print
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(GREEN);
  
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  // not clear what that last parameter is, looks like true gives probably every packet received and false reduces the nr. of devices to unique per scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), all); 
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  for (int i = 0; i < 256 ; i++){
    bParticipants[i] = false;
  }
}

void loop() { 
  // clear right side of the screen
  M5.Lcd.fillRect(240, 0, 80, 240, BLACK);
  M5.Lcd.drawRect(242, 2, 76, 236, GREEN);
  M5.Lcd.setTextSize(2);
  uint8_t bat = M5.Power.getBatteryLevel();
  M5.Lcd.setCursor(260,195);
  M5.Lcd.printf("Bat:");
  M5.Lcd.setCursor(260,215);
  M5.Lcd.printf("%d", bat);

  // count the number of unique participants (simplification)
  numberofParticipants = 0;
  for (int i = 0; i < 256 ; i++){
    if (bParticipants[i]){
      numberofParticipants++;
    }
  }
  
  // reset participants
  for (int i = 0; i < 256 ; i++){
    bParticipants[i] = false;
  }
  
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(255,10);
  M5.Lcd.printf("%d", numberofParticipants); 
  
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.print("Participants: ");
  Serial.println(numberofParticipants);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  Serial.println("--");
}
