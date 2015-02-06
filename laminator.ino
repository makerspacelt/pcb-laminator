const int RELAYS_COUNT = 4;
const int SUPER_MAX_TEMPERATURE = 22;

int relays[RELAYS_COUNT] = {2, 3, 4, 5};
int ntc[RELAYS_COUNT] = {A0, A1, A2, A3};
int ntc_map[RELAYS_COUNT]; //= {a1, a0, a3, a2};
int ntc_temperature[RELAYS_COUNT];

void switchOffRelay(int relay) {
  digitalWrite(relays[relay], HIGH);
}

void switchOnRelay(int relay) {
  digitalWrite(relays[relay], LOW);
}

void switchOffAllRelays() {
  for (int i = 0; i < RELAYS_COUNT; i++) {
    switchOffRelay(i);
  }
}

int getNtcTemperature (int ntc) {
  int bits = analogRead(ntc);
  int temperature = (1000 - bits) / 10;
  return temperature;
}

int getRelayTemperature (int relay) {
  return getNtcTemperature(ntc_map[relay]);
}

void setup() {                
  for( int i = 0; i < RELAYS_COUNT; i++ )
   {
      pinMode(ntc[i], INPUT_PULLUP);
      pinMode(relays[i], OUTPUT);
      switchOffRelay(i);
   }
  Serial.begin(9600);
}

void initMap() {
  
}

void validateTemperature() {
  boolean sheat = false;
  int temperature;
  for (int i = 0; i < RELAYS_COUNT; i++) {
    temperature = getNtcTemperature(ntc[i]);
    if (temperature > SUPER_MAX_TEMPERATURE) {
      switchOffAllRelays();
      pinMode(13, OUTPUT);
      while(true){
        delay(300);
        Serial.print("C: ");        
        Serial.print(temperature);
        Serial.print("; NTC: ");
        Serial.print(i);
        Serial.print("; C now: ");        
        Serial.print(getNtcTemperature(ntc[i]));
        Serial.println("; Laminator on fire!!!");
        digitalWrite(13, !digitalRead(13));
      }
    }
  }
}

void loop() {
  initMap();
  validateTemperature();
  
  
/*  delay(1000);             
  int sensorValue0 = analogRead(A0);
  int sensorValue1 = analogRead(A1);
  int sensorValue2 = analogRead(A2);
  int sensorValue3 = analogRead(A3);
  // print out the value you read:
  Serial.print("0A - ");
  Serial.print(sensorValue0);
  Serial.print("; 1A - ");
  Serial.print(sensorValue1);
  Serial.print("; 2A - ");
  Serial.print(sensorValue2);
  Serial.print("; 3A - ");
  Serial.println(sensorValue3);*/
}
