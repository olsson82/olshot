# Target Sensor Code
This is the target sensor code. When uploading with the ISP programmer, select Arduino Nano as board and programmer USBasp, then under sketch use **upload with programmer**

Change **targID** and **sendID** to unique id numbers.

During test you can have **#define DEBUG** enabled for serial output. Not recommended when you are going to use the sensors live.

If you need to change the fail safe time change in **myDesiredTime**

During test have the **PALevel** set to **RF24_PA_MIN** and then you can use **RF24_PA_MAX** when you go live. When you have them close during test, it is recommended to have them on min.

## Your code
If you have other version of the target sensor, share them with us.
