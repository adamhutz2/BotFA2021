//  Written/Compiled by Adam Hutz and KJ Wu, OVERGROWN Group, 2021
//  Built upon examples found within the FastLED and TwinkleFOX libraries
//  which were developed by Daniel Garcia and Mark Kriegsman, 2012 - 2015
//  Credit also to James Lewis for his pwm-fade-with-millis.ino example,
//  here used for regular fading of "fuzzy" Avatar-like LED strands.
//  See baldengineer.com/fading-led-analogwrite-millis-example.html

#include "FastLED.h"
#include <Conceptinetics.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define NUM_LEDS      12 //Note: I think there were 8 in the system, but can't hurt to index a few extra just in case
#define LED_TYPE   WS2811
#define COLOR_ORDER   GRB
#define DATA_PIN        8
//#define CLK_PIN       4
#define VOLTS          5
#define MAX_MA       4000
#define BUBBLE_CHANNEL 20
#define HAZE_CHANNEL 21
#define DMX_MASTER_CHANNELS   100
#define RXEN_PIN                2
DMX_Master        dmx_master ( DMX_MASTER_CHANNELS, RXEN_PIN );

#define UP 0
#define DOWN 1
#define HOVER 2

CRGBArray<NUM_LEDS> leds;

//  PIN ASSIGNMENTS
//  *****************************************************************************************
//  DISTANCE SENSOR PIN ASSIGNMENTS:
const int trigPin = 9;
const int echoPin = 10;

//  POTENTIOMETER PIN ASSIGNMENTS:
int durationPot = A0;
int fogPot = A1;
int distancePot = A2;
int cooldownPot = A3;
int delayPot = A4;
int fogOffsetPot = A5;

int fuzziesPin1 = 11;        //These "fuzzies" pins control the Avatar-like anemone light strings
int fuzziesPin2 = 5;         //that protrude through some of the ceramic objects.
int fuzziesPin3 = 6;

//int buttonPin = 12;       //For testing
//int buttonState = 0;      //For testing



//  POTENTIOMETER PARAMETERS
//  ******************************************************************************************
//  DURATION POT PARAMS:
int durationPotValue = 0;
int minDuration = 2000;        //Length of interaction (RANGE: 2 SEC - 30 SECS)
int maxDuration = 30000;
int durationPotValueAdjusted = 0;
bool durationExceeded = 0;

//  FOG POT PARAMS:
int fogPotValue = 0;
int fogPotValueAdjusted = 0;    //Amount of fog (RANGE: 0 - 100 PERCENT (BELOW 30% = 0%))
bool fogTriggered = 0;

//  DISTANCE POT PARAMS:
int distancePotValue = 0;
int distancePotValueAdjusted = 0;
int minDistance = 10;
int maxDistance = 183;          //Distance of trigger (RANGE: 10cm - 183cm)
bool distanceTriggered = 0;

//  COOLDOWN POT PARAMS:
int cooldownPotValue = 0;
int minCooldown = 10000;
int maxCooldown = 300000;       //Delay between spawning events (RANGE: 10 SECS - 300 SECS)
int cooldownPotValueAdjusted = 0;
bool cooldownExceeded = 1;

//  DELAY POT PARAMS:
int delayPotValue = 0;
int minDelay = 0;
int maxDelay = 15000;           //Delay between trigger and activated events (RANGE: 0 SECS - 15 SECS)
int delayPotValueAdjusted = 0;
int delayPotValueAdjustedCountdown = 0;
bool delayExceeded = 0;

//  FOG OFFSET POT PARAMS:
int fogOffsetPotValue = 0;
int minFogOffset = 0;
int maxFogOffset = 5000;        //Delay between when fog begins and when bubbles begin (RANGE: 0 SECS - 5 SECS)
int fogOffsetPotValueAdjusted = 0;  //...and also between when fog ends and bubbles end (same range)
bool fogOffset1Exceeded = 0;
bool fogOffset2Exceeded = 0;

