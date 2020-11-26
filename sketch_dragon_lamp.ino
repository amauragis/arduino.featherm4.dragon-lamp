// Dragon Lamp code
// targetting feather M4 express

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>

// pin assignments
constexpr int PIN_ON_BOARD_LED = 8;
constexpr int PIN_NEO_STRING   = 12;
constexpr int PIN_MODE_BTN     = 10;
constexpr int PIN_DIM_POT      = 17;

// neopixel configs
constexpr int NEO_STRING_NUM_PIXELS   = 21;
constexpr uint32_t ON_BOARD_LED_MAP   = NEO_GRB;
constexpr uint32_t NEO_STRING_LED_MAP = NEO_GRBW;

// debounce config
constexpr unsigned long debounce_delay = 50;

// globals
static Adafruit_NeoPixel_ZeroDMA on_board( 1, PIN_ON_BOARD_LED, ON_BOARD_LED_MAP );
static Adafruit_NeoPixel_ZeroDMA leds( NEO_STRING_NUM_PIXELS, PIN_NEO_STRING, NEO_STRING_LED_MAP );
static uint8_t brightness_level = 0;

// fwd declare lighting functions
void lighting_white();
void lighting_red();
void lighting_rainbow();
void lighting_fire();
void lighting_green_fire();

// global array for holding lighting functions
void ( *lighting_functions[] )() = { lighting_white, lighting_red, lighting_rainbow, lighting_fire, lighting_green_fire };
constexpr int num_lighting_modes = sizeof( lighting_functions ) / sizeof( lighting_functions[ 0 ] );
static int lighting_idx          = 0; // active lighting function index (and powerup mode)

constexpr uint16_t hue_deg_to_u16( int D )
{
    return 65535L * D / 360;
}

constexpr uint8_t pct_to_u8( int P )
{
    return 255 * P / 100;
}

// set the status led to the specified color
void set_status_led( uint16_t pixel_hue )
{
    for( int i = 0; i < on_board.numPixels(); ++i )
    {
        on_board.setPixelColor( i, leds.gamma32( leds.ColorHSV( pixel_hue ) ) );
    }
    on_board.show();
}

// debounced check for a negative edge on the mode button, increment global lighting index
void check_mode_btn()
{
    static int button_state                 = HIGH;
    static int last_button_state            = HIGH;
    static unsigned long last_debounce_time = 0;
    int mode_button_now                     = digitalRead( PIN_MODE_BTN );

    if( mode_button_now != last_button_state )
        last_debounce_time = millis();

    if( ( millis() - last_debounce_time ) > debounce_delay )
    {
        // if the button state has changed:
        if( mode_button_now != button_state )
        {
            button_state = mode_button_now;

            if( button_state == LOW )
            {
                lighting_idx        = ( lighting_idx + 1 ) % num_lighting_modes;
                uint16_t status_hue = hue_deg_to_u16( lighting_idx * ( 360 / num_lighting_modes ) );
                set_status_led( status_hue );
                Serial.print( "New Lighting Mode: " );
                Serial.print( lighting_idx, DEC );
                Serial.print( "\n" );
            }
        }
    }
    last_button_state = mode_button_now;
}

// check adc pin for brightness setting, globally apply
void check_brightness()
{
    brightness_level = analogRead( PIN_DIM_POT );

    leds.setBrightness( brightness_level );
}

void lighting_solid( uint8_t r, uint8_t g, uint8_t b, uint8_t w )
{
    for( int i = 0; i < leds.numPixels(); ++i )
    {
        leds.setPixelColor( i, leds.Color( r, g, b, w ) );
    }
}

void lighting_white()
{
    lighting_solid( 128, 128, 128, 255 );
}

void lighting_red()
{
    lighting_solid( 255, 0, 0, 0 );
}

void lighting_rainbow()
{
    uint32_t rainbow_start = micros() / 24;
    for( int i = 0; i < leds.numPixels(); ++i )
    {
        uint32_t pixel_hue = rainbow_start + ( i * 65536L / leds.numPixels() );
        // reverse the "flow" of color so it moves from 1->end rather than end->1
        leds.setPixelColor( leds.numPixels() - 1 - i, leds.gamma32( leds.ColorHSV( pixel_hue ) ) );
    }
}

