#include "BluetoothSerial.h"
#include <TFT_eSPI.h>
#include <SPIFFS.h>
#include <FS.h>
#include <esp_sleep.h>
#include <EEPROM.h>
#include "Ticker.h"

#define ENABLE_CAP_TOUCH
//#define ENABLE_RES_TOUCH
//#define NO_TOUCH

#define CUSTOM_TOUCH_SDA 18
#define CUSTOM_TOUCH_SCL 19
#define touchInterruptPin GPIO_NUM_27

#ifdef ENABLE_CAP_TOUCH
#include <Wire.h>
#include <FT6236.h>
FT6236 ts = FT6236();
#endif

#define FILESYSTEM SPIFFS

#define CALIBRATION_FILE "/calibrationData"

TFT_eSPI tft = TFT_eSPI();
BluetoothSerial BTSerial;

boolean homescreen = true;

int wait_time = 0;

int touch_times = 0;
int screen_start = 0;
int screen_off = 0;
int background = 0;

RTC_DATA_ATTR int sleep_state = 3;
RTC_DATA_ATTR int ledBrightness = 30;
RTC_DATA_ATTR int dontsleep = 0;
int address = 0;

int MAX_CPU = 100; //Perc CPU
int MAX_RAM = 100; //Perc RAM
int MAX_CPU_TEMP = 120;  //Temp CPU
int MAX_GPU = 100;  //Perc GPU
int MAX_VRAM = 100;  //Perc GPU
int MAX_GPU_TEMP = 120;  //Temp GPU

int WARN_CPU = 75; //Perc CPU
int WARN_RAM = 75; //Perc RAM
int WARN_CPU_TEMP = 75; //Temp CPU
int WARN_GPU = 75; //Perc GPU
int WARN_VRAM = 75; //Perc GPU
int WARN_GPU_TEMP = 75;   //Temp GPU

int cpu[1];
int ram[1];
int cput[1];
int gpu[1];
int vram[1];
int gput[1];

int margin = 40;
int screenwidth = 480;
int screenheight = 320;

int volume = 0;

Ticker blueTimer;
Ticker touchTimer;
void blueCallback();
void touchCallback();

//--------------------------------------------------------------------------------------- VOID SETUP

void setup() {
  Serial.begin(115200);
  BTSerial.begin("SystemMonitorPC"); // You can change this name, but on MacOs make sure to match your port to this name in the script
  EEPROM.begin(512);

  EEPROM.get(address, ledBrightness);

  // If the data is not valid (e.g., first run), initialize yourVariable to a default value
  if (ledBrightness < 0 || ledBrightness > 254) {
    ledBrightness = 30;
    EEPROM.put(address, ledBrightness);
    EEPROM.commit();
  }
  
  // Begin regular Serial for debugging
  Serial.begin(115200);
  Serial.println("---------------------------------------------");
  Serial.println("|     Manoper - SystemMonitorPC edition     |");
  Serial.println("---------------------------------------------");
  Serial.println("Normal Serial begun.");

  // Begin our filesystem
  if (FILESYSTEM.begin()) {
    Serial.println("SPIFFS begun.");
  } else {
    Serial.println("SPIFFS failed!");
  }

#ifdef ENABLE_CAP_TOUCH
  //Begin the touchscreen
  if (ts.begin(40, CUSTOM_TOUCH_SDA, CUSTOM_TOUCH_SCL)) {
    Serial.println("Touchscreen begun.");
  } else {
    Serial.println("Touchscreen failed!");
  }
#endif

Serial.println("--------------------------------------- SETUP");
serialprint();

// Set up Touch timer to run every 500 milliseconds (0.5 seconds)
blueTimer.attach(0.4, blueCallback);
touchTimer.attach(0.001, touchCallback);
}

//--------------------------------------------------------------------------------------- VOID LOOP

void loop() {
}

void blueCallback() {
  if(BTSerial.available()) {
    bt();
  } else {
    no_bt();
  }
  
}

void touchCallback() {
  if(sleep_state == 0){
     touch();
   }
  
  BTSerial.println(volume);
  BTSerial.write(volume);
  volume = 0;
}

//--------------------------------------------------------------------------------------- VOID BT

