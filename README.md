ATTENTION, STARTS WITH SCREEN OFF WAITING FOR DATA!!!

----------------------------------------------------

Last Update - 01/02/2024:
	
     Fix hard flash screen turn on (also need to comment in file TFT_eSPI.cpp line 789 and 796)

----------------------------------------------------

24/12/2023:
	
 Monitor.py:

    connection rx and tx now you have volume controle, add 2 timers


arduino:

    add second menu, add 2 timers, add volume controle

----------------------------------------------------

20/12/2023:
	
 Monitor.py:
 		
    extern file COM.txt to change PORT COM (order; COM RX, enter, COM TX);
 		
     more fast update data;
  		
    trying to do connection rx and tx.
 
----------------------------------------------------

16/12/2023:

    simplify code, fix wait_time, fix blink screen, add second background for menu (finished)

----------------------------------------------------

15/12/2023:

     A lot of changes, add second background for menu (testing)

----------------------------------------------------

09/12/2023:
	
     Add ledBrightness touch for inc or dec level (save on EEPROM), add sleep for 1 day touch option (not auto)
 
----------------------------------------------------

08/12/2023:
	
     Add deep sleep 1min when bt not receive data (wakes every 1min after for check data)

----------------------------------------------------

01/12/2023:

    Arduino code without graphshowing and this code have:
	Touch left top turn on screen, touch right top turn off screen, when not turned off screen with touch or reset the screen turn auto on if data it not null, better performance data show and timers. 
 
	For all arduino code background multioption with better quality.

----------------------------------------------------
14/08/2023:
	
     https://github.com/manoper93/MonitorPC_WT32-SC01/assets/70864023/4122fc66-a44e-4120-bc2f-43a21ca1c50c

--------------------------
First Update - 05/03/2023:

![20230221_195041aa](https://user-images.githubusercontent.com/70864023/222975950-0b791d5f-fe44-46d5-a8be-7de803ef2561.jpg)

----------------------------------------------------


MonitorPC with support WT32-SC01

FOLLOW THE STEPS TO INSTALL THE PC MONITOR:

    1. Install python 3.x and Arduino IDE 1.8.19 (not > v2);

    2. Open CMD and install the python requirements ;

    3. Add "Esp8266 Filesystem Uploader " to Arduino , instructions:
	https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/

    4. Open Arduino and install libraries:
	Adafruit -GFX- Library
	TFT_eSPI
	ESPAsyncWebserver
	asyncTCP
	ArduinoJson
	
	If you use capacitive touch :
		https://github.com/DustinWatts/FT6236

    5. Replace the user_setup.h in TFT_eSPI with the one according to your device from the location " user_setup.h Examples ", with the same original name " user_setup.h ";

    6. Open "BT-SystemMonitor_-_Manoper-Edition.ino " (this one dedicated to WT32-SC01) if you want the original one (https://github.com/DustinWatts/Bluetooth-System-Monitor) and edit for your device, like also the levels of WARNING (RED) and LIMITS of the data, further down you can change the symbols and the graphics text menu;

    7. Install device on Arduino (https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/) and the selected device must be "esp32 Dev Module" without editing parameters, ONLY Partition Scheme - "NO OTA (2MB APP, 2MB SPIFFS)";

    8. Compile and submit;

    9. Upload the images (bg.png) using the "Esp32 Sketch Data Upload" button;


UNTIL HERE ARDUINO IS DONE

--------------------------

    10. In the PYTHON FILE (monitor.py) edit your COM port with a text editor, or other parameters that you want to modify to send to the device (to execute: CMD, in the respective place, and command " python monitor.py " ;

    11. The file start_LibreHardwareMonitor.bat executes the program necessary for collecting the information, it starts the program (from the folder "\ LibreHardwareMonitor " at the root) to avoid authentications at all times, open the program and set it to start on boot , minimized on close and on startup;

    12. Now let's create the .EXE file, In the CMD go to the folder "\TOOLS\auto- py -to-exe-master", and start the command " python run.py" (DO NOT CLOSE CMD);

    13. In the program " Auto Py to Exe" in Script Location select the file "monitor.py";

    14. Then select " One Directory "( One File gives error );

    15. Then select "Windows Based ( hide the console)";

    16. Then select Icon ;

    17. Then select the Add button in Additional Files Folder the folder " LibreHardwareMonitor " ok, and in the Add Files the "start_LibreHardwareMonitor.bat"

    18. Leave the remaining options as they are;

    19. Now button CONVERT .PY TO .EXE, after finished it will be in an OUTPUT folder;

    20. Copy and paste in to Windows Programs, and in "monitor.exe" select copy and in the location " C:\Users\**USERNAME**\AppData\Roaming\Microsoft\Windows\Start Menu\ Programs \" Paste as a link, and in the location "C:\Users\**USERNAME**\AppData\Roaming\Microsoft\Windows\Start Menu\ Programs \ Startup " Paste the same as a link;

    21. To finish opening Task Manager in the Startup Applications location will be the respective program created and it is your choice to leave autoboot or not.



----------------------------------------------------

NOW:

![20231216_165225](https://github.com/manoper93/MonitorPC_WT32-SC01/assets/70864023/5e0b145b-2e3d-4ac8-bada-682d0c357439)
![20231224_153449](https://github.com/manoper93/MonitorPC_WT32-SC01/assets/70864023/81d554e5-765b-45ec-88be-b316deb17acc)
![20231224_153456](https://github.com/manoper93/MonitorPC_WT32-SC01/assets/70864023/5eaaf4bc-2907-4faf-8a2e-b04c7dccf877)

----------------------------------------------------


