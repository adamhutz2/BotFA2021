// LIGHTSTRING PINS:
const byte fuzziesPin1 =  5;
const byte fuzziesPin2 =  6;
const byte fuzziesPin3 =  11;

unsigned long currentMillis;

//**********************************pwm fade code below
//pwm-fade-with-millis.ino
// Example Fading LED with analogWrite and millis()
// See baldengineer.com/fading-led-analogwrite-millis-example.html for more information
// Created by James Lewis

// define directions for LED fade
#define UP 0
#define DOWN 1
#define HOVER 2

// constants for min and max PWM
const int minPWM = 3;
const int maxPWM = 255;

// State Variable for Fade Direction
byte fadeDirection1 = UP;
byte fadeDirection2 = UP;
byte fadeDirection3 = UP;

// Global Fade Value
// but be bigger than byte and signed, for rollover
int fadeValue1 = 0;
int fadeValue2 = 0;
int fadeValue3 = 0;

// How smooth to fade?
byte fadeIncrement1 = 3;
byte fadeIncrement2 = 2;
byte fadeIncrement3 = 1;

// millis() timing Variable, just for fading
unsigned long previousFadeMillis;
unsigned long fade1Millis;
unsigned long fade2Millis;
unsigned long fade3Millis;

// How fast to increment?
int fadeInterval1 = 30;
int fadeInterval2 = 40;
int fadeInterval3 = 50;

//*******************************PWM fade code above

void setup() {
  delay( 2000 ); //safety startup delay

  analogWrite(fuzziesPin1, fadeValue1);
  analogWrite(fuzziesPin2, fadeValue2);
  analogWrite(fuzziesPin3, fadeValue3);

  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  doTheFade(currentMillis);
}

void doTheFade(unsigned long thisMillis) {
  {
    if (thisMillis - previousFadeMillis >= fadeInterval1) {
      if (fadeDirection1 == UP) {
        fadeValue1 = fadeValue1 + fadeIncrement1;
        if (fadeValue1 >= maxPWM) {
          fadeValue1 = maxPWM;
          fadeDirection1 = DOWN;
        }
      } else if (fadeDirection1 == DOWN) {
        fadeValue1 = fadeValue1 - fadeIncrement1;
        if (fadeValue1 <= minPWM) {
          fadeValue1 = minPWM;
          fade1Millis = thisMillis;
          fadeDirection1 = HOVER;
        }
      }
      else if (fadeDirection1 == HOVER && thisMillis - fade1Millis >= 5000) {
        fadeDirection1 = UP;
      }
    }




    if (thisMillis - previousFadeMillis >= fadeInterval2) {
      if (fadeDirection2 == UP) {
        fadeValue2 = fadeValue2 + fadeIncrement2;
        if (fadeValue2 >= maxPWM) {
          fadeValue2 = maxPWM;
          fadeDirection2 = DOWN;
        }
      } else if (fadeDirection2 == DOWN) {
      fadeValue2 = fadeValue2 - fadeIncrement2;

      if (fadeValue2 <= minPWM) {
          fadeValue2 = minPWM;
          fade2Millis = thisMillis;
          fadeDirection2 = HOVER;
        }
      }
      else if (fadeDirection2 == HOVER && thisMillis - fade1Millis >= 6500) {
        fadeDirection2 = UP;
      }
    }

    if (thisMillis - previousFadeMillis >= fadeInterval3) {
      if (fadeDirection3 == UP) {
        fadeValue3 = fadeValue3 + fadeIncrement3;
        if (fadeValue3 >= maxPWM) {
          fadeValue3 = maxPWM;
          fadeDirection3 = DOWN;
        }
      } else if (fadeDirection3 == DOWN) {
      fadeValue3 = fadeValue3 - fadeIncrement3;

      if (fadeValue3 <= minPWM) {
          fadeValue3 = minPWM;
          fade3Millis = thisMillis;
          fadeDirection3 = HOVER;
        }
      }
      else if (fadeDirection3 == HOVER && thisMillis - fade3Millis >= 3500) {
        fadeDirection3 = UP;
      }
    }
  }

  analogWrite(fuzziesPin1, fadeValue1);
  analogWrite(fuzziesPin2, fadeValue2);
  analogWrite(fuzziesPin3, fadeValue3);

  previousFadeMillis = thisMillis;

  Serial.print("                                         fadeValue1 | 2 | 3 = ");
  Serial.print(fadeValue1);
  Serial.print(" | ");
  Serial.print(fadeValue2);
  Serial.print(" | ");
  Serial.println(fadeValue3);
  Serial.print("Current | fade1 millis are: ");
  Serial.print(thisMillis);
  Serial.print(" | ");
  Serial.println(fade1Millis);
  Serial.print(                                                                      "Fade direction 1 is:");
  Serial.println(fadeDirection1);

}
