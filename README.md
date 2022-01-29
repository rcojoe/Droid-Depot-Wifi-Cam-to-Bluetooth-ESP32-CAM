# Droid-Depot-Wifi-Cam-to-Bluetooth-ESP32-CAM

/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 * updated by Joe Schalk
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
![IMG_2032](https://user-images.githubusercontent.com/11729742/151645617-78e3cadb-89b6-4be4-b4de-3f733bf52c82.JPEG)
![IMG_2033](https://user-images.githubusercontent.com/11729742/151645622-2ab68d9e-c23a-4fb4-9654-79bde963a919.JPEG)
![IMG_2034](https://user-images.githubusercontent.com/11729742/151645624-cf83d0a1-0cf6-402c-a342-aef8ff99347d.JPEG)


https://user-images.githubusercontent.com/11729742/151645639-a618f7a6-f5d4-4b2a-b467-744e52b01d3b.MP4

