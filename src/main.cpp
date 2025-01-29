#include <Arduino.h>
#include <JC_Button.h>         // Include the JC_Button library for button handling
#include <Adafruit_NeoPixel.h> // Include the Adafruit NeoPixel library for LED control

// Pin Definitions
#define PIN 5        // Pin for NeoPixel data
#define NUMPIXELS 2  // Number of NeoPixels
#define BUZZER_PIN 6 // Pin for the buzzer
#define BUTTON_PIN 7 // Pin for the button

// Timing Definitions
#define DELAYVAL 300        // Delay for LED animations
#define MAX_FOCUS_TIME 5000 // Max focus time (5 seconds for testing, change to 25 minutes: 25 * 60 * 1000)
#define MAX_BREAK_TIME 1000 // Max break time (1 second for testing, change to 5 minutes: 5 * 60 * 1000)
#define MAX_BREAKS 4        // Maximum number of breaks allowed

// State Definitions
#define INITIAL 0        // Initial state
#define FOCUS 1          // Focus state
#define BREAK 2          // Break state
#define FINISHED 3       // Finished state
#define BREAK_FINISHED 4 // Break finished state

// NeoPixel and Button Objects
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Button myBtn(BUTTON_PIN);

// Global Variables
unsigned long focusTimer = 0; // Timer for focus state
unsigned long breakTimer = 0; // Timer for break state
unsigned long timer = 0;      // General timer
int brakes = 0;               // Number of breaks taken
uint8_t state = INITIAL;      // Current state

// Function Declarations
void lightsCycle();
void ledsOff();
void beep();
void initialStartSequence();

// Light Animation Function
void lightsCycle()
{
  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red
  pixels.setPixelColor(1, pixels.Color(0, 0, 0));   // Off
  pixels.show();
  delay(DELAYVAL);

  pixels.setPixelColor(0, pixels.Color(0, 0, 0));   // Off
  pixels.setPixelColor(1, pixels.Color(0, 255, 0)); // Green
  pixels.show();
  delay(DELAYVAL);
}

// Turn Off LEDs
void ledsOff()
{
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.setPixelColor(1, pixels.Color(0, 0, 0));
  pixels.show();
}

// Buzzer Beep Function
void beep()
{
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
}

// Initial Start Sequence
void initialStartSequence()
{
  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(1, pixels.Color(0, 255, 0)); // Green
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));   // Off
  pixels.show();

  // Wait for button press to start
  while (myBtn.lastChange() == 0)
  {
    myBtn.read();
  }

  // Play light animation and beep
  for (int i = 0; i < 5; i++)
  {
    lightsCycle();
  }
  ledsOff();
  beep();

  // Initialize timers and set state to FOCUS
  timer = millis();
  focusTimer = millis();
  breakTimer = 0;
  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red
  pixels.setPixelColor(1, pixels.Color(0, 0, 0));   // Off
  pixels.show();
  state = FOCUS;
  Serial.println("Focus Mode on");
}

// Setup Function
void setup()
{
  myBtn.begin(); // Initialize the button
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  initialStartSequence(); // Start the initial sequence
}

// Main Loop
void loop()
{
  myBtn.read(); // Read the button state

  switch (state)
  {
  case FOCUS:
    if (millis() - timer >= 1000)
    {
      Serial.println("Focus Timer: " + String((millis() - focusTimer) / 1000) + " Seconds");
      timer = millis();
    }
    if (millis() - focusTimer >= MAX_FOCUS_TIME)
    {
      state = FINISHED;
      Serial.println("Focus Finished: Timer completed");
    }
    if (myBtn.wasPressed() && myBtn.lastChange() >= 100)
    {
      state = BREAK;
      breakTimer = millis(); // Record break start time
      Serial.println("Break started");
      Serial.println("In Focus state: You focused for " + String((millis() - focusTimer) / 1000) + " Seconds, switching to break mode.");
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));   // Off
      pixels.setPixelColor(1, pixels.Color(0, 255, 0)); // Green
      pixels.show();
      beep();
    }
    break;

  case BREAK:
    if (millis() - timer >= 1000)
    {
      Serial.println("In Break State Break Timer: " + String((millis() - breakTimer) / 1000) + " Seconds");
      timer = millis();
    }
    if (millis() - breakTimer >= MAX_BREAK_TIME)
    {
      state = BREAK_FINISHED;
      brakes += 1; // Increment break count
      focusTimer = millis();
      Serial.println("Break Finished");
      Serial.println("You took a break for " + String((millis() - breakTimer) / 1000) + " Seconds");
      Serial.println("Focus started");
      pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red
      pixels.setPixelColor(1, pixels.Color(0, 0, 0));   // Off
      pixels.show();
      beep();
      delay(500);
      beep();
      delay(500);
      beep();
    }
    break;

  case FINISHED:
    Serial.println("In Finished State You focused for " + String((millis() - focusTimer) / 1000) + " Seconds");
    Serial.println("You took " + String(brakes) + " breaks");
    Serial.println("You are done for the day");
    breakTimer = 0;
    focusTimer = 0;
    timer = 0;                                        // Reset the timer
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));   // Off
    pixels.setPixelColor(1, pixels.Color(0, 255, 0)); // Green
    pixels.show();
    beep();
    delay(500);
    beep();
    delay(500);
    beep();
    state = INITIAL;
    break;

  case INITIAL:
    initialStartSequence();
    break;

  default:
    break;
  }
}