/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 * Inspired by the work of:
 * Niko at https://nkmakes.github.io/2020/09/02/esp32-tank-robot-joystick-http-web-control/
 * @baptistelaget at https://medium.com/@baptistelaget/controlling-disneys-droids-from-droid-depots-with-webbluetooth-febbabe50587
 * This code to help me use callback functions instead of the heavy lifting in the loop: https://microcontrollerslab.com/esp32-websocket-server-arduino-ide-control-gpios-relays/#Importing_Libraries
 * This code to help me send the video out: https://github.com/robotzero1/esp32cam-rc-car/blob/main/esp32cam-rc-car.ino
 * Just for reference but a nice rewrite and maintained version of the esp32-cam demo code: https://github.com/easytarget/esp32-cam-webserver
 * Another Reference code for the esp32-cam but focuses on the streaming of the picture not using sockets : https://github.com/alanesq/esp32cam-demo
 * The electrical and pinout schematic of the esp32-cam: https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/
 * This was a rant by a user saying that the platform.io libraries are further behind than the arduino libraries and were unreliable. I don't know how that's possible.  I had to agree https://github.com/me-no-dev/ESPAsyncWebServer/issues/900 with this because the libraries in platform.io would crash the esp32
 * 
 * On the hardware side, I would not have gotten off the ground without the help of https://technoreview85.com/how-to-program-esp-32-cam-using-arduino-uno-board/ to help get my esp32-cam hooked up 
 *    without a FDTI.  I have a fully isolated, transformer seperated DSO Tech SH098C3 FDTI for maximum safety while I was doing energy monitoring development but that wouldn't deliver enough power to run the esp32-cam at the 5 volt side (recommended) did allow it to boot at 3.3v with lots of video noise.
 * 
 */

#include "BLEDevice.h"
//#include "BLEScan.h"
//#include <ArduinoWebsockets.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoOTA.h>
#include "web.h"
#include "esp_camera.h"


#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
 
extern int max_ws_clients= 3;
extern int max_ws_queued_messages=3;

camera_fb_t * fb = NULL;

WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

//using namespace websockets;
AsyncWebSocket server("/ws");
AsyncWebServer webserver(80);

int LValue, RValue, commaIndex;
int oldLValue = 0, oldRValue = 0;
const byte led = 33;



