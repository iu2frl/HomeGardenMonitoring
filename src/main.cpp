#include <Arduino.h>
#include <arduinoFFT.h>
#include <ESP8266WiFi.h> 
#include <ArduinoOTA.h>
#include <InfluxDbClient.h>
#include <ArduinoJson.h>
#include <secrets.h> // Secret variables are stored in 'include/secrets.h' which is not included with the repo for security reasons.
#include <check_secrets.h>

// Pinout
const uint8_t analog_input = A0;

// FFT variables
const uint16_t samples = 1024; //This value MUST ALWAYS be a power of 2
float realValues[samples];
float imagValues[samples];

// WiFi variables
const char *SSID = WIFI_SSID;
const char *PASSWORD = WIFI_PASS;

float sampleSignalAndGetFreq() {
  // Get the current time in milliseconds since startup
  Serial.println("Starting sampling of the input signal");
  unsigned long start = millis();
  // Sample the signal from the AD
  for (size_t i = 0; i < samples; i++)
  {
    realValues[i] = analogRead(analog_input);
    imagValues[i] = 0;
    delayMicroseconds(800);
  }
  // Calculate how much time have passed
  float elapsedMillis = static_cast<float>(millis() - start);
  Serial.println("Sampling completed after " + String(elapsedMillis) + "ms");
  float samplingFrequency = static_cast<float>(samples) / (elapsedMillis / 1000.0);
  Serial.println("Sampling frequency is " + String(samplingFrequency) + "Hz");
  // Calculate sampling frequency
  return samplingFrequency;
}

void calculateFFT(double samplingFreq) {
  Serial.println("Calculating FFT of the signal. Sampling frequency: " + String(samplingFreq) + "Hz");
  // Perform the FFT
  unsigned long start = millis();
  ArduinoFFT<float> FFT = ArduinoFFT<float>(realValues, imagValues, samples, samplingFreq);
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.dcRemoval();
  // The output will overwrite the first half of the integer data array.
  FFT.complexToMagnitude();
  Serial.println("FFT calculation completed in " + String(millis() - start) + "ms");
}

void startWiFi() {
  // Handle WiFi connection
  Serial.print("Connecting to ");
  Serial.print(SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retries++;
    if (retries >= 512) {
      Serial.println("WiFi Setup failed, rebooting!");
      delay(1000);
      ESP.restart();
    }
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupOTA() {
  // Setup OTA
  ArduinoOTA.setHostname(SENSOR_NAME);
  #ifdef OTA_PASS
  // Password is enabled only if set in include/secrets.h
  ArduinoOTA.setPassword(OTA_PASS);
  #endif
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

String serializeData(float samplingFreq) {
  Serial.println("Starting JSON creation");
  // Create JSON to be stored in DB
  JsonDocument jsonDocument;
  jsonDocument["device"] = SENSOR_NAME;
  jsonDocument["sampleFreq"] = samplingFreq;
  jsonDocument["samplesCnt"] = samples;
  JsonArray data = jsonDocument.createNestedArray("data");
  for (size_t i = 0; i < (samples/2); i++)
  {
    data.add(realValues[i]);
  }
  String outputDocument;
  serializeJson(jsonDocument, outputDocument);
  Serial.println("Operation completed!");
  return outputDocument;
}

void pushToInflux(float samplingFreq) {
  Serial.println("Starting FFT storage to InfluxDB");
  // Single InfluxDB instance
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
  Point pointDevice("garden_fft_data");
  // Set tags
  pointDevice.addTag("device", SENSOR_NAME);
  // Add data
  pointDevice.addField("json_data", serializeData(samplingFreq));
  // Write data
  if (!client.writePoint(pointDevice)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  } else {
    Serial.println("Operation completed!");
  }
}

// Arduino code
void setup() {
  // put your setup code here, to run once:
  pinMode(analog_input, INPUT);
  Serial.begin(115200);
  delay(1000);
  Serial.println("====   WiFi Init   ====");
  startWiFi();
  Serial.println("====   OTA Init    ====");
  setupOTA();
  Serial.println("==== Configuration ====");
  Serial.println("Analog pin: " + String(analog_input));
  Serial.println("Samples count: " + String(samples));
  Serial.println("====   Starting    ====");
}

void loop() {
  // Sample the signal from the AD8232
  float samplingFreq = sampleSignalAndGetFreq();
  // Calculate the FFT of the signal
  calculateFFT(samplingFreq);
  // Push data to InfluxDB
  pushToInflux(samplingFreq);
  // Wait some time
  for (size_t i = 0; i < 600; i++)
  {
    ArduinoOTA.handle();
    delay(100);
  }
}