void lighting_green_fire()
{
    constexpr uint32_t update_interval = 50; // ms

    const uint32_t base_color = leds.gamma32( leds.ColorHSV( hue_deg_to_u16( 110 ), pct_to_u8( 90 ), pct_to_u8( 100 ) ) );

    constexpr uint8_t num_rdm_pix = 12;
    constexpr uint16_t rdm_hue_lo = hue_deg_to_u16( 75 );
    constexpr uint16_t rdm_hue_hi = hue_deg_to_u16( 180 );
    constexpr uint8_t rdm_sat_lo  = pct_to_u8( 60 );
    constexpr uint8_t rdm_sat_hi  = pct_to_u8( 100 );

    static uint32_t last_update = 0;

    uint32_t now = millis();

    if( ( now - last_update ) <= update_interval )
        return;

    last_update = now;

    // apply base color
    for( int i = 0; i < leds.numPixels(); ++i )
    {
        leds.setPixelColor( i, base_color );
    }

    for( int i = 0; i < num_rdm_pix; ++i )
    {
        if( i < num_rdm_pix / 3 ) // make a third of the pixels some degree of purple
        {
            int pix_ix            = random( 0, leds.numPixels() );
            uint16_t hue          = hue_deg_to_u16( 280 );
            uint8_t sat           = random( rdm_sat_lo, rdm_sat_hi );
            constexpr uint8_t val = pct_to_u8( 100 );

            leds.setPixelColor( pix_ix, leds.gamma32( leds.ColorHSV( hue, sat, val ) ) );
        }
        else // the rest use the random parameters
        {
            int pix_ix            = random( 0, leds.numPixels() );
            uint16_t hue          = random( rdm_hue_lo, rdm_hue_hi );
            uint8_t sat           = random( rdm_sat_lo, rdm_sat_hi );
            constexpr uint8_t val = pct_to_u8( 100 );

            leds.setPixelColor( pix_ix, leds.gamma32( leds.ColorHSV( hue, sat, val ) ) );
        }
    }
}

void lighting_fire()
{
    constexpr uint32_t update_interval = 50; // ms

    const uint32_t base_color = leds.gamma32( leds.ColorHSV( hue_deg_to_u16( 30 ), pct_to_u8( 85 ), pct_to_u8( 100 ) ) );

    constexpr uint8_t num_rdm_pix = 5;
    constexpr uint16_t rdm_hue_lo = hue_deg_to_u16( 0 );
    constexpr uint16_t rdm_hue_hi = hue_deg_to_u16( 55 );
    constexpr uint8_t rdm_sat_lo  = pct_to_u8( 60 );
    constexpr uint8_t rdm_sat_hi  = pct_to_u8( 100 );

    static uint32_t last_update = 0;

    uint32_t now = millis();

    if( ( now - last_update ) <= update_interval )
        return;

    last_update = now;

    // apply base color
    for( int i = 0; i < leds.numPixels(); ++i )
    {
        leds.setPixelColor( i, base_color );
    }

    for( int i = 0; i < num_rdm_pix; ++i )
    {
        int pix_ix            = random( 0, leds.numPixels() );
        uint16_t hue          = random( rdm_hue_lo, rdm_hue_hi );
        uint8_t sat           = random( rdm_sat_lo, rdm_sat_hi );
        constexpr uint8_t val = pct_to_u8( 100 );

        leds.setPixelColor( pix_ix, leds.gamma32( leds.ColorHSV( hue, sat, val ) ) );
    }
}

// ███████╗███████╗████████╗██╗   ██╗██████╗
// ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ███████║███████╗   ██║   ╚██████╔╝██║
// ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝
void setup()
{
    randomSeed( analogRead( A0 ) ); // A0 floating

    on_board.begin();
    on_board.setBrightness( 2 );
    uint16_t status_hue = hue_deg_to_u16( lighting_idx * ( 360 / num_lighting_modes ) );
    set_status_led( status_hue );

    leds.begin();
    leds.setBrightness( brightness_level );

    pinMode( PIN_MODE_BTN, INPUT );
    analogReadResolution( 8 );
}

// ██╗      ██████╗  ██████╗ ██████╗
// ██║     ██╔═══██╗██╔═══██╗██╔══██╗
// ██║     ██║   ██║██║   ██║██████╔╝
// ██║     ██║   ██║██║   ██║██╔═══╝
// ███████╗╚██████╔╝╚██████╔╝██║
// ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝
void loop()
{
    check_mode_btn();
    check_brightness();

    // call lighting function from table
    lighting_functions[ lighting_idx ]();

    // paint leds
    leds.show();
}
