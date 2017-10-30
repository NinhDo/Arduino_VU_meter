/*
 * VU meter for Neo Pixel by Ninh Do
 * 
 * Reads the analog signal from the music and lights up the LEDs respectively
 * Created for 2x24 Neo Pixel LED rings, one larger than the other
 * + 1x12 Neo Pixel LED ring that is smaller that sits in the middle.
 * 
 * Total number of LEDs: 60
 * 
 * Uses analogReference(INTERNAL) to because the audio signals are really weak
 * 
 * Filterstuff from Boldevin http://www.instructables.com/id/Sonic-Bow-Tie-by-David-B-Engen/
 */

#include <Adafruit_NeoPixel.h>

#define LEDPIN 6        // Change if you want to
#define AUDIOPIN A0     // Change if you want to

const uint8_t NUMPIXELS = 60;    // Total  number of pixels
const uint8_t OUTER_START = 0;   // When the outer ring starts
const uint8_t OUTER_END = 24;    // When the outer ring ends + 1 (for looping)
const uint8_t CENTER_START = 24; // When the center ring starts
const uint8_t CENTER_END = 48;   // When the center ring ends + 1 (for looping)
const uint8_t INNER_START = 48;  // When the inner ring starts
const uint8_t INNER_END = 60;    // When the inner ring ends 0 1 (for looping)

// Colors. Some value between 0-255
// Neopixels are really bright, so you should keep these low unless you want a rave party
// Green Light
const uint8_t GREEN_R = 0;
const uint8_t GREEN_G = 50;
const uint8_t GREEN_B = 0;

// Yellow Light
const uint8_t YELLOW_R = 50;
const uint8_t YELLOW_G = 25;
const uint8_t YELLOW_B = 0;

// Orange Light
const uint8_t ORANGE_R = 50;
const uint8_t ORANGE_G = 8;
const uint8_t ORANGE_B = 0;

// Red Light
const uint8_t RED_R = 50;
const uint8_t RED_G = 0;
const uint8_t RED_B = 0;

// Threshold of the colors (percentage). Red threshold is anything above ORANGE_THRESHOLD
const uint8_t GREEN_THRESHOLD = 60;
const uint8_t YELLOW_THRESHOLD = 75;
const uint8_t ORANGE_THRESHOLD = 90;

// The LED Rings, connected in series
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

// A list of amplitude scalesin
const double listOfScales[] = {
  0.0001000,// -80   dB
  0.0001334,// -77.5 dB
  0.0001778,// -75   dB
  0.0002371,// -72.5 dB
  0.0003162,// -70   dB
  0.0004217,// -67.5 dB
  0.0005623,// -65   dB
  0.0007499,// -62.5 dB
  0.001000, // -60   dB
  0.001334, // -57.5 dB
  0.001778, // -55   dB
  0.002371, // -52.5 dB
  0.003162, // -50   dB
  0.004217, // -47.5 dB
  0.005623, // -45   dB
  0.007499, // -42.5 dB
  0.01000,  // -40   dB
  0.01334,  // -37.5 dB
  0.01778,  // -35   dB
  0.02371,  // -32.5 dB
  0.03162,  // -30   dB
  0.04217,  // -27.5 dB
  0.05623,  // -25   dB
  0.07499,  // -22.5 dB
  0.1000,   // -20   dB
  0.1334,   // -17.5 dB
  0.1778,   // -15   dB
  0.2371,   // -12.5 dB
  0.3162,   // -10   dB
  0.4217,   // -7.5  dB
  0.5623,   // -5    dB
  0.7499,   // -2.5  dB
  1.000,    // +0    dB
  1.334,    // +2.5  dB    
  1.778,    // +5    dB
  2.371,    // +7.5  dB
  3.162,    // +10   dB
  4.217,    // +12.5 dB
  5.623,    // +15   dB
  7.499,    // +17.5 dB
  10.00,    // +20   dB
};

const uint8_t scaleSize = 24; // Should equal the number of LEDs in one ring (greatest number if different numbers)
const double scale[scaleSize] = {
  0.0001000,// -80   dB
  0.0001778,// -75   dB
  0.0003162,// -70   dB
  0.0005623,// -65   dB
  0.001000, // -60   dB
  0.001778, // -55   dB
  0.003162, // -50   dB
  0.005623, // -45   dB
  0.01000,  // -40   dB
  0.01778,  // -35   dB
  0.03162,  // -30   dB
  0.05623,  // -25   dB
  0.1000,   // -20   dB
  0.1778,   // -15   dB
  0.3162,   // -10   dB
  0.5623,   // -5    dB
  1.000,    // +0    dB
  1.778,    // +5    dB
  3.162,    // +10   dB
  5.623,    // +15   dB
  10.00,    // +20   dB
  17.78,    // +25   dB
  31.62,    // +30   dB
  56.23,    // +35   dB
};

const uint8_t sampleSize = 20;
uint16_t samples[sampleSize];

double sigma4000;
double sigma2000;
double sigma400;

