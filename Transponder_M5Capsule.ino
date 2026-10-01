/**
 * @file Transponder_M5Capsule.ino
 * @author Hindenbugbite
 * @brief M5Capsule Trackmate Clone
 * @version 0.1
 * @date 2026-10-01
 *
 *
 * @Hardwares: M5Capsule
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 */

#include "M5Capsule.h"

#define TIMER_PIN 1
#define IR_PIN    4
#define DUTY     40 // 40 out of 64 ~62%
#define DELAY_5S 185200 // ~5 sec using 27 uSec ticks

hw_timer_t *timer = NULL;
volatile int bitCount = 0;
volatile int intervalTime = 400;
volatile bool pinState = false;


// Array of transponder IDs
uint8_t ID[5][4] = {
    {0x05, 0xA6, 0x56, 0x99},  // 1446
    {0x0D, 0xEA, 0x56, 0x78},  // 3562
    {0x03, 0x86, 0xD8, 0x5D},  // 0902
    {0x03, 0x48, 0xBD, 0x2D},  // 084D
    {0x04, 0xD2, 0x5B, 0xBB},  // 1234
};

// Timer ISR that should run every 24 uSec
// IRAM is about 200kB
void IRAM_ATTR onTimer() {
    //pinState = !pinState;
    //digitalWrite(TIMER_PIN, pinState);

    // Output is inverted, 1 = no PWM, 0 = PWM
    if (pinState) {
        GPIO.out_w1tc = (1UL << 1);
        ledcWrite(IR_PIN, 0);
    } else {
        GPIO.out_w1ts = (1UL << 1);
        ledcWrite(IR_PIN, DUTY);
    }

    // Increment counter
    if (bitCount == intervalTime) {
        bitCount = 0;
    } else {
        bitCount++;
    }
}

void setup() {
    auto cfg = M5.config();
    M5Capsule.begin(cfg);

    M5Capsule.Speaker.tone(4000, 50);
    Serial.begin(115200);
    delay(2000);

    pinMode(TIMER_PIN, OUTPUT);
    digitalWrite(TIMER_PIN, true);
    pinMode(IR_PIN, OUTPUT);
    //digitalWrite(IR_PIN, true);

    // Set timer to 10MHz with ISR call every 27 uSec
    timer = timerBegin(10000000);
    if (timer != NULL) {
        timerAttachInterrupt(timer, &onTimer);
        timerAlarm(timer, 270, true, 0);
        Serial.println("Timer ISR registered!");
        M5Capsule.Speaker.tone(4800, 80);
    }

    // Setup PWM to IR LED, 450kHz, 8-bit resolution
    //ledcSetClockSource(ledc_clk_cfg_t LEDC_APB_CLK);
    bool status = ledcAttach(IR_PIN, 450000, 6);
    if (status) {
        ledcWrite(IR_PIN, DUTY); 
        Serial.println("PWM setup successful!");
        M5Capsule.Speaker.tone(5600, 50);
    } else {
        Serial.println("PWM setup failed!");
        M5Capsule.Speaker.tone(3500, 150);
    }
}

// Arduino on ESP32 uses FreeRTOS and the loop() runs between 100-300kHz
void loop() {
    static int lastBit = 0;
    static int useID = 0;
    static int useID_buf = 0;
    static unsigned long frameCount = 0;
    static unsigned long printDelay = DELAY_5S; // ~5 sec using 27 uSec ticks

    // Set the output state based on bit location
    if (bitCount != lastBit) {
        lastBit = bitCount;
        // Set pinState based on the bit position
        if ((bitCount == 0) || (bitCount == 10) || (bitCount == 20) || (bitCount == 30)) {
            pinState = 0;
        }
        else if ((bitCount == 9) || (bitCount == 19) || (bitCount == 29) || (bitCount == 39)) {
            pinState = 1;
        }
        else if ((bitCount >= 1) && (bitCount <= 8)) {
            pinState = ID[useID_buf][0] & (1U << (bitCount - 1));
        }
        else if ((bitCount >= 11) && (bitCount <= 18)) {
            pinState = ID[useID_buf][1] & (1U << (bitCount - 11));
        }
        else if ((bitCount >= 21) && (bitCount <= 28)) {
            pinState = ID[useID_buf][2] & (1U << (bitCount - 21));
        }
        else if ((bitCount >= 31) && (bitCount <= 38)) {
            pinState = ID[useID_buf][3] & (1U << (bitCount - 31));
        }
        else {
            pinState = 1;
        }
        printDelay--;
    }

    // Random time interval between frames in 24 uSec bit counts
    // 40 is minimum for consecutive frames, 400 is about 10 mSec
    if (bitCount == 0) {
        intervalTime = random(40, 400);
        frameCount++;       // keep track of frame statistics
        useID_buf = useID;  // only update an ID change when a new frame starts
    }

    // Report status to serial port at ~5 second intervals
    if (printDelay == 0) {
        printDelay = DELAY_5S;
        Serial.printf("Loop reports %lu frames transmitted\r\n", frameCount);
    }

    // Read any button presses
    M5Capsule.update();
    if (M5Capsule.BtnA.wasPressed()) {
        
        if (useID == 4) {
            useID = 0;
            M5Capsule.Speaker.tone(4000, 80);
        } else {
            useID++;
            M5Capsule.Speaker.tone(3500, 80);
        }
        Serial.printf("Switching to transponder ID%d\r\n", useID);
    }
}