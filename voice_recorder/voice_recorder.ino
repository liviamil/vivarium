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
unsigned long sampleInterval = 125; // 125 microseconds = 8000 Hz (standard audio rate)
unsigned long totalSamples = 0;

void setup() {
    pinMode(MIC_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    Serial.println("SD Card ready.");
}

void loop() {
    if (!isRecording && (millis() % 1000 == 0)) {
        int micValue = analogRead(MIC_PIN);
        Serial.print("Mic reading: ");
        Serial.println(micValue);
    }

    bool buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
        lastDebounceTime = millis();
        isRecording = !isRecording;
        
        if (isRecording) {
            digitalWrite(LED_PIN, HIGH);
            totalSamples = 0;
            if (SD.mkdir("AUDIO")) {
                Serial.println("Created AUDIO directory");
                } else {
                Serial.println("Failed to create directory or already exists");
                }
            audioFile = SD.open("A.WAV", FILE_WRITE);
            // audioFile = SD.open("/AUDIO/recording.wav", FILE_WRITE);
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
            audioFile.write((uint8_t)(scaledValue & 0xFF));
            audioFile.write((uint8_t)((scaledValue >> 8) & 0xFF));
            totalSamples++;
            if (totalSamples % 100 == 0) {
                audioFile.flush();
            }
            if (totalSamples % 1000 == 0) {
                Serial.print("Samples recorded: ");
                Serial.println(totalSamples);
            }
        }
    }
}

void writeWavHeader() {
    // Create a properly formatted WAV header
    uint8_t header[44];
    
    // "RIFF" chunk descriptor
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    
    // Chunk size (to be filled later)
    header[4] = 0;
    header[5] = 0;
    header[6] = 0;
    header[7] = 0;
    
    // "WAVE" format
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    
    // "fmt " sub-chunk
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    
    // Sub-chunk size (16 for PCM)
    header[16] = 16;
    header[17] = 0;
    header[18] = 0;
    header[19] = 0;
    
    // Audio format (1 for PCM)
    header[20] = 1;
    header[21] = 0;
    
    // Number of channels (1)
    header[22] = 1;
    header[23] = 0;
    
    // Sample rate (8000 Hz - more standard than 1000 Hz)
    header[24] = 0x40;  // 8000 in little-endian
    header[25] = 0x1F;
    header[26] = 0;
    header[27] = 0;
    
    // Byte rate (Sample Rate * BitsPerSample * Channels / 8)
    header[28] = 0x80;  // 16000 in little-endian
    header[29] = 0x3E;
    header[30] = 0;
    header[31] = 0;
    
    // Block align (BitsPerSample * Channels / 8)
    header[32] = 2;
    header[33] = 0;
    
    // Bits per sample (16)
    header[34] = 16;
    header[35] = 0;
    
    // "data" sub-chunk
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    
    // Data size (to be filled later)
    header[40] = 0;
    header[41] = 0;
    header[42] = 0;
    header[43] = 0;
    
    // Write the entire header at once
    audioFile.write(header, 44);
    
    // Also update the sample interval to match the 8000 Hz rate
    sampleInterval = 125; // 125 microseconds = 8000 Hz
}

void updateWavHeader() {
    uint32_t dataSize = totalSamples * 2;
    uint32_t fileSize = dataSize + 44 - 8;
    audioFile.seek(4);
    audioFile.write((uint8_t)(fileSize & 0xFF));
    audioFile.write((uint8_t)((fileSize >> 8) & 0xFF));
    audioFile.write((uint8_t)((fileSize >> 16) & 0xFF));
    audioFile.write((uint8_t)((fileSize >> 24) & 0xFF));
    audioFile.seek(40);
    audioFile.write((uint8_t)(dataSize & 0xFF));
    audioFile.write((uint8_t)((dataSize >> 8) & 0xFF));
    audioFile.write((uint8_t)((dataSize >> 16) & 0xFF));
    audioFile.write((uint8_t)((dataSize >> 24) & 0xFF));
}