double max4000;
double max2000;
double max400;

const double sigmaReducer4000 = 0.9;
const double sigmaReducer2000 = 0.4;
const double sigmaReducer400 = 0.5;

const double maximumReducer2000 = 0.7;
const double maximumReducer4000 = 0.7;
const double maximumReducer400 = 0.7;

/*
 * void setPixel
 *  Turns on pixel at the given point + offset
 * 
 * Parameters:
 *  uint8_t i       - The pixel you want to turn on
 *  uint8_t OFFSET  - The offset of the pixel (0 if only 1 ring)
 *  uint8_t PIXELS  - The number of pixels on the ring
 *  bool off        - To turn off a pixel. Default false
  */

void setPixel(uint8_t i, uint8_t OFFSET, uint8_t PIXELS, bool off = false) {
  if(off) {
    pixels.setPixelColor(i + OFFSET, 0, 0, 0);
    return;
  }
  // i-th pixel percentage of the ring
  uint8_t p = i * 100 / PIXELS;
  if (p < GREEN_THRESHOLD) {
    pixels.setPixelColor(i + OFFSET, GREEN_R, GREEN_G, GREEN_B);
  } else if (p < YELLOW_THRESHOLD) {
    pixels.setPixelColor(i + OFFSET, YELLOW_R, YELLOW_G, YELLOW_B);
  } else if (p < ORANGE_THRESHOLD) {
    pixels.setPixelColor(i + OFFSET, ORANGE_R, ORANGE_G, ORANGE_B);
  } else {
    pixels.setPixelColor(i + OFFSET, RED_R, RED_G, RED_B);
  }
}


/*
 * void turnOn
 *  Loops through and sets the pixel
 *  Also sets the maximum value if it needs to
 *  
 * Parameters:
 *  uint8_t RING_START    - The offset of the start LED
 *  double& sigma         - Reference to which sigma to use
 *  double& maximum       - Reference to which maximum to use
 *  double sigmaReducer   - To reduce (or enhance) the sigma value. Default 1.0
 *  double maximumReducer - To reduce (or enhance) the maximum value. Default 1.0
 *  uint8_t pixels        - The number of pixels in the ring
 */
void turnOn(uint8_t RING_START, double& sigma, double& maximum, double sigmaReducer = 1.0, double maximumReducer = 1.0, uint8_t pixels = 24) {
  if (sigma > maximum * 1.5) {
    maximum = sigma;
    for (int i = 0; i < pixels; i++) {
      setPixel(i, RING_START, pixels);
    }
  } else {
    for (uint8_t i = 0; i < pixels; i++) {
      if (sigma * sigmaReducer > (maximum * scale[i * (24 / pixels)])) {
        setPixel(i, RING_START, pixels);
      } else {
        setPixel(i, RING_START, pixels, true);
      }
    }
  }
}


// Bandpass filter stuff from Boldevin (Link at the top)
void calc4000() {
  for (uint8_t i = 0; i < sampleSize / 2; i++) {
    sigma4000 += sq(abs(samples[2 * i] - samples[2 * i + 1]));
  }
}

void calc2000() {
  for (uint8_t i = 0; i < sampleSize / 4; i++) {
    sigma2000 += sq(abs(samples[2 * i] + samples[2 * i + 1] - samples[2 * 2] + samples[2 * i + 3]));
  }
}

void calc400() {
  uint8_t sum400Vec[6] = { 0, 0, 0, 0, 0, 0 };
  for (uint8_t i = 0; i < sampleSize / 10; i++) {
    for (uint8_t j = 0; j < 10; j++) {
      sum400Vec[i] += samples[i * 10 + j];
    }
  }
  sigma400 = sq(abs((sum400Vec[0] - sum400Vec[1]) + (sum400Vec[2] - sum400Vec[3]) + (sum400Vec[4] - sum400Vec[5])));
}

void setup() {
  //analogReference(INTERNAL);                  // Set analog reference voltage to INTERNAL (1.1V). Without this, the values from the audio input is too low for the arduino to care.Op-Amp to fix?
  analogRead(0);                              // Read once to clear it
  pixels.begin();                             // Initialize pixels
  for (uint8_t i = 0; i < sampleSize; i++) {  // Initialize samples array
    samples[i] = 0;
  }
}

void loop() {
  // Gather samples
  for (uint8_t i = 0; i < sampleSize; i++) {
    samples[i] = analogRead(A0);
  }
  // Reset sigmas
  sigma4000 = 0;
  sigma2000 = 0;
  sigma400 = 0;

  // Calculate new sigmas
  calc4000();
  calc2000();
  calc400();

  // Turn on stuff
  turnOn(OUTER_START, sigma4000, max4000, sigmaReducer4000, maximumReducer4000, 24);
  turnOn(CENTER_START, sigma2000, max2000, sigmaReducer2000, maximumReducer2000, 24);
  turnOn(INNER_START, sigma400, max400, sigmaReducer400, maximumReducer400, 12);

  // Render
  pixels.show();
}
