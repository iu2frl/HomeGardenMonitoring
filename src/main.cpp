#include <Arduino.h>
#include <arduinoFFT.h>
#include <ESP8266WiFi.h> 
#include <secrets.h> // Secret variables are stored in this file, which is not included with the repo for security reasons.

// Pinout
const uint8_t analog_input = A0;

// FFT variables
const uint16_t samples = 512; //This value MUST ALWAYS be a power of 2
float vReal[samples];
float vImag[samples];

// WiFi variables
const char *SSID = WIFI_SSID;
const char *PASSWORD = WIFI_PASS;
const int PORT = 8080;
WiFiServer server(PORT);

// Functions declaration
void calculateFFT();
float sampleSignalAndGetFreq();
void startWiFi();

// Arduino code
void setup() {
  // put your setup code here, to run once:
  pinMode(analog_input, INPUT);
  Serial.begin(115200);
  delay(1000);
  Serial.println("====   WiFi Init   ====");
  startWiFi();
  Serial.println("==== Configuration ====");
  Serial.print("Analog pin: " + analog_input);
  Serial.print("Samples count: " + samples);
  Serial.println("====   Starting    ====");
}

void loop() {
  // Sample the signal from the AD8232
  float samplingFreq = sampleSignalAndGetFreq();
  // Calculate the FFT of the signal
  calculateFFT(samplingFreq);
}

float sampleSignalAndGetFreq() {
  // Get the current time in milliseconds since startup
  Serial.println("Starting sampling of the input signal");
  unsigned long start = millis();
  // Sample the signal from the AD
  for (size_t i = 0; i < samples; i++)
  {
    vReal[i] = analogRead(analog_input);
    vImag[i] = 0;
  }
  // Calculate how much time have passed
  float elapsedMillis = static_cast<float>(millis() - start);
  Serial.println("Sampling completed after " + String(elapsedMillis) + "ms");
  float samplingFrequency = static_cast<float>(samples) / (elapsedMillis / 1000.0);
  Serial.println("Samplinf frequency is " + String(samplingFrequency) + "Hz");
  // Calculate sampling frequency
  return samplingFrequency;
}

void calculateFFT(double samplingFreq) {
    // Get samples
    ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFreq);
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);	/* Weigh data */
    FFT.compute(FFTDirection::Forward); /* Compute FFT */
    // Compute magnitude
    // The output will overwrite the first half of the integer data array.
    FFT.complexToMagnitude();
}

void startWiFi() {
  // Handle WiFi connection
  Serial.print("Connecting to ");
  Serial.print(SSID);
  
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
}
