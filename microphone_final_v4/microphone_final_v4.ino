/*
 * ===============================================
 * COMPLETE ARDUINO MICROPHONE PROJECT v4.0
 * ===============================================
 * 
 * Features:
 * - Real-time audio visualization (Serial Plotter)
 * - LED triggers (volume, frequency, clap, pattern)
 * - VU meter display (Serial Monitor)
 * - SIMPLE AMPLITUDE DETECTION 
 * - Works WITHOUT capacitor (DC bias handled in software)
 * - Detects claps, voice, music, and continuous sounds
 * 
 * Hardware Connections:
 * ┌─────────────────────────────────────────────────────┐
 * │ MICROPHONE CIRCUIT (NO CAPACITOR NEEDED!)           │
 * ├─────────────────────────────────────────────────────┤
 * │ Arduino 5V   → 2kΩ resistor → 220Ω resistor        │
 * │                                    ↓                │
 * │                            Mic Pin 1 (+)            │
 * │                                    ↓                │
 * │                            Directly to A0           │
 * │                                                     │
 * │ Mic Pin 2    → GND                                  │
 * │                                                     │
 * │ LED:                                                │
 * │ Pin 13  → 220Ω resistor → LED (+) → LED (-) → GND   |
 * | Pin 12 → 220Ω resistor → LED (+) → LED (-) → GND    |
 * └─────────────────────────────────────────────────────┘
 * 
 * IMPORTANT NOTES:
 * ───────────────
 * 1. USE 5V, NOT 3.3V!
 *    - The mic works 1-10V range, rated at 3V
 *    - 5V gives much stronger signal than 3.3V
 *    - Your signal will be too weak with 3.3V
 * 
 * 2. NO CAPACITOR NEEDED!
 *    - Original circuit uses 10µF capacitor to block DC
 *    - This code handles DC bias in software (easier!)
 *    - Connect A0 DIRECTLY to junction (Mic Pin 1 + 220Ω)
 * 
 * 3. MICROPHONE POLARITY!
 *    - Pin 1 (marked with + or red jumper wire) → resistors
 *    - Pin 2 (unmarked or orange jumper wire) → GND
 * 
 * Instructions:
 * ─────────────
 * 1. Wire the circuit (see ASCII diagram above)
 * 2. Upload this sketch
 * 3. Open Tools → Serial Plotter (115200 baud)
 * 4. Make noise and watch the graph!
 * 5. Adjust VOLUME_THRESHOLD if LED too sensitive/insensitive
 */

// ==================== CONFIGURATION ====================

const int MIC_PIN = A0;
const int LED_PIN_1 = 13;
const int LED_PIN_2 = 12; // Optional for other trigger metrics 
const int LED_PIN_3 = 11; /// Optional for other trigger metrics

// Choose ONE display mode
#define DISPLAY_PLOTTER        // Serial Plotter

// Choose ONE trigger mode
#define TRIGGER_VOLUME         // LED on when loud - START HERE
// #define TRIGGER_FREQUENCY   // Different LEDs for frequencies (needs 3 LEDs)
#define TRIGGER_CLAP        // LED flash on clap only
// #define TRIGGER_PATTERN     // 3 claps in 2 seconds

// ADJUST THESE!
const int VOLUME_THRESHOLD = 3;    // 2-15 typical
const int CLAP_THRESHOLD = 50;     // 10-30 typical

// Sampling settings
const int NUM_SAMPLES = 100;       // For amplitude detection

// ==================== GLOBAL VARIABLES ====================

// For pattern/clap detection
int clapCount = 0;
unsigned long firstClapTime = 0;
unsigned long lastClapTime = 0;
int lastAmplitude = 0;

// For frequency detection (if needed)
int samples[128];
int sampleIndex = 0;

// ==================== SETUP ====================

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  
  #ifdef TRIGGER_VOLUME
    Serial.println("Mode: VOLUME");
  #endif
  #ifdef TRIGGER_FREQUENCY
    Serial.println("Mode: FREQUENCY (needs 3 LEDs)");
  #endif
  #ifdef TRIGGER_CLAP
    Serial.println("Mode: CLAP");
  #endif
  #ifdef TRIGGER_PATTERN
    Serial.println("Mode: PATTERN (3 claps)");
  #endif
  
  Serial.print("Threshold: ");
  Serial.println(VOLUME_THRESHOLD);
  Serial.println();
  delay(1000);
}

// ==================== MAIN LOOP ====================