// The remote service we wish to connect to.
static BLEUUID serviceUUID("09b600A0-3e42-41fc-b474-e9c0c8f0c801");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("09b600b1-3e42-41fc-b474-e9c0c8f0c801");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
//const fromHexString = hexString =>
//            new Uint8Array(hexString.replace(/\s+/g, '').match(/.{1,2}/g).map(byte => parseInt(byte, 16)));

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
    digitalWrite(led, 1);
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    //pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
    std::map<std::string, BLERemoteService*>* pServices = pClient->getServices();
    Serial.println("Services : " + String(pServices->size()));
    for (std::map<std::string, BLERemoteService*>::iterator it = pServices->begin(); it != pServices->end(); ++it) {
    Serial.println(it->first.c_str());
  }
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");
    std::map<std::string, BLERemoteCharacteristic*>* pCharacteristics = pRemoteService-> getCharacteristics();
     Serial.println(" - Found our BLERemoteCharacteristics");
    for (std::map<std::string, BLERemoteCharacteristic*>::iterator it = pCharacteristics->begin(); it != pCharacteristics->end(); ++it) {
    Serial.println(it->first.c_str());
    //Serial.println(pRemoteService->toString().c_str());
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    //pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    pRemoteCharacteristic = pRemoteService->getCharacteristic(it->first);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.print(it->first.c_str());
    Serial.println(" - Found our characteristic");
    Serial.println(pRemoteCharacteristic->toString().c_str());
    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

static    byte pair[3] = {0x22,0x20,0x01};
static    byte cmd[8] =     {0x27,0x42,0x0F,0x44,0x44,0x00,0x1F,0x00};
static    byte cmd2[8] =     {0x27,0x42,0x0F,0x44,0x44,0x00,0x18,0x02};
//static    byte moveit[10] = {0x29,0x42,0x05,0x46,0x80,0xFF,0x01,0x2C,0x00,0x00};
    if(pRemoteCharacteristic->canWrite()){
    pRemoteCharacteristic->writeValue(pair,3, 1);
    delay(500);
    pRemoteCharacteristic->writeValue(pair,3, 1);
    delay(500);
    pRemoteCharacteristic->writeValue(pair,3, 1);
    delay(500);
    pRemoteCharacteristic->writeValue(pair,3, 1);
    delay(500);
    pRemoteCharacteristic->writeValue(cmd,8, 1);
    delay(100);
    pRemoteCharacteristic->writeValue(cmd2,8, 1);
    delay(300);   
    //pRemoteCharacteristic->writeValue(moveit,10, 1);
    delay(100); 
    /*    
    pRemoteCharacteristic->writeValue((byte)0x22, 0);
    pRemoteCharacteristic->writeValue((byte)0x20, 0);
    pRemoteCharacteristic->writeValue((byte)0x01, 1);
    delay(500);
    pRemoteCharacteristic->writeValue((byte)0x22, 0);
    pRemoteCharacteristic->writeValue((byte)0x20, 0);
    pRemoteCharacteristic->writeValue((byte)0x01, 1);
    delay(500);         
    pRemoteCharacteristic->writeValue((byte)0x22, 0);
    pRemoteCharacteristic->writeValue((byte)0x20, 0);
    pRemoteCharacteristic->writeValue((byte)0x01, 1);
    delay(500);
   
    Serial.println("Playing Sound");
    pRemoteCharacteristic->writeValue((byte)0x27, 0);
    pRemoteCharacteristic->writeValue((byte)0x42, 0);
    pRemoteCharacteristic->writeValue((byte)0x0F, 0);
    pRemoteCharacteristic->writeValue((byte)0x44, 0);
    pRemoteCharacteristic->writeValue((byte)0x44, 0);
    pRemoteCharacteristic->writeValue((byte)0x0, 0);
    pRemoteCharacteristic->writeValue((byte)0x1F, 0);
    pRemoteCharacteristic->writeValue((byte)0x0, 1);
    delay(10);
    /*
    pRemoteCharacteristic->writeValue(0x27, 0);
    pRemoteCharacteristic->writeValue(0x42, 0);
    pRemoteCharacteristic->writeValue(0x0F, 0);
    pRemoteCharacteristic->writeValue(0x44, 0);
    pRemoteCharacteristic->writeValue(0x44, 0);
    pRemoteCharacteristic->writeValue(0x00, 0);
    pRemoteCharacteristic->writeValue(0x18, 0);
    pRemoteCharacteristic->writeValue(0x02, 0);
    delay(500);
*/
    Serial.println("Finished Characteristic Writing");
    }
  }
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    digitalWrite(led, 0);
    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
     Serial.print(advertisedDevice.getName().c_str());
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    //if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      if (!strcmp(advertisedDevice.getName().c_str(),"DROID")){
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void notifyClients() {
  //server.textAll("data coming out");
  //server.binaryAll({0x31,0x32});
  //send picture
  fb = esp_camera_fb_get();    
  server.binaryAll(fb->buf, fb->len);  
  //Serial.print("Length ");
  //Serial.println(fb->len);
  //for (int i = 0; i < 5000; i++){ // dump the pixels' value
  //Serial.print(fb->buf[i],HEX);  
  //}
  esp_camera_fb_return(fb);
}

