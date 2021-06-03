//Arduino controller for bubble fluid evacuation of Ballet of the Final Anthozoans, 2021
//by Adam Hutz, 2021

/*
 * The goal of this code is to regularly drain a receptical designed to catch bubble liquid
 * falling off a bubble wand. The challenge here is that the liquid must be drained over a wall,
 * meaning we can't rely on gravity alone. Monitoring the level of liquid directly would also
 * be difficult, so this system simply runs an extraction pump for a certain number of seconds
 * periodically--either several times per day or more likely several times per week. We'll start
 * with once every two hours and dial it back if that seems like way too much.
 * 
 * The "reset" button on the Arduino causes an "initialization evacuation" of fluids, meaning
 * that it can be used in an emergency or convenience situation to empty the vessel.
 */

const byte pumpPin = 12;
//const int potPin = 0; 
//int potValue = 0;

unsigned long previousMillis = 0;

// The minimum amount of pump active time that will evacuate the collection chamber fully.
unsigned long evacTime = 60000; // Set at 60 seconds. It seems to process about 3oz bubble fluid every 60s.
unsigned long pumpInterval = 7200000; //pump evacuates chamber once every two hours (in milliseconds). This number can be tuned to make evacs more frequent.

bool pumpStatus = 0;
bool intervalStatus = 0;

void setup() {
  //startup delay
  delay(2000);
  Serial.begin(9600);
  Serial.println("Serial monitoring started.");

  pinMode(pumpPin, OUTPUT);
  //  pinMode(potPin, INPUT);

  Serial.print("Initial evac of ");
  Serial.print(evacTime / 1000);
  Serial.println(" seconds.");
  // Initialize. This makes the "reset" button on the Arduino (or unplugging, plugging in) an emergency pump button.
  digitalWrite(pumpPin, HIGH);
  delay(evacTime);
  digitalWrite(pumpPin, LOW);
  intervalStatus = 1;
  Serial.println("System initialized.");
}

void loop() {
  unsigned long currentMillis = millis();
  //  potValue = analogRead(potPin);
  //  Serial.print("Pot value = ");
  //  Serial.println(potValue);
  //  pumpInterval = map(potValue, 0, 1024, minInterval, maxInterval);
  //  Serial.print("Pump interval (milliseconds) = ");
  //  Serial.println(pumpInterval);

//  Serial.print("Current millis|previous millis|Next evac in _ minutes = ");
//  Serial.print(currentMillis);
//  Serial.print("|");
//  Serial.print(previousMillis);
//  Serial.print("|");
//  Serial.println(round((pumpInterval-(currentMillis - previousMillis))/60000));
  //Serial.println(pumpInterval);

  //  Serial.print("|");
  //  Serial.println(pumpInterval);

  if (intervalStatus == 1 && currentMillis - previousMillis >= pumpInterval) {
//    Serial.println("Pump interval exceeded; starting to pump.");
    digitalWrite(pumpPin, HIGH);
    previousMillis = currentMillis;
    intervalStatus = 0;
    pumpStatus = 1;
  };

  if (pumpStatus == 1 && currentMillis - previousMillis >= evacTime) {
//    Serial.println("Pump evacuation time exceeded; stopping pump, starting interval.");
    digitalWrite(pumpPin, LOW);
    previousMillis = currentMillis;
    pumpStatus = 0;
    intervalStatus = 1;
  };
}
