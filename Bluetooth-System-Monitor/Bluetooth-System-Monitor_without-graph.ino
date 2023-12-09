#include "BluetoothSerial.h"
#include <TFT_eSPI.h>
#include <SPIFFS.h>
#include <FS.h>
#include <esp_sleep.h>
#include <EEPROM.h>

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

int t = 1;
int g = 0;
int c = 0;
int onoff = 0;

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int slp = 0;
RTC_DATA_ATTR int ledBrightness = 30;
int address = 0;

int ymax_cpu = 100; //Percentagem Utilizacao CPU
int ymax_fan = 100; //Percentagem Memoria RAM
int ymax_ram = 120;  //Temperatura CPU
int ymax_hdd = 100;  //Percentagem Utilizacao GPU
int ymax_gpu = 100;  //Percentagem Memoria GPU
int ymax_procs = 120;  //Temperatura GPU


int warn_cpu = 75; //Percentagem Utilizacao CPU
int warn_rpm = 75; //Percentagem Memoria RAM
int warn_ram = 75; //Temperatura CPU
int warn_hdd = 75; //Percentagem Utilizacao GPU
int warn_gpu = 75; //Percentagem Memoria GPU
int warn_procs = 75;   //Temperatura GPU


int16_t cpu[1];
int16_t fan[1];
int16_t ram[1];
int16_t hdd[1];
int16_t gpu[1];
int16_t procs[1];


int margin = 40;
int screenwidth = 480;
int screenheight = 320;

//--------------------------------------------------------------------------------------- VOID SETUP

void setup() {
  // Begin BT Serial
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
  Serial.println("!!!!! Manoper - SystemMonitorPC edition !!!!!");
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
Serial.println("Boot state: " + String(bootCount));
Serial.println("sleep state: " + String(slp));
Serial.println("ledBrightness state: " + String(ledBrightness));
Serial.println("---------------------------------------");

if(bootCount < 1){
  lcd();
}
bootCount=1;
}

//--------------------------------------------------------------------------------------- VOID LOOP

void loop() {

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
  if (tft.getTouch(&x, &y)) {

    t_x = x;
    t_y = y;
    pressed = true;

  }
#endif
  
  if (BTSerial.available()) {
    // Esperamos dados no formato: "33,428,8343,16000,68,371"
    String cpu_s = BTSerial.readStringUntil(',');
    String fan_s = BTSerial.readStringUntil(',');
    String ram_s = BTSerial.readStringUntil(',');
    String hdd_s = BTSerial.readStringUntil(',');
    String gpu_s = BTSerial.readStringUntil(',');
    String procs_s = BTSerial.readStringUntil('/');

    cpu[0] = cpu_s.toInt();
    fan[0] = fan_s.toInt();
    ram[0] = ram_s.toInt();
    hdd[0] = hdd_s.toInt();
    gpu[0] = gpu_s.toInt();
    procs[0] = procs_s.toInt();

    if(slp == 1){
      lcd();
      slp=0;
      Serial.println("---------------------------------------IF BT OK");
      Serial.println("Boot state: " + String(bootCount));
      Serial.println("sleep state: " + String(slp));
      Serial.println("ledBrightness state: " + String(ledBrightness));
      Serial.println("---------------------------------------");
    }
    
    if (ram_s.length() > 0) {
        if (ram[0] != 0 && c==0) {
            ledcAttachPin(TFT_BL, 0);
            Serial.println("BT dados recebidos (mostra cada vez esta mensagem)");
            updateHomeScreen();
        }
        g=0;
    }
    
  } else {
    if(g==8000){
      ledcAttachPin(TFT_BL, 1);
      Serial.print("BT sem dados recebidos (so mostra uma vez) - ms = ");
      Serial.println(g);
      c=0;
      slp=1;
    }
    if(slp == 1 && g==8001){
      Serial.println("--------------------------------------- IF BT NOT OK");
      Serial.println("Boot state: " + String(bootCount));
      Serial.println("sleep state: " + String(slp));
      Serial.println("ledBrightness state: " + String(ledBrightness));
      Serial.println("---------------------------------------");
      esp_sleep_enable_timer_wakeup(60e6); // 1 minute in microseconds
      esp_deep_sleep_start();
    }
    if(g <= 8000){
      g++;
    }
  }

  // Process Touches
  if (pressed) {
      
      if (t_y < screenheight / 2) {
        Serial.print("SUPERIOR - ");
        if (t_x < screenwidth / 3) {
          Serial.println("Esquerda");
          t++;
          if(t>1){
            c=0;
            t=1;
            ledcAttachPin(TFT_BL, 0);
          }
         

        } else if (t_x < (screenwidth / 3) * 2) {
          
          Serial.println("Meio");
          Serial.println("--------------------------------------- 1 day off");
          Serial.println("Boot state: " + String(bootCount));
          Serial.println("sleep state: " + String(slp));
          Serial.println("ledBrightness state: " + String(ledBrightness));
          Serial.println("---------------------------------------");
          // Configurar para acordar após um dia (em microssegundos)
          esp_sleep_enable_timer_wakeup(24 * 60 * 60 * 1e6);
          esp_deep_sleep_start();
          
        } else {
          Serial.println("Direita");
          t--;
          if(t<1){
            c=1;
            t=1;
            ledcAttachPin(TFT_BL, 1);
          }
        }

      }else{
        Serial.print("INFERIOR - ");
        if (t_x < screenwidth / 3) {
          Serial.println("Esquerda");

          if(ledBrightness < 235){
            ledBrightness += 20;
          } else {
            ledBrightness = 254;
          }
          Serial.println("inc ledBrightness state: " + String(ledBrightness));
          lcd();
          EEPROM.put(address, ledBrightness);
          EEPROM.commit();
          
        } else if (t_x < (screenwidth / 3) * 2) {
          Serial.println("Meio");

          ledBrightness = 30;
          Serial.println("default ledBrightness state: " + String(ledBrightness));
          lcd();
          EEPROM.put(address, ledBrightness);
          EEPROM.commit();

        } else {
          Serial.println("Direita");
          
           if(ledBrightness > 20){
            ledBrightness -= 20;
          } else {
            ledBrightness = 0;
          }
          Serial.println("dec ledBrightness state: " + String(ledBrightness));
          lcd();
          EEPROM.put(address, ledBrightness);
          EEPROM.commit();
          
        }

      }
      delay(100);
      pressed = false;
    }

}

