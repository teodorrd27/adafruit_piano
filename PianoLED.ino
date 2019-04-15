#include <MIDI.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include "noteList.h"
#include "pitches.h"

MIDI_CREATE_DEFAULT_INSTANCE();

static const unsigned sMaxNumNotes = 16;
MidiNoteList<sMaxNumNotes> midiNotes;
byte curWheelPos = 0;

#define KEYBOARD_LOWEST_NOTE 21
#define KEYBOARD_HIGHEST_NOTE 108
#define STRIP_REVERSED true

#define STRIP_PIN 10
#define STRIP_NUM_PIXELS 73

#define RED 0
#define GREEN 255
#define BLUE 0

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_NUM_PIXELS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

/**
  * True -- set the RED GREEN BLUE flags above to pick your RGB color
  * False -- leave false to continue with effects
  */
bool userDefinedColor = true
// -----------------------------------------------------------------------------

inline void handleGateChanged(bool inGateActive)
{
    digitalWrite(LED_BUILTIN, inGateActive ? LOW : HIGH);
}

inline void pulseGate()
{
    handleGateChanged(false);
    delay(10);
    handleGateChanged(true);
}

// -----------------------------------------------------------------------------

byte noteToStripIndex(byte pitch)
{
    byte index;
    if(pitch < KEYBOARD_LOWEST_NOTE)
      index = 0;
    else if(pitch > KEYBOARD_HIGHEST_NOTE)
      index = STRIP_NUM_PIXELS -1;
    else
      index = (STRIP_NUM_PIXELS -1) * (pitch - KEYBOARD_LOWEST_NOTE) / (KEYBOARD_HIGHEST_NOTE - KEYBOARD_LOWEST_NOTE);
    if(STRIP_REVERSED)
      index = (STRIP_NUM_PIXELS -1) - index;

    return index;
}

void handleNotesChanged(bool isFirstNote = false)
{
    if (midiNotes.empty())
    {
        strip.clear();
        strip.show();
        handleGateChanged(false);
    }
    else
    {
        byte pitch = 0;
        strip.clear();
        for (byte i = 0; i < midiNotes.size(); ++i)
        {
            if(midiNotes.get(i, pitch))
            {
                switch (userDefinedColor){
                  case true:    strip.setPixelColor(noteToStripIndex(pitch), strip.Color(RED, GREEN, BLUE));
                                break;

                  case false:     strip.setPixelColor(noteToStripIndex(pitch), Wheel(curWheelPos & 255));
                                  curWheelPos++;
                                  break;
                }

            }
        }
        strip.show();

        if (midiNotes.getLast(pitch))
        {
            if (isFirstNote)
            {
                handleGateChanged(true);
            }
            else
            {
                pulseGate(); // Retrigger envelopes. Remove for legato effect.
            }
        }
    }
}

// -----------------------------------------------------------------------------

void handleNoteOn(byte inChannel, byte inNote, byte inVelocity)
{
    const bool firstNote = midiNotes.empty();
    midiNotes.add(MidiNote(inNote, inVelocity));
    //digitalWrite(LED_BUILTIN, LOW);
    handleNotesChanged(firstNote);
}

void handleNoteOff(byte inChannel, byte inNote, byte inVelocity)
{
    midiNotes.remove(inNote);
    //digitalWrite(LED_BUILTIN, HIGH);
    handleNotesChanged();
}

// -----------------------------------------------------------------------------

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    //Serial.begin(115200);//For debug only
    digitalWrite(LED_BUILTIN, HIGH);

    strip.begin();
    //strip.show(); // Initialize all pixels to 'off'
    rainbow(10);
    strip.clear();
    strip.show();
}

void loop()
{
    MIDI.read();
}

// ----------------------------------------------------------------------------

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
