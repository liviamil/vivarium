#include <SD.h>
#include <SPI.h>

#define MIC_PIN A0
#define BUTTON_PIN 26
#define LED_PIN 24
#define SD_CS_PIN 32

File audioFile;
bool isRecording = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long sampleTimer = 0;
unsigned long sampleInterval = 250;
unsigned long totalSamples = 0;

void setup() {
    pinMode(MIC_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(100000);
    
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    Serial.println("SD Card ready.");
}

void loop() {
    bool buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
        lastDebounceTime = millis();
        isRecording = !isRecording;
        
        if (isRecording) {
            digitalWrite(LED_PIN, HIGH);
            totalSamples = 0;
            audioFile = SD.open("recording.wav", FILE_WRITE);
            if (audioFile) {
                writeWavHeader();
            } else {
                Serial.println("Failed to open file for writing");
                isRecording = false;
            }
        } else {
            digitalWrite(LED_PIN, LOW);
            if (audioFile) {
                updateWavHeader();
                audioFile.close();
            }
        }
    }
    lastButtonState = buttonState;
    
    if (isRecording) {
        unsigned long currentMicros = micros();
        if (currentMicros - sampleTimer >= sampleInterval) {
            sampleTimer = currentMicros;
            int micValue = analogRead(MIC_PIN);
            int16_t scaledValue = (micValue - 512) * 64;
            audioFile.write(scaledValue & 0xFF);
            audioFile.write((scaledValue >> 8) & 0xFF);
            totalSamples++;
        }
    }
}

void writeWavHeader() {
    byte header[44] = {
        0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
        0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0xA0, 0x0F, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00,
        0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00
    };
    audioFile.write(header, 44);
}

void updateWavHeader() {
    uint32_t dataSize = totalSamples * 2;
    uint32_t fileSize = dataSize + 44 - 8;
    audioFile.seek(4);
    audioFile.write(fileSize & 0xFF);
    audioFile.write((fileSize >> 8) & 0xFF);
    audioFile.write((fileSize >> 16) & 0xFF);
    audioFile.write((fileSize >> 24) & 0xFF);
    audioFile.seek(40);
    audioFile.write(dataSize & 0xFF);
    audioFile.write((dataSize >> 8) & 0xFF);
    audioFile.write((dataSize >> 16) & 0xFF);
    audioFile.write((dataSize >> 24) & 0xFF);
}