byte moveL[10] = {0x29,0x42,0x05,0x46,0x80,0xFF,0x01,0x2C,0x00,0x00};
byte moveR[10] = {0x29,0x42,0x05,0x46,0x80,0xFF,0x01,0x2C,0x00,0x00};
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    //Serial.print((char*)data);
    //Serial.println(" Received Data");
    
    //  Serial.println("handling_message");
    commaIndex = String((char*)data).indexOf(',');
    LValue = String((char*)data).substring(0, commaIndex).toInt();
    RValue = String((char*)data).substring(commaIndex + 1).toInt();
    if (oldLValue != LValue || oldRValue != RValue){
    notifyClients();  
    Serial.print(LValue);
    Serial.print("L - R");
    Serial.println(RValue);
    if (LValue < 0) {
      moveL[4] =  0x80;
      moveL[5] =  -LValue;
    } else{
      moveL[4] =  0x00;
      moveL[5] =  LValue;
    }
  
    if (RValue < 0) {
      moveR[4] =  0x81;
      moveR[5] =  -RValue;
    } else{
      moveR[4] =  0x01;
      moveR[5] =  RValue;
    }
    if (connected){
      oldLValue = LValue;
      oldRValue = RValue;
    pRemoteCharacteristic->writeValue(moveL,10, 1);
    pRemoteCharacteristic->writeValue(moveR,10, 1);
    }
  //  motor1.drive(LValue);
  //  motor2.drive(RValue);
    }
 
//    if (strcmp((char*)data, "toggle") == 0) {
//      //GPIO_State = !GPIO_State;
//      notifyClients();
//    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  server.onEvent(onEvent);
  webserver.addHandler(&server);
}


// handle http messages
//void handle_message(WebsocketsMessage msg) {
//  Serial.println("handling_message");
//  if (oldLValue != LValue || oldRValue != RValue){
//  commaIndex = msg.data().indexOf(',');
//  LValue = msg.data().substring(0, commaIndex).toInt();
//  RValue = msg.data().substring(commaIndex + 1).toInt();
//  Serial.print(LValue);
//  Serial.print("L - R");
//  Serial.println(RValue);
//  if (LValue < 0) {
//    moveL[4] |=  0x80;
//    moveL[5] =  -LValue;
//  } else{
//    moveL[4] |=  0x00;
//    moveL[5] =  LValue;
//  }
//
//  if (RValue < 0) {
//    moveR[4] |=  0x81;
//    moveR[5] =  -LValue;
//  } else{
//    moveR[4] |=  0x01;
//    moveR[5] =  LValue;
//  }
//  if (connected){
//    oldLValue = LValue;
//    oldRValue = RValue;
//  pRemoteCharacteristic->writeValue(moveL,10, 1);
//  delay(50);
//  pRemoteCharacteristic->writeValue(moveR,10, 1);
//  }
////  motor1.drive(LValue);
////  motor2.drive(RValue);
//  }
//}



void setup() {
    camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  //if (psramFound()) {
    //config.frame_size = FRAMESIZE_UXGA;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
 // } else {
    //config.frame_size = FRAMESIZE_SVGA;
 //   config.frame_size = FRAMESIZE_QVGA;
 //   config.jpeg_quality = 12;
 //   config.fb_count = 1;
 // }
   
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
 
  sensor_t * s = esp_camera_sensor_get();
  //s->set_framesize(s, FRAMESIZE_SVGA);
  s->set_framesize(s, FRAMESIZE_QVGA);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  
  Serial.println('\n');

  wifiMulti.addAP("******", "*****");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("******", "*****");
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  
  ArduinoOTA.setHostname("ESP32droidblecam");
  ArduinoOTA.setPassword("******");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");


  
  initWebSocket();
  // HTTP handler assignment
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, sizeof(index_html_gz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  // start server
  webserver.begin();
  Serial.print("Is server live? ");
//  Serial.println(server.available());



Serial.println("Leaving Setup function.");
} // End of setup.

unsigned long previousTime = millis();

const unsigned long interval = 250;

// This is the Arduino main loop function.
void loop() {

    unsigned long diff = millis() - previousTime;
  if(diff > interval) {
      fb = esp_camera_fb_get();    
      server.binaryAll(fb->buf, fb->len);  
      esp_camera_fb_return(fb);
      previousTime += diff;
  }
  ArduinoOTA.handle();
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
      Serial.println("About to run the connecttoServer function.");
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    //String newValue = "Time since boot: " + String(millis()/1000);
    //Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
   // pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  //delay(1000);
  //Serial.println("Mainloop");
  server.cleanupClients();



/*  WebsocketsClient  client = server.accept();
  client.onMessage(handle_message);
  if (client.available()) {
    Serial.println("client available");
    client.poll();
      }
      */
} // End of loop