void loop() {
  // ========== AMPLITUDE DETECTION ==========
  
  int minVal = 1023;
  int maxVal = 0;
  
  // Sample quickly - same as simple version
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int val = analogRead(MIC_PIN);
    
    if (val < minVal) minVal = val;
    if (val > maxVal) maxVal = val;
    
    // ALSO store some samples for frequency detection
    if (i % 10 == 0 && sampleIndex < 128) {
      samples[sampleIndex] = val;
      sampleIndex++;
    }
  }
  
  // Reset sample index
  if (sampleIndex >= 128) {
    sampleIndex = 0;
  }
  
  // Calculate amplitude
  int amplitude = maxVal - minVal;
  
  // ========== DISPLAY ==========
  
  #ifdef DISPLAY_PLOTTER
    Serial.print("Amplitude:");
    Serial.print(amplitude);
    Serial.print(",");
    Serial.print("Threshold:");
    Serial.println(VOLUME_THRESHOLD);
  #endif
  
  #ifdef DISPLAY_VU_METER
    displayVUMeter(amplitude);
  #endif
  
  // ========== LED TRIGGERS ==========
  
  #ifdef TRIGGER_VOLUME
    // Simple volume trigger
    if (amplitude > VOLUME_THRESHOLD) {
      digitalWrite(LED_PIN_1, HIGH);
    } else {
      digitalWrite(LED_PIN_1, LOW);
    }
  #endif
  
  #ifdef TRIGGER_FREQUENCY
    // Frequency detection
    int freq = detectFrequency();
    
    if (freq > 5 && freq < 15 && amplitude > 3) {
      digitalWrite(LED_PIN_1, HIGH);
      digitalWrite(LED_PIN_2, LOW);
      digitalWrite(LED_PIN_3, LOW);
    }
    else if (freq >= 15 && freq < 50 && amplitude > 3) {
      digitalWrite(LED_PIN_1, LOW);
      digitalWrite(LED_PIN_2, HIGH);
      digitalWrite(LED_PIN_3, LOW);
    }
    else if (freq >= 50 && amplitude > 3) {
      digitalWrite(LED_PIN_1, LOW);
      digitalWrite(LED_PIN_2, LOW);
      digitalWrite(LED_PIN_3, HIGH);
    }
    else {
      digitalWrite(LED_PIN_1, LOW);
      digitalWrite(LED_PIN_2, LOW);
      digitalWrite(LED_PIN_3, LOW);
    }
  #endif
  
  #ifdef TRIGGER_CLAP
    // Clap detection
    int amplitudeChange = amplitude - lastAmplitude;
    
    if (amplitudeChange > CLAP_THRESHOLD && millis() - lastClapTime > 300) {
      digitalWrite(LED_PIN_2, HIGH);
      lastClapTime = millis();
      delay(100);
      digitalWrite(LED_PIN_2, LOW);
    }
    
    lastAmplitude = amplitude;
  #endif
  
  #ifdef TRIGGER_PATTERN
    // Pattern detection
    int amplitudeChange = amplitude - lastAmplitude;
    
    if (amplitudeChange > CLAP_THRESHOLD && millis() - lastClapTime > 200) {
      clapCount++;
      if (clapCount == 1) {
        firstClapTime = millis();
      }
      lastClapTime = millis();
      Serial.print("Clap ");
      Serial.println(clapCount);
    }
    
    if (clapCount >= 3 && millis() - firstClapTime < 2000) {
      digitalWrite(LED_PIN_1, HIGH);
      Serial.println("*** PATTERN! ***");
      delay(1000);
      digitalWrite(LED_PIN_1, LOW);
      clapCount = 0;
    }
    
    if (millis() - firstClapTime > 2000) {
      clapCount = 0;
    }
    
    lastAmplitude = amplitude;
  #endif
  
  delay(10);
}

// ==================== FUNCTIONS ====================
int detectFrequency() {
  // Calculate average
  long sum = 0;
  for (int i = 0; i < 128; i++) {
    sum += samples[i];
  }
  int average = sum / 128;
  
  // Count zero crossings
  int crossings = 0;
  bool wasAbove = (samples[0] > average);
  
  for (int i = 1; i < 128; i++) {
    bool isAbove = (samples[i] > average);
    if (wasAbove != isAbove) {
      crossings++;
      wasAbove = isAbove;
    }
  }
  
  return crossings;
}

void displayVUMeter(int amplitude) {
  Serial.print("\r");
  
  int level = map(amplitude, 0, 30, 0, 40);
  level = constrain(level, 0, 40);
  
  Serial.print("Amplitude: ");
  
  for (int i = 0; i < 40; i++) {
    if (i < level) {
      if (i < 20) {
        Serial.print("█");
      } else if (i < 30) {
        Serial.print("▓");
      } else {
        Serial.print("▒");
      }
    } else {
      Serial.print("░");
    }
  }
  
  Serial.print(" | ");
  Serial.print(amplitude);
  Serial.print("   ");
  
  Serial.flush();
}