void bt(){
    String cpu_s = BTSerial.readStringUntil(',');
    String ram_s = BTSerial.readStringUntil(',');
    String cput_s = BTSerial.readStringUntil(',');
    String gpu_s = BTSerial.readStringUntil(',');
    String vram_s = BTSerial.readStringUntil(',');
    String gput_s = BTSerial.readStringUntil('/');

    if(cpu_s.toInt() <= MAX_CPU)       cpu[0] = cpu_s.toInt();
    if(ram_s.toInt() <= MAX_RAM)       ram[0] = ram_s.toInt();
    if(cput_s.toInt() <= MAX_CPU_TEMP) cput[0] = cput_s.toInt();
    if(gpu_s.toInt() <= MAX_GPU)       gpu[0] = gpu_s.toInt();
    if(vram_s.toInt() <= MAX_VRAM)     vram[0] = vram_s.toInt();
    if(gput_s.toInt() <= MAX_CPU)      gput[0] = gput_s.toInt();

    if(sleep_state >= 1 || screen_start == 0){
      screen_start = 1;      
      sleep_state = 0;
      touch_times = 0;
      background = 0;
      lcd();
      Serial.println("---------------------------------------IF BT DATA RECEIVED AFTER SLEEP, TURN ON SCREEN");
      serialprint();
    }
    
    if(screen_off == 0){
      ledcAttachPin(TFT_BL, 0);
    }
    
    if (background == 0) { 
      Serial.println("-- BT DATA RECEIVED - wait_time MAX 60ms: " + String(wait_time));
      updateHomeScreen();
    }
    
    sleep_state = 0;
    wait_time = 0;
    
}

//--------------------------------------------------------------------------------------- VOID NO BT

void no_bt(){
  if(wait_time == 30 || wait_time == 60){
     ledcAttachPin(TFT_BL, 1);
     sleep_state = 1;
     if(wait_time == 60){
        sleep_state = 2;
     }
     touch_times = 0;
   }
    
   if(sleep_state == 2 && wait_time == 61 && dontsleep == 0){
     Serial.println("--------------------------------------- IF NO BT SLEEP 1min");
     serialprint();
     esp_sleep_enable_timer_wakeup(60e6); // 1 minute in microseconds
     esp_deep_sleep_start();
   }
    
   if(wait_time <= 60){
     Serial.println("NO BT DATA RECEIVED - wait_time MAX 60ms: " + String(wait_time));
     wait_time++;
   }
}

//--------------------------------------------------------------------------------------- VOID TOUCH

