#include <OneWire.h>
#include <DallasTemperature.h>
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>
 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
 
#define ONE_WIRE_BUS 3
DeviceAddress sensor1;
// Setup a oneWire
OneWire oneWire(ONE_WIRE_BUS);
float temperatura;
DallasTemperature sensors(&oneWire);

//https://wiki.keyestudio.com/KS0429_keyestudio_TDS_Meter_V1.0
#define TdsSensorPin A1
#define VREF 5.0 // voltagem de referência
#define SCOUNT 30 // sum of sample point
int analogBuffer[SCOUNT]; // armazena o valor analogico num vetor
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
//SENSOR DE PH https://www.electroniclinic.com/arduino-libraries-download-and-projects-they-are-used-in-project-codes/
float calibration_value = 23.34 - 0.7; //valor para calibrar os valores do pH
int phval = 0;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;

void setup()
{
  //CARREGA LCD
  lcd.begin(16, 2);
  ///
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);
  sensors.begin();
  if (!sensors.getAddress(sensor1, 0))
    Serial.println("Sensores nao encontrados !");
  printarTelaBoasVindas();
}
void loop()
{
  //TEMPERATURA
  sensors.requestTemperatures();
  temperatura = sensors.getTempCByIndex(0);
  Serial.print("Celsius: ");
  //Printa no terminal a temperatura
  Serial.print(sensors.getTempCByIndex(0));
  Serial.println(sensors.getTempFByIndex(0));
  delay(1000);
  

  // CÓDIGO DA DOCUMENTAÇÃO DO SENSOR DE TDS
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) //a cada 40 milissegundos, leia o valor analógico do ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //leia o valor analógico e armazene no buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVolatge = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //COMPENSAÇÃO ATRIBUIDA NO VALOR DE TDS
    Serial.print("Valor de TDS:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");
  }
  //SENSOR DE PH
  for (int i = 0; i < 10; i++)
  {
    buffer_arr[i] = analogRead(A0);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buffer_arr[i] > buffer_arr[j])
      {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  avgval = 0;
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];
  float volt = (float)avgval * 5.0 / 1024 / 6; //CALIBRA VOLTAGEM DO SENSOR DE PH
  ph_act = -5.70 * volt + calibration_value;//PH ATUAL

  Serial.print("pH Val: ");
  Serial.println(ph_act);
  delay(1000);
  printarTela();
}
int getMedianNum(int bArray[], int iFilterLen)
{
  //TDS MÉDIO
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

void printarTelaBoasVindas()
{
  lcd.setCursor(0,0);
  lcd.print("Bem vindo a");
  lcd.setCursor(1,0);
  lcd.print("AMPara");
  delay(5000);
}
void printarTela()
{
  //TEMPERATURA
  lcd.setCursor(0,0);
  lcd.print("TEMPERATURA ");
  delay(3000);
  lcd.clear();
  delay(500);
  lcd.print("Valor normal:");
  lcd.setCursor(0,1);
  lcd.print("20 *C - 28 *C");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Valor atual");
  lcd.setCursor(0,1);
  lcd.print(temperatura);
  lcd.print(" *C");
  delay(3000);
  lcd.clear();
  //TDS
  lcd.setCursor(0,0);
  lcd.print("TDS ");
  delay(3000);
  lcd.clear();
  delay(500);
  lcd.print("Valor normal:");
  lcd.setCursor(0,1);
  lcd.print("0 PPM - 200 PPM");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Valor atual");
  lcd.setCursor(0,1);
  lcd.print(tdsValue);
  lcd.print(" ppm");
  delay(3000);
  lcd.clear();
  //PH
  lcd.setCursor(0,0);
  lcd.print("PH ");
  delay(3000);
  lcd.clear();
  delay(500);
  lcd.print("Valor normal:");
  lcd.setCursor(0,1);
  lcd.print("6.5 PH - 7.5 PH");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Valor atual");
  lcd.setCursor(0,1);
  lcd.print(ph_act);
  lcd.print(" pH");
  delay(3000);
  lcd.clear();
}