//   OTHER VARIABLES
//  ******************************************************************************************
// ultrasonic distance sensor variables
long duration;
int distance;
bool trigHigh = 0;

// variables dealing with time
unsigned long previousMillis = 0;
unsigned long previousMicros = 0;

// Overall twinkle speed.
// 0 (VERY slow) to 8 (VERY fast).
// 4, 5, and 6 are recommended, default is 4.
#define TWINKLE_SPEED 1

// Overall twinkle density.
// 0 (NONE lit) to 8 (ALL lit at once).
// Default is 5.
#define TWINKLE_DENSITY 2

// How often to change color palettes.            //Only one palette is currently used, so this never changes
#define SECONDS_PER_PALETTE  30
// Also: toward the bottom of the file is an array
// called "ActivePaletteList" which controls which color
// palettes are used; you can add or remove color palettes
// from there freely.

// Background color for 'unlit' pixels
// Can be set to CRGB::Black if desired.
CRGB gBackgroundColor = CRGB::Black;
// Example of dim incandescent fairy light background color
// CRGB gBackgroundColor = CRGB(CRGB::FairyLight).nscale8_video(16);

// If AUTO_SELECT_BACKGROUND_COLOR is set to 1,
// then for any palette where the first two entries
// are the same, a dimmed version of that color will
// automatically be used as the background color.
#define AUTO_SELECT_BACKGROUND_COLOR 0

// If COOL_LIKE_INCANDESCENT is set to 1, colors will
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 0

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;


//   PWM FADING CODE (FOR FUZZY LIGHTS)
//  ******************************************************************************************

// constants for min and max PWM
const int minPWM = 3;             //"Dimmest" value that anemone lights will achieve before reversing direction
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

// How fast to increment? (Increasing values will increase speed of fade)
int fadeInterval1 = 5;
int fadeInterval2 = 10;
int fadeInterval3 = 15;

//   PROGRAM SETUP
//  ******************************************************************************************

void setup() {
  delay( 2000 ); //safety startup delay
  FastLED.setMaxPowerInVoltsAndMilliamps( VOLTS, MAX_MA);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
  .setCorrection(TypicalLEDStrip);

  chooseNextColorPalette(gTargetPalette);

  dmx_master.enable ();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  pinMode(fuzziesPin1, OUTPUT);
  pinMode(fuzziesPin2, OUTPUT);
  pinMode(fuzziesPin3, OUTPUT);

  //  pinMode(buttonPin, INPUT); //For use in testing

  analogWrite(fuzziesPin1, fadeValue1);
  analogWrite(fuzziesPin2, fadeValue2);
  analogWrite(fuzziesPin3, fadeValue3);

  //  Serial.begin(9600);
}


//   PROGRAM LOOP
//  ******************************************************************************************