void touch(){

  int t_y, t_x;
  uint16_t x, y;
  bool pressed = false;
  
  pressed = false;

#ifdef ENABLE_CAP_TOUCH
  if (ts.touched()){
    // Retrieve a point
    TS_Point p = ts.getPoint();

    // Flip things around so it matches our screen rotation
    p.x = map(p.x, 0, 320, 320, 0);
    t_y = p.x;
    t_x = p.y;
    pressed = true;
  }
#endif

#ifdef ENABLE_RES_TOUCH
  if (tft.getTouch(&x, &y)){
    t_x = x;
    t_y = y;
    pressed = true;
  }
#endif
  if (pressed && sleep_state == 0) {
    
    touch_times++;
    if(touch_times > 2) touch_times = 2;
    
    if(touch_times == 2){

      if(background == 1){
        if (t_y < screenheight / 2) {
          Serial.print("TOP - ");
            
          if (t_x < screenwidth / 3) {
              Serial.println("LEFT (menu1)"); //------------------------------------------- TOP LEFT
    
                touch_times = 0;
                background = 0;
                wallpaper();
                delay(50);
    
            } else if (t_x < (screenwidth / 3) * 2) {
              Serial.println("MIDDLE (menu1)"); //------------------------------------------- TOP MIDDLE
    
                background = 2;
                wallpaper();
                delay(10);
               
            } else {
              Serial.println("RIGHT (menu1)"); //------------------------------------------- TOP RIGHT
              
                screen_off = 1;
                touch_times = 0;
                ledcAttachPin(TFT_BL, 1);
                delay(50);
            }
    
          }else{
           Serial.print("BOTTOM - ");
            
            if (t_x < screenwidth / 3) {
              Serial.println("LEFT (menu1)"); //------------------------------------------- BOTTOM LEFT
                if(ledBrightness <= 245){
                  ledBrightness += 10;
                } else {
                  ledBrightness = 255;
                }
                Serial.println("inc ledBrightness state: " + String(ledBrightness));
                EEPROM.put(address, ledBrightness);
                EEPROM.commit();
                ledcWrite(0, ledBrightness); // Start @ initial Brightness
                delay(10);
              
            } else if (t_x < (screenwidth / 3) * 2) {
              Serial.println("MIDDLE (menu1)"); //------------------------------------------- BOTTOM MIDDLE
    
                ledBrightness = 10;
                Serial.println("default ledBrightness state: " + String(ledBrightness));
                EEPROM.put(address, ledBrightness);
                EEPROM.commit();
                ledcWrite(0, ledBrightness); // Start @ initial Brightness
                delay(10);
    
            } else {
              Serial.println("RIGHT (menu1)"); //------------------------------------------- BOTTOM RIGHT
                
                 if(ledBrightness >= 20){
                  ledBrightness -= 10;
                } else {
                  ledBrightness = 10;
                }
                Serial.println("dec ledBrightness state: " + String(ledBrightness));
                EEPROM.put(address, ledBrightness);
                EEPROM.commit();
                ledcWrite(0, ledBrightness); // Start @ initial Brightness
                delay(10);
            }
    
          }

      
      } else if(background == 2){
        
         if (t_y < screenheight / 2) {
            Serial.print("TOP - ");
        
            if (t_x < screenwidth / 3) {
                Serial.println("LEFT (menu2)"); //------------------------------------------- TOP LEFT
                background = 1;
                wallpaper();
                delay(50);
                
              } else if (t_x < (screenwidth / 3) * 2) {
                Serial.println("MIDDLE (menu2)"); //------------------------------------------- TOP MIDDLE
                delay(10);
                
              } else {
                Serial.println("RIGHT (menu2)"); //------------------------------------------- TOP RIGHT
                delay(10);
              }
      
            }else{
             Serial.print("BOTTOM - ");
              
              if (t_x < screenwidth / 3) {
                Serial.println("LEFT (menu2)"); //------------------------------------------- BOTTOM LEFT
                volume = 1;
                delay(10);
                
              } else if (t_x < (screenwidth / 3) * 2) {
                Serial.println("MIDDLE (menu2)"); //------------------------------------------- BOTTOM MIDDLE          
                volume = 3;
                delay(10);
                
              } else {
                Serial.println("RIGHT (menu2)"); //------------------------------------------- BOTTOM RIGHT       
                volume = 2;
                delay(10);
                
              }
      
            }

      }
  
      pressed = false;

    } else if(touch_times == 1){
      
      if(screen_off == 0){
        ledcAttachPin(TFT_BL, 0);
        background = 1;
        wallpaper();
      } else {
        ledcAttachPin(TFT_BL, 0);
        touch_times = 0;
        screen_off = 0;
        background = 0;
        wallpaper();
      }
      
    }
  }
}


//--------------------------------------------------------------------------------------- VOID LCD

void lcd(){
  
  ledcSetup(0, 5000, 8);
#ifdef TFT_BL
  ledcAttachPin(TFT_BL, 0);
#else
  ledcAttachPin(backlightPin, 0);
#endif
  ledcWrite(0, ledBrightness);

  tft.init();
  tft.setRotation(1);
  tft.setFreeFont(&FreeSansBold12pt7b);

#ifdef ENABLE_RES_TOUCH
  touch_calibrate();
#endif

  wallpaper();
  ledcAttachPin(TFT_BL, 1);
}

void wallpaper(){
  if(background == 0){
    drawBmp("/bg.bmp", 0, 0);
    Serial.println("Background drawn - HOME");
  } else if(background == 1){
    drawBmp("/bg2.bmp", 0, 0);
    Serial.println("Background drawn - MENU");
  } else if(background == 2){
    drawBmp("/bg3.bmp", 0, 0);
    Serial.println("Background drawn - TOOLS");
  }
}

//--------------------------------------------------------------------------------------- VOID UPDATE VALUE

