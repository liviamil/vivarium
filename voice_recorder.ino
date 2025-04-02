#define MIC_PIN A0
#define BUTTON_PIN 26
#define LED_PIN 24

// WAV file header (44 bytes) stored in program memory
PROGMEM const byte header[44] = {
    0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56,
    0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x01, 0x00, 0xA0, 0x0F, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x01,
    0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00
};

bool isRecording = false;  // To keep track of the recording state
bool lastButtonState = HIGH;  // To store the last button state
unsigned long lastDebounceTime = 0;  // To handle debouncing
unsigned long debounceDelay = 50;    // Debounce time
unsigned long sampleTimer = 0;      // For timing the samples
unsigned long sampleInterval = 250;  // Microseconds between samples (8kHz)
unsigned long totalSamples = 0;     // Count of total samples recorded

void setup() {
    pinMode(MIC_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // Start Serial communication
    Serial.begin(100000);  // Higher baud rate for better performance
    while (!Serial) { ; }  // Wait for serial port to open
}

void loop() {
    bool buttonState = digitalRead(BUTTON_PIN);
    
    // Check if button was pressed (change in state)
    if (buttonState == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
        // Reset debounce time
        lastDebounceTime = millis();
        
        // Toggle the recording state
        isRecording = !isRecording;
        
        if (isRecording) {
            // Start recording
            digitalWrite(LED_PIN, HIGH);  // Turn on LED to indicate recording
            totalSamples = 0;             // Reset sample counter
            sendWavHeader();              // Send WAV header when starting recording
        } else {
            // Stop recording
            digitalWrite(LED_PIN, LOW);   // Turn off LED to indicate stopped recording
            updateWavHeader();            // Update the WAV header with final size
        }
    }
    
    // Save the current button state for the next loop
    lastButtonState = buttonState;
    
    // If recording, send microphone data to Serial
    if (isRecording) {
        unsigned long currentMicros = micros();
        
        // Check if it's time for the next sample (10kHz = 100µs between samples)
        if (currentMicros - sampleTimer >= sampleInterval) {
            sampleTimer = currentMicros;
            
            int micValue = analogRead(MIC_PIN);
            
            // Convert 10-bit ADC value (0-1023) to 16-bit audio sample (-32768 to 32767)
            // Center it around zero and scale for better audio quality
            int16_t scaledValue = (micValue - 512) * 64;
            
            // Send the data to the serial port
            Serial.write(scaledValue & 0xFF);       // Low byte
            Serial.write((scaledValue >> 8) & 0xFF); // High byte
            
            totalSamples++;
        }
    }
}

// Send the WAV header at the start of recording
void sendWavHeader() {
    // Send the header directly from program memory
    for (int i = 0; i < 44; i++) {
        Serial.write(pgm_read_byte(&header[i]));
    }
}

// Update the WAV header with the final size information
void updateWavHeader() {
    // Calculate the data size (2 bytes per sample)
    uint32_t dataSize = totalSamples * 2;
    
    // Calculate the total file size (data size + header size - 8)
    uint32_t fileSize = dataSize + 44 - 8;
    
    // Send the file size (bytes 4-7 in the header)
    Serial.write(fileSize & 0xFF);
    Serial.write((fileSize >> 8) & 0xFF);
    Serial.write((fileSize >> 16) & 0xFF);
    Serial.write((fileSize >> 24) & 0xFF);
    
    // Send the data size (bytes 40-43 in the header)
    Serial.write(dataSize & 0xFF);
    Serial.write((dataSize >> 8) & 0xFF);
    Serial.write((dataSize >> 16) & 0xFF);
    Serial.write((dataSize >> 24) & 0xFF);
}