void loop() {
  unsigned long currentMillis = millis();

  drawTwinkles(leds);           //This function for color selection for the embedded neopixels
  FastLED.show();               //This function for illuminating the embedded neopixels
  doTheFade(currentMillis);     //This function fades the fuzzy lights in and out at intervals
  mapPots();                    //This function reads all six potentiometers and maps their resistance values to "min/max" ranges for each adjustment
  distanceFinding();            //This function reads the distance sensor once per loop

  //  if (buttonState == LOW && cooldownExceeded == 1) {               This line is for testing with a button

  if (distance < distancePotValueAdjusted && cooldownExceeded == 1) {
    //    Serial.print("Distance sensor triggered at ");
    //    Serial.print(distance);
    //    Serial.print(" cm. Starting fog at ");
    //    Serial.print(fogPotValueAdjusted);
    //    Serial.println(" percent.");
    dmx_master.setChannelValue ( HAZE_CHANNEL, fogPotValueAdjusted);
    cooldownExceeded = 0;
    fogTriggered = 1;
    previousMillis = currentMillis;
  }

  if (fogTriggered == 1 && currentMillis - previousMillis > fogOffsetPotValueAdjusted) {
    //    Serial.print("Fog offset of ");
    //    Serial.print(fogOffsetPotValueAdjusted);
    //    Serial.println(" milliseconds exceeded; starting bubbles");
    dmx_master.setChannelValue ( BUBBLE_CHANNEL, 11 );
    fogTriggered = 0;
    fogOffset1Exceeded = 1;
    previousMillis = currentMillis;
  }

  if (fogOffset1Exceeded == 1 && currentMillis - previousMillis > durationPotValueAdjusted) {
    //    Serial.print("Experience duration of ");
    //    Serial.print(durationPotValueAdjusted);
    //    Serial.println(" milliseconds exceeded. Stopping fog.");
    dmx_master.setChannelValue ( HAZE_CHANNEL, 0 );
    fogOffset1Exceeded = 0;
    durationExceeded = 1;
    previousMillis = currentMillis;
  }

  if (durationExceeded == 1 && currentMillis - previousMillis > fogOffsetPotValueAdjusted) {
    //    Serial.print("Fog offset of ");
    //    Serial.print(fogOffsetPotValueAdjusted);
    //    Serial.println(" milliseconds exceeded; stopping bubbles. Cooldown started.");
    dmx_master.setChannelValue ( BUBBLE_CHANNEL, 0 );
    durationExceeded = 0;
    fogOffset2Exceeded = 1;
    previousMillis = currentMillis;
  }

  if (fogOffset2Exceeded == 1 && currentMillis - previousMillis > cooldownPotValueAdjusted) {
    //    Serial.print("Cooldown of ");
    //    Serial.print(cooldownPotValueAdjusted);
    //    Serial.println(" milliseconds exceeded. System ready.");
    fogOffset2Exceeded = 0;
    cooldownExceeded = 1;
    previousMillis = currentMillis;
  }

  else {
    systemShutdown();
  }

  EVERY_N_SECONDS( SECONDS_PER_PALETTE ) {
    chooseNextColorPalette( gTargetPalette );
  }

  EVERY_N_MILLISECONDS( 10 ) {
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 12);
  }
}



//  NEOPIXEL FUNCTIONS, INCLUDING FAST LED and TWINKLEFOX
//  ******************************************************

//  This function loops over each pixel, calculates the
//  adjusted 'clock' that this pixel should use, and calls
//  "CalculateOneTwinkle" on each pixel.  It then displays
//  either the twinkle color of the background color,
//  whichever is brighter.
void drawTwinkles( CRGBSet & L)
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;

  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if ( (AUTO_SELECT_BACKGROUND_COLOR == 1) &&
       (gCurrentPalette[0] == gCurrentPalette[1] )) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if ( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if ( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();

  for ( CRGB& pixel : L) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16 = PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF) >> 4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if ( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color,
      // use the new color.
      pixel = c;
    } else if ( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      pixel = blend( bg, c, deltabright * 8);
    } else {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      pixel = bg;
    }
  }
}

