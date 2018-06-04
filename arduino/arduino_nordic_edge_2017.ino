/**
 * This sketch was used for a simple test case at the Nordic Edge Expo 2017 in Stavanger, Norway. 
 * I had a couple of sensors attached to an Arduino board, connected to a Raspberry PI via USB, 
 * and I uset NodeRED on the RPi to parse and display the data.
 * 
 */

#include <DHT.h>

#define PIN_SENSOR_DHT 7

const float VRefer = 3.3;       // voltage of adc reference
const int pinAdc = A1;
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 2000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
String Dust = "";
String MQ4 = "";
String O2 = "";
//const int gasPin = A0; //GAS sensor output pin to Arduino analog A0 pin

DHT SensorDHT(PIN_SENSOR_DHT,DHT11);

void setup() {
  Serial.begin(9600);
 
  SensorDHT.begin();
  pinMode(8,INPUT);
  starttime = millis();//get the current time;
}

void loop() {

  //Serial.println(readConcentration());
  O2 = (String)readConcentration();
  float sensor_volt; //Define variable for sensor voltage
  float RS_air; //Define variable for sensor resistance
  float R0; //Define variable for R0
  float sensorValue; //Define variable for analog readings
  for (int x = 0 ; x < 500 ; x++) //Start for loop
  {
    sensorValue = sensorValue + analogRead(A0); //Add analog values of sensor 500 times
  }
  sensorValue = sensorValue / 500.0; //Take average of readings
  sensor_volt = sensorValue * (5.0 / 1023.0); //Convert average to voltage
  RS_air = ((5.0 * 10.0) / sensor_volt) - 10.0; //Calculate RS in fresh air
  R0 = RS_air / 4.4; //Calculate R0

  //Serial.print("R0 = "); //Display "R0"
  //Serial.println(R0); //Display value of R0
  MQ4 = (String)R0;
  delay(1000); //Wait 1 second
  
//Read Temperature and Humidity from DHT11 Sensor
  String Temp_DHT = (String)SensorDHT.readTemperature();
  //Serial.print(Temp_DHT);
  //Serial.print("\n");
  String Humid_DHT = (String)SensorDHT.readHumidity();
  String str_Payload;
  int NData = 6;

  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) >= sampletime_ms)//if the sampel time = = 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=&gt;100
    Dust = (String)(1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62); // using spec sheet curve
    if (Dust == "0.62") {
      return;
    }
    else {
      //  Print til serial for debug.
      //Serial.print("concentration = ");
      //Serial.print(Dust);
      //Serial.println(" pcs/0.01cf");
      //Serial.println("\n");
      lowpulseoccupancy = 0;
      starttime = millis();
    }
  }

//Making a string of the format: NData Data1 Data2 Data3 ... DataN
  str_Payload += NData;
  str_Payload += " " + Temp_DHT;
  str_Payload += " " + Humid_DHT;
  str_Payload += " " + Dust;
  str_Payload += " " + MQ4;
  str_Payload += " " + O2;
  str_Payload += " ";
  
  byte Payload[str_Payload.length()];
  int LengthFrameAPI = 18 + sizeof(Payload);
  int LengthPayload = sizeof(Payload);
  
  Serial.print(str_Payload);
    
  delay(2000);

//  Print til serial for debug.
//  Serial.print(analogRead(A1));
//  Serial.print(analogRead(A2));
//  Serial.print(analogRead(A3));
//  Serial.print(analogRead(A4));
//  Serial.print(analogRead(A5));
}

float readO2Vout()
{
    long sum = 0;
    for(int i=0; i<32; i++)
    {
        sum += analogRead(pinAdc);
    }

    sum >>= 5;

    float MeasuredVout = sum * (VRefer / 1023.0);
    return MeasuredVout;
}

float readConcentration()
{
    // Vout samples are with reference to 3.3V
    float MeasuredVout = readO2Vout();

    //float Concentration = FmultiMap(MeasuredVout, VoutArray,O2ConArray, 6);
    //when its output voltage is 2.0V,
    float Concentration = MeasuredVout * 0.21 / 2.0;
    float Concentration_Percentage=Concentration*100;
    return Concentration_Percentage;
}