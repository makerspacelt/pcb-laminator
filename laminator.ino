const int RELAYS_COUNT = 4;
const int SUPER_MAX_TEMPERATURE = 10000;
const int SUPER_MIN_TEMPERATURE = -10000;
const int WORK_MAX_TEMPERATURE = 72;
const int WORK_MIN_TEMPERATURE = 70;
const int NTC_MAP_TEMPERATURE_DELTA = 5;
const int STATUS_NOT_MAPPED = 0;
const int STATUS_MAP_RELAYS = 5;
const int STATUS_WORK = 10;
const int STATUS_DEBUG = 9999;

int relays[RELAYS_COUNT] = {2, 3, 4, 5};
int ntc[RELAYS_COUNT] = {A0, A1, A2, A3};
int ntc_map[RELAYS_COUNT]; // {A3, A2, A0, A1}
int ntc_temperature_cache[RELAYS_COUNT];
int laminator_status;
int print_delay = 0;
int print_interval = 2048;

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
  int temperature = -0.16 * bits + 205;
  return temperature;
//  return bits;
}

void cacheNtcTemperature () {
  for (int i = 0; i < RELAYS_COUNT; i++) {
    ntc_temperature_cache[i] = getNtcTemperature(ntc[i]);
  }
}

int getRelayTemperature (int relay) {
  return getNtcTemperature(ntc_map[relay]);
}

void setup() {                
  for (int i = 0; i < RELAYS_COUNT; i++) {
//    pinMode(ntc[i], INPUT_PULLUP);
    pinMode(relays[i], OUTPUT);
    switchOffRelay(i);
  }
  laminator_status = STATUS_NOT_MAPPED;
  Serial.begin(9600);
}

boolean notMapped(int ntc) {
    for (int i = 0; i < RELAYS_COUNT; i++) {
      if (ntc_map[i] == ntc) {
        return false;
      }
    }
  return true;  
}

void initMap() {
  int relay_on;
  // TODO: add time out for mapping
  if (laminator_status == STATUS_NOT_MAPPED) {
    cacheNtcTemperature();
    laminator_status = STATUS_MAP_RELAYS;
  }
  if (laminator_status == STATUS_MAP_RELAYS) {
    for (int i = 0; i < RELAYS_COUNT; i++) {
      if (ntc_map[i] == NULL) {
        switchOnRelay(i);
        relay_on = i;
        break;
      }
    }
    for (int i = 0; i < RELAYS_COUNT; i++) {
      if ((getNtcTemperature(ntc[i]) - ntc_temperature_cache[i]) > NTC_MAP_TEMPERATURE_DELTA && notMapped(ntc[i])) {
        Serial.print("Mapped relay: ");
        Serial.print(relay_on);
        Serial.print(" to NTC ");
        Serial.println(i);
        ntc_map[relay_on] = ntc[i];
        switchOffRelay(relay_on);
        cacheNtcTemperature();
        if (relay_on == (RELAYS_COUNT - 1)) {
          laminator_status = STATUS_WORK;
        }
      }
    }
  }
}

void validateTemperature() {
  boolean sheat = false;
  int temperature;
  if (laminator_status == STATUS_DEBUG) {
    return;
  }
  for (int i = 0; i < RELAYS_COUNT; i++) {
    temperature = getNtcTemperature(ntc[i]);
    if (temperature > SUPER_MAX_TEMPERATURE || temperature < SUPER_MIN_TEMPERATURE) {
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

void printTemperatures(){  
  if (print_delay == print_interval) {
    int temp;
    for (int i = 0; i < RELAYS_COUNT; i++) {
      if (laminator_status == STATUS_DEBUG) {
        Serial.print(" NTC ");
        temp = getNtcTemperature(i);
      } else {
        Serial.print(" Relay ");
        temp = getRelayTemperature(i);
      }
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(temp);
      Serial.print("C; ");
    }
    Serial.println(" ");
    print_delay = 0;
  }
  print_delay++;
}

void work(){
  if (laminator_status == STATUS_WORK) {
    for (int i = 0; i < RELAYS_COUNT; i++) {
      if (getRelayTemperature(i) < WORK_MIN_TEMPERATURE) {
        switchOnRelay(i);
      }
      if (getRelayTemperature(i) > WORK_MAX_TEMPERATURE) {
        switchOffRelay(i);
      }
    }  
  }
}

void debug() {
  int incoming = Serial.read();
  if (incoming == 'N') {
    Serial.println("Debug mode ON;");
    switchOffAllRelays();
    laminator_status = STATUS_DEBUG;
    print_interval = 16384;
    print_delay = 0;
  }
  if (incoming == 'F') {
    Serial.println("Debug mode OFF;");
    switchOffAllRelays();
    for (int i = 0; i < RELAYS_COUNT; i++) {
        ntc_map[i] = NULL;
    }
    laminator_status = STATUS_NOT_MAPPED;
    print_interval = 2048;
    print_delay = 0;
  }
  if (laminator_status == STATUS_DEBUG) {
    if (incoming == '1') {
      digitalWrite(relays[0], !digitalRead(relays[0]));
    }
    if (incoming == '2') {
      digitalWrite(relays[1], !digitalRead(relays[1]));
    }
    if (incoming == '3') {
      digitalWrite(relays[2], !digitalRead(relays[2]));
    }
    if (incoming == '4') {
      digitalWrite(relays[3], !digitalRead(relays[3]));
    }
  }
}

void loop() {
  initMap();
  validateTemperature();
  work();
  printTemperatures();
  debug();
  
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