CRGB computeOneTwinkle( uint32_t ms, uint8_t salt)
{
  uint16_t ticks = ms >> (8 - TWINKLE_SPEED);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  uint8_t bright = 0;
  if ( ((slowcycle8 & 0x0E) / 2) < TWINKLE_DENSITY) {
    bright = attackDecayWave8( fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if ( bright > 0) {
    c = ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
    if ( COOL_LIKE_INCANDESCENT == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}

uint8_t attackDecayWave8( uint8_t i)
{
  if ( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i / 2));
  }
}

// This function takes a pixel, and if its in the 'fading down'
// part of the cycle, it adjusts the color a little bit like the
// way that incandescent bulbs fade toward 'red' as they dim.
void coolLikeIncandescent( CRGB & c, uint8_t phase)
{
  if ( phase < 128) return;
  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{ CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight,
  HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight,
  QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight,
  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight
};

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
{ 0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0xE0F0FF
};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM =
{
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
};

// Add or remove palette names from this list to control which color
// palettes are used, and in what order.
const TProgmemRGBPalette16* ActivePaletteList[] = {
  //  &FairyLight_p,
  &Snow_p,
  //  &Ice_p
};

// Advance to the next color palette in the list (above).
void chooseNextColorPalette( CRGBPalette16 & pal)
{
  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);
  static uint8_t whichPalette = -1;
  whichPalette = addmod8( whichPalette, 1, numberOfPalettes);

  pal = *(ActivePaletteList[whichPalette]);
}

void mapPots() {
  // Code that establishes the desired distance
  distancePotValue = analogRead(distancePot);
  distancePotValueAdjusted = round(map(distancePotValue, 0, 1023, minDistance, maxDistance));

  // Code that establishes the desired length of time the system operates
  durationPotValue = analogRead(durationPot);
  durationPotValueAdjusted = round(map(durationPotValue, 0, 1023, minDuration, maxDuration));

  // Code that establishes the wait after trigger before the interaction occurs
  delayPotValue = analogRead(delayPot);
  delayPotValueAdjusted = (map(delayPotValue, 0, 1023, minDelay, maxDelay));

  // Code that establishes the delay after the interaction has occurred
  cooldownPotValue = analogRead(cooldownPot);
  cooldownPotValueAdjusted = map(cooldownPotValue, 0, 1023, minCooldown, maxCooldown);

  // Code that establishes the desired fog level
  fogPotValue = analogRead(fogPot);
  fogPotValueAdjusted = map(fogPotValue, 0, 1023, 0, 100);

  if (fogPotValueAdjusted < 30) {
    fogPotValueAdjusted = 0;
  }

  // Code that establishes the fog/bubble offset
  fogOffsetPotValue = analogRead(fogOffsetPot);
  fogOffsetPotValueAdjusted = map(fogOffsetPotValue, 0, 1023, 0, 10000);

  //  Serial.print("Pot values 1|2|3|4|5|6 = ");
  //  Serial.print(distancePotValueAdjusted);
  //  Serial.print("|");
  //  Serial.print(durationPotValueAdjusted);
  //  Serial.print("|");
  //  Serial.print(delayPotValueAdjusted);
  //  Serial.print("|");
  //  Serial.print(cooldownPotValueAdjusted);
  //  Serial.print("|");
  //  Serial.print(fogPotValueAdjusted);
  //  Serial.print("|");
  //  Serial.println(fogOffsetPotValueAdjusted);

}

void systemShutdown() {
  dmx_master.setChannelValue ( BUBBLE_CHANNEL, 0 );
  dmx_master.setChannelValue ( HAZE_CHANNEL, 0 );
}

void distanceFinding() {
  //  unsigned long currentMicros = micros();

  //  NEW UNTESTED VERSION
  //  digitalWrite(trigPin, LOW);
  //
  //  if (trigHigh == 0 && currentMicros - previousMicros >= 2) {
  //    digitalWrite(trigPin, HIGH);
  //    trigHigh = 1;
  //    previousMicros = currentMicros;
  //  }
  //
  //  if (trigHigh == 1 && currentMicros - previousMicros >= 10) {
  //    digitalWrite(trigPin, LOW);
  //    trigHigh = 0;
  //    previousMicros = currentMicros;
  //  }
  //
  //  duration = pulseIn(echoPin, HIGH);
  //  distance = duration * 0.034 / 2;

  // OLD version:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // For testing, in the absence of a distance sensor, we can use a button:
  //  buttonState = digitalRead(buttonPin);M
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

  //    SERIAL PRINTOUTS FOR TESTING VALUES OF FUZZY LED STRANDS
  //  Serial.print("                                                   fadeValue: 1 | 2 | 3 = ");
  //  Serial.print(fadeValue1);
  //  Serial.print(" | ");
  //  Serial.print(fadeValue2);
  //  Serial.print(" | ");
  //  Serial.print(fadeValue3);
  //  Serial.print(  "|||  Fade direction 1 | 2 | 3 = ");
  //  Serial.print(fadeDirection1);
  //  Serial.print(" | ");
  //  Serial.print(fadeDirection2);
  //  Serial.print(" | ");
  //  Serial.println(fadeDirection3);
}
