// targetting feather M4

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>

const int PIN_ON_BOARD_LED = 8;
const int PIN_NEO_STRING = 12;
const int PIN_MODE_BTN = 10;
const int PIN_DIM_POT = 17;

const int NEO_STRING_NUM_PIXELS = 21;
const uint32_t ON_BOARD_LED_MAP = NEO_GRB;
const uint32_t NEO_STRING_LED_MAP = NEO_GRBW;


Adafruit_NeoPixel_ZeroDMA on_board(1, PIN_ON_BOARD_LED, ON_BOARD_LED_MAP);
Adafruit_NeoPixel_ZeroDMA leds(NEO_STRING_NUM_PIXELS, PIN_NEO_STRING, NEO_STRING_LED_MAP );

static uint8_t brightness_level = 8;

// debouncing state
static int button_state = HIGH;
static int last_button_state = HIGH;
static unsigned long last_debounce_time = 0;
static unsigned long debounce_delay = 50;

static uint32_t fire_last_ts = 0;

//typedef enum {
//  WHITE = 0,
//  FIRE = 1,
//  GREEN_FIRE = 2 ,
//  RAINBOW = 3,
//} lighting_mode_t;

//lighting_mode_t lighting_mode;

uint8_t lighting_mode = 4;

//static uint32_t rainbow_state;

void setup() {

  randomSeed(analogRead(A0));

  on_board.begin();
  on_board.setBrightness(64);
  for (int i = 0; i < on_board.numPixels(); i++) {
    on_board.setPixelColor(i, on_board.Color(255, 255, 255));
  }
  on_board.show();

  leds.begin();
  leds.setBrightness(brightness_level);

  pinMode(PIN_MODE_BTN, INPUT);
  analogReadResolution(8);

}

void check_lighting_mode()
{
  int mode_button_now = digitalRead(PIN_MODE_BTN);

  if (mode_button_now != last_button_state) {
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (mode_button_now != button_state) {
      button_state = mode_button_now;

      if (button_state == LOW) {
        //        lighting_mode = WHITE; //( lighting_mode + 1 ) % 4;
        lighting_mode = (lighting_mode + 1) % 6;
        Serial.print("New Lighting Mode: ");
        Serial.print(lighting_mode, DEC);
        Serial.print("\n");
      }
    }
  }
  last_button_state = mode_button_now;
}

void check_brightness()
{
  // read wiper
  int wiper = analogRead(PIN_DIM_POT);
  //  Serial.print("wiper: ");
  //  Serial.print(wiper, DEC);
  //  Serial.print("\n");

  brightness_level = wiper;
  on_board.setBrightness(brightness_level);
  on_board.show();

  leds.setBrightness(brightness_level);
}

void rainbow_effect()
{
  uint32_t rainbow_start = micros() / 24;
  for (int i = 0; i < leds.numPixels(); i++) {
    uint32_t pixelHue = rainbow_start + (i * 65536L / leds.numPixels());
    leds.setPixelColor(i, leds.gamma32(leds.ColorHSV(pixelHue)));
  }
}

void fire_effect()
{
  const uint16_t hue_min = 0;
  const uint16_t hue_max = (65535L) * 60 / 360;
  const uint16_t hue_range = hue_max - hue_min;


  uint32_t now = millis();

  if ((now - fire_last_ts) > 100)
  {
    fire_last_ts = now;
    Serial.print("Updated fire\n");
    uint32_t seed = random(hue_min, hue_max);

    for (int i = 0; i < leds.numPixels(); i++) {
      uint32_t pixelHue = (seed + (i * (hue_range) / leds.numPixels())) % hue_range - hue_min;
      leds.setPixelColor(i, leds.gamma32(leds.ColorHSV(pixelHue)));
    }
  }
  
}

void loop() {

  check_lighting_mode();

  check_brightness();


  switch (lighting_mode) {
    default:
    case 0:
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixelColor(i, leds.Color(0, 0, 0, 255));
      }
      break;

    case 1:
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixelColor(i, leds.Color(255, 0, 0, 0));
      }
      break;

    case 2:
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixelColor(i, leds.Color(0, 255, 0, 0));
      }
      break;

    case 3:
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixelColor(i, leds.Color(0, 0, 255, 0));
      }
      break;

    case 4:
      rainbow_effect();
      break;

    case 5:
      fire_effect();
      break;


  }

  leds.show();


  //  uint16_t i;
  //
  //  // Rainbow cycle
  //  uint32_t elapsed, t, startTime = micros();
  //  for(;;) {
  //    t       = micros();
  //    elapsed = t - startTime;
  //    if(elapsed > 5000000) break; // Run for 5 seconds
  //    uint32_t firstPixelHue = elapsed / 32;
  //    for(i=0; i<strip.numPixels(); i++) {
  //      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
  //      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  //    }
  //    strip.show();
  //  }
}