void updateHomeScreen(){
  
  String CPU_data = String(cpu[0]) + " %";
  String RAM_data = String(ram[0]) + " %";
  String CPUT_data = String(cput[0]) + " .C";

  String GPU_data = String(gpu[0]) + " %";
  String VRAM_data = String(vram[0]) + " %";
  String GPUT_data = String(gput[0]) + " .C";

  tft.fillRect(40, 130, 440, 30, TFT_BLACK);
  tft.fillRect(350, 130, 440, 30, TFT_BLACK);
  tft.fillRect(25, 280, 440, 30, TFT_BLACK);
  
//------------------------------------------- CPU USADE
    if (cpu[0] > WARN_CPU) {
      tft.setTextColor(TFT_RED);
      tft.drawCentreString(CPU_data, 75, 135, 1);
    } else {
      if(cpu[0] > 0){
        tft.setTextColor(TFT_GREEN);
        tft.drawCentreString(CPU_data, 75, 135, 1);
      } else {
        tft.setTextColor(TFT_WHITE);
        tft.drawCentreString(CPU_data, 75, 135, 1);
      }
    }

//------------------------------------------- RAM
    if (ram[0] > WARN_RAM) {
      tft.setTextColor(TFT_RED);
      tft.drawCentreString(RAM_data, 244, 135, 1);
    } else {
      if(ram[0] > 0){
        tft.setTextColor(TFT_GREEN);
        tft.drawCentreString(RAM_data, 244, 135, 1);
      } else {
        tft.setTextColor(TFT_WHITE);
        tft.drawCentreString(RAM_data, 244, 135, 1);
      }
    }

//------------------------------------------- CPU TEMP
    if (cput[0] > WARN_CPU_TEMP){
      tft.setTextColor(TFT_RED);
      tft.drawCentreString(CPUT_data, 403, 135, 1);;
    } else {
      if(cput[0] > 0){
        tft.setTextColor(TFT_GREEN);
        tft.drawCentreString(CPUT_data, 403, 135, 1);
      } else {
        tft.setTextColor(TFT_WHITE);
        tft.drawCentreString(CPUT_data, 403, 135, 1);
      }
    }

//------------------------------------------- GPU USADE
    if (gpu[0] > WARN_GPU) { 
      tft.setTextColor(TFT_RED);
      tft.drawCentreString(GPU_data, 75, 280, 1);
    } else {
      if(gpu[0] > 0){
        tft.setTextColor(TFT_GREEN);
        tft.drawCentreString(GPU_data, 75, 280, 1);
      } else {
        tft.setTextColor(TFT_WHITE);
        tft.drawCentreString(GPU_data, 75, 280, 1);
      }
    }

//------------------------------------------- GPU VRAM
    if (vram[0] > WARN_VRAM) { 
      tft.setTextColor(TFT_RED);
      tft.drawCentreString(VRAM_data, 244, 280, 1);
    } else {
      if(vram[0] > 0){
        tft.setTextColor(TFT_GREEN);
        tft.drawCentreString(VRAM_data, 244, 280, 1);
      } else {
        tft.setTextColor(TFT_WHITE);
        tft.drawCentreString(VRAM_data, 244, 280, 1);
      }
    }

//------------------------------------------- GPU TEMP
    if (gput[0] > WARN_GPU_TEMP) { 
      tft.setTextColor(TFT_RED);
      tft.drawCentreString(GPUT_data, 403, 280, 1);
    } else {
      if(gput[0] > 0){
        tft.setTextColor(TFT_GREEN);
        tft.drawCentreString(GPUT_data, 403, 280, 1);
      } else {
        tft.setTextColor(TFT_WHITE);
        tft.drawCentreString(GPUT_data, 403, 280, 1);
      }
    }

}

//--------------------------------------------------------------------------------------- VOID IMAGE

void drawBmp(const char *filename, int16_t x, int16_t y){

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (!bmpFS){
    Serial.print("File not found:");
    Serial.println(filename);
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row;
  uint8_t r, g, b;

  if (read16(bmpFS) == 0x4D42){
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)){
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++){

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++){
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
    } else {
      Serial.println("[WARNING]: BMP format not recognized.");
    }
  }
  bmpFS.close();
}

//----------------------------------------------------- UNSIGNED INT IMAGE

uint16_t read16(fs::File &f){
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

//----------------------------------------------------- UNSIGNED LONG INT IMAGE

uint32_t read32(fs::File &f){
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}


//--------------------------------------------------------------------------------------- VOID TOUCH CALIBRATION

#ifdef ENABLE_RES_TOUCH
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // Check if calibration file exists and size is correct
  if (FILESYSTEM.exists(CALIBRATION_FILE))
  {
    fs::File f = FILESYSTEM.open(CALIBRATION_FILE, "r");
    if (f)
    {
      if (f.readBytes((char *)calData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }

  if (calDataOK)
  {
    // Calibration data valid
    tft.setTouch(calData);
    Serial.println("Touch calibration loaded.");
  }
  else
  {
    // Data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");
    Serial.println("Touch calibration loaded.");

    // Store data
    fs::File f = FILESYSTEM.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
#endif

void serialprint(){
  Serial.println("wait_time: " + String(wait_time));
  Serial.println("touch_times: " + String(touch_times));
  Serial.println("sleep state: " + String(sleep_state));
  Serial.println("ledBrightness state: " + String(ledBrightness));
  Serial.println("---------------------------------------");
}
