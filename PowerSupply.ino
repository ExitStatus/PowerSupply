#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LowPower.h>
#include <EEPROM.h>

#include "RingBuffer.h"

#define PIN_BATTERY A0
#define PIN_CHARGING A1
#define PIN_CHARGED A2

#define FULL_VOLTAGE  4.0
#define EMPTY_VOLTAGE 2.8

#define OVERSAMPLING  3
#define BUFFERSIZE  32

LiquidCrystal_I2C lcd(0x27, 16, 2);
RingBuffer buf = RingBuffer(BUFFERSIZE);

typedef struct _persistentData
{
  uint32_t Bom;
  float MaxVoltage;
  float MinVoltage;
} PersistentData;

PersistentData data;
bool programmingMode = false;

byte Block[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

void setup() 
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, Block);

  delay(100);

  if (GetCurrentVoltage() < 2.2)
  {
    programmingMode = true;
    DisplayProgrammingMode();
  }
  else
  {
    InitData();
    DisplayMaxMinData();
  
    delay(5000);

    for (int i=0; i < BUFFERSIZE; i++)
      buf.Add(ReadSensor());
  }
}

void loop() 
{
  if (!programmingMode)
  {
    float voltage = GetAverageVoltage();
    int percent = GetChargePercent(voltage);
  
    DisplayCharge(percent, voltage);
  }  

  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
}

void InitData()
{
  EEPROM.get(0, data);
  if (data.Bom != 0xbeef)
  {
    data.Bom = 0xbeef;
    data.MaxVoltage = FULL_VOLTAGE;
    data.MinVoltage = EMPTY_VOLTAGE;
    
    EEPROM.put(0, data);
  }  
}

int ReadSensor()
{
  int sensorValue = 0;
  for (int i=0; i<OVERSAMPLING; i++)
    sensorValue += analogRead(PIN_BATTERY);

  return (int)(sensorValue / OVERSAMPLING);
}

float GetAverageVoltage()
{
  buf.Add(ReadSensor());
  float voltage = buf.Average() * (5.0 / 1023.0);

  if (voltage > 3.3) // ie not plugged into USB for programming
  {
    if (voltage < data.MinVoltage)
    {
      data.MinVoltage = voltage;
      EEPROM.put(0, data);    
    }
  
    if (voltage > data.MaxVoltage)
    {
      data.MaxVoltage = voltage;
      EEPROM.put(0, data);    
    }
  }

  return voltage;
}

float GetCurrentVoltage()
{
  return ReadSensor() * (5.0 / 1023.0);
}

int GetChargePercent(float voltage)
{
  int percent = (int)(((voltage - data.MinVoltage) * 100) / (data.MaxVoltage - data.MinVoltage));
  
  if (percent > 100)
    percent = 100;

  if (percent < 0)
    percent = 0;  

   return percent;
}

void DisplayCharge(int percent, float voltage)
{
  lcd.setCursor(0,0);
  lcd.print("Batt: ");
  lcd.print(percent);
  lcd.print("% ");
  lcd.print(voltage,2);
  lcd.print("v");

  // Stop display leaving 2 v's
  if (percent < 100)
    lcd.print(" ");

  int graphx = 16 * ((float)percent / 100);
  lcd.setCursor(0,1);
  
  for (int i=0; i < 16; i++)
  {
    if (i < graphx)
      lcd.write(0);
   else
      lcd.print(" ");
  }  
}

void DisplayMaxMinData()
{
  lcd.setCursor(0,0);
  lcd.print("Max: ");
  lcd.print(data.MaxVoltage, 2);
  lcd.print("v");

  lcd.setCursor(0,1);
  lcd.print("Min: ");
  lcd.print(data.MinVoltage, 2);
  lcd.print("v");  
}

void DisplayProgrammingMode()
{
  lcd.setCursor(2,0);
  lcd.print("Programming");
  lcd.setCursor(6,1);
  lcd.print("Mode");
}