//--------------------------------------------------------------------------------------- VOID

void updateHomeScreen() {
  
  String temp_data = String(cpu[0]) + " %";
  String rpm_data = String(fan[0]) + " %";
  String ram_data = String(ram[0]) + " .C";

  String hdd_data = String(hdd[0]) + " %";
  String gpu_data = String(gpu[0]) + " %";
  String procs_data = String(procs[0]) + " .C";

  tft.fillRect(40, 130, 440, 30, TFT_BLACK);
  tft.fillRect(350, 130, 440, 30, TFT_BLACK);
  tft.fillRect(25, 280, 440, 30, TFT_BLACK);

  if (cpu[0] > warn_cpu) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(temp_data, 75, 135, 1);
  } else {
    if(cpu[0] > 0){
      tft.setTextColor(TFT_GREEN);
      tft.drawCentreString(temp_data, 75, 135, 1);
    } else {
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString(temp_data, 75, 135, 1);
    }
  }



  if (fan[0] > warn_rpm) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(rpm_data, 244, 135, 1);
  } else {
    if(fan[0] > 0){
      tft.setTextColor(TFT_GREEN);
      tft.drawCentreString(rpm_data, 244, 135, 1);
    } else {
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString(rpm_data, 244, 135, 1);
    }
  }



  if (ram[0] > warn_ram) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(ram_data, 403, 135, 1);;
  } else {
    if(ram[0] > 0){
      tft.setTextColor(TFT_GREEN);
      tft.drawCentreString(ram_data, 403, 135, 1);
    } else {
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString(ram_data, 403, 135, 1);
    }
  }



  if (hdd[0] > warn_hdd) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(hdd_data, 75, 280, 1);
  } else {
    if(hdd[0] > 0){
      tft.setTextColor(TFT_GREEN);
      tft.drawCentreString(hdd_data, 75, 280, 1);
    } else {
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString(hdd_data, 75, 280, 1);
    }
  }



  if (gpu[0] > warn_gpu) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(gpu_data, 244, 280, 1);
  } else {
    if(gpu[0] > 0){
      tft.setTextColor(TFT_GREEN);
      tft.drawCentreString(gpu_data, 244, 280, 1);
    } else {
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString(gpu_data, 244, 280, 1);
    }
  }
  


  if (procs[0] > warn_procs) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(procs_data, 403, 280, 1);
  } else {
    if(procs[0] > 0){
      tft.setTextColor(TFT_GREEN);
      tft.drawCentreString(procs_data, 403, 280, 1);
    } else {
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString(procs_data, 403, 280, 1);
    }
  }

}

//--------------------------------------------------------------------------------------- VOID

void drawBmp(const char *filename, int16_t x, int16_t y)
{

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  fs::File bmpFS;

  bmpFS = FILESYSTEM.open(filename, "r");

  if (!bmpFS)
  {

    Serial.print("File not found:");
    Serial.println(filename);
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row;
  uint8_t r, g, b;

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
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
    }
    else
      Serial.println("[WARNING]: BMP format not recognized.");
  }
  bmpFS.close();
}

uint16_t read16(fs::File &f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}


//--------------------------------------------------------------------------------------- VOID

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


void lcd(){
  // Setup PWM channel and attach pin bl_pin
  ledcSetup(0, 5000, 8);
#ifdef TFT_BL
  ledcAttachPin(TFT_BL, 0);
#else
  ledcAttachPin(backlightPin, 0);
#endif // defined(TFT_BL)
  ledcWrite(0, ledBrightness); // Start @ initial Brightness

  // Initialise the TFT stuff
  tft.init();
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  Serial.println("Boot STATE = 1 OU INICIADO SE SLP = 0");

  #ifdef ENABLE_RES_TOUCH
  touch_calibrate();
#endif

  // Draw Background
  drawBmp("/bg.bmp", 0, 0);
  Serial.println("Background drawn.");

  Serial.println("Setup done.");
  ledcAttachPin(TFT_BL, 1);
}
