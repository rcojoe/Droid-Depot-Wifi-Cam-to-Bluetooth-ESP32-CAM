# Droid-Depot-Wifi-Cam-to-Bluetooth-ESP32-CAM with OTA

So I came up and created this during a covid outbreak at my house for my kids.  They wanted a way to stay in their room but still come out and explore the house while they were in quarentine. We had been to the Droid Depot at Disney a few years earlier.  The kids made some droids while there and we had a great time with them, but then they didn't use them anymore, and were more center peices in their room until now.  

The first version had a baby monitor duct taped to R2's head.  The photo is below.  That worked well for two way communication but lacked any control after about 20 feet outside their door. The cons were the bluetooth range was poor using the native remote and the camera needed to be turned on and off each time because the baby monitor camera drained them after a few hours of being on.
Here is a movie of it:
https://user-images.githubusercontent.com/11729742/151645639-a618f7a6-f5d4-4b2a-b467-744e52b01d3b.MP4

The second version had a baby monitor on top but used a standard ESP32 for wifi to bluetooth communications.  It used the same code below,  without web based video.  This worked really well and is what we used during quarentine.  The cons was that the baby monitor was heavy and R2 was a bit top heavy with it mounted. The baby monitor took up the space of where we evetually duct taped a basket for passing items back and forth from the living room and back to their room, like snacks.  
The 2nd version was too heavy for the bb8, so use the 3rd version for them.

The 3rd version included the esp32-cam.  This came in from an order from amazon about 3 days before our quarentine ended, so we didn't get much time to test it out or perfect the coding, but it did allow a basket to be mounted on top, but the con is that it lacks 2 way communication onboard.  I'm sure there is a way to figure this out, but I didn't need to because shouting back and forth worked for the last couple days of a 10 day quarentine (imagine that).  

![IMG_2032](https://user-images.githubusercontent.com/11729742/151645617-78e3cadb-89b6-4be4-b4de-3f733bf52c82.JPEG)
![IMG_2033](https://user-images.githubusercontent.com/11729742/151645622-2ab68d9e-c23a-4fb4-9654-79bde963a919.JPEG)
![IMG_2034](https://user-images.githubusercontent.com/11729742/151645624-cf83d0a1-0cf6-402c-a342-aef8ff99347d.JPEG)

<br>
<br>
<br>
Setup is simple: <br>
<br>
Get the Aruduino IDE<br>
Download the code here.<br>
Set your home's wifi info on line 436 and if you have a repeater,  use it as an alternative wifi on the next line.<br>
Compile and send to the esp32-cam<br>
Let it boot.  <br>
If you have serial monitor hooked to it, then it will tell you the IP address<br>
The mdns hostname is set on line 452.  Its set to ESP32droidblecam.<br>
Thats about it.<br>
<br>
To hook it to a droid,<br>
Turn the remote off.  Turn the droid off for a few seconds and back on.<br>
Restart the ESP32 using the reset button or by power interuption.<br>
After about 10-15 seconds, the droid will play a little tune which means the bluetooth is hooked up.<br>
Using a browser (I used Chrome and Safari) go to the mdns address or the IP (I used the IP address). <br>
The website should quickly load and display the camera footage.  Use your finger or mouse to drag the green circle around the screen.<br>
<br>
Some notes for testing:<br>
Once finished programming using a usb cable,  leave it plugged into your computer to make sure it reboots with a good IP address and also connects to the droid.  The esp32 or esp32-cam does not have to be on the droid to contol it,  it just need to be within about 15 feet of it to use and test.  Once the bluetooth connection and wifi connection is verifed, then move it over to the droid and power it with the power bank like I did.  







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
 * This was a rant by a user saying that the platform.io libraries are further behind than the arduino libraries and were unreliable. I had to agree https://github.com/me-no-dev/ESPAsyncWebServer/issues/900 with this because the libraries in platform.io would crash the esp32.  The arduino IDE worked perfect.  
 * 
 * On the hardware side, I would not have gotten off the ground without the help of https://technoreview85.com/how-to-program-esp-32-cam-using-arduino-uno-board/ to help get my esp32-cam hooked up 
 *    without an FDTI.  I have a fully isolated, transformer seperated DSO Tech SH098C3 FDTI for maximum safety while I was doing energy monitoring development but that wouldn't deliver enough power to run the esp32-cam at the 5 volt side (recommended).  But did allow it to boot on the 3.3v rail with lots of video noise.
 * 
 */
 Remote control Web page with live shot
 ![IMG_1994](https://user-images.githubusercontent.com/11729742/151645690-0b275095-0885-48d2-a663-a3404c73fa00.PNG)


Todo (feel free to submit pull requets):
  1. Clean up the code to make it more readable.  This was a rushed job for me and single purposed, but is a great starting point for others.
  2. Work to improve the video refresh rate while controling the droid.  This seems to be javascript related on the browser side.  It appears the javascript loop is causing the video to freeze while your finger is actively on the button and there needs to be some kind of backend yeild to allow the video to catch up when in the joystick routine.
  3. Add turret controls for R2.
  4. Add some sound effects.
  5. impliment a keep alive heartbeat so the droid BLE doesn't disconnect after 5 minutes and go to sleep.





