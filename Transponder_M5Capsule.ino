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
#define TRACKMATE 0
#define ILAP      1

hw_timer_t *timer = NULL;
volatile int bitCount = 0;
volatile int intervalTime = 400;
volatile bool pinState = false;

struct IDcode {
    uint8_t type;
    uint8_t code[6];
    int IDnum;
};

// Array of transponder IDs
IDcode ID[5] = {
    {TRACKMATE, {0x05, 0xA6, 0x56, 0x99, 0x0, 0x0}, 1446},  // trackmate 1446
    {TRACKMATE, {0x03, 0x86, 0xD8, 0x5D, 0x0, 0x0}, 902},  // trackmate 0902
    {TRACKMATE, {0x04, 0xD2, 0x5B, 0xBB, 0x0, 0x0}, 1234},  // trackmate 1234
    {ILAP, {0x0D, 0x4A, 0xAB, 0xAE, 0xEB, 0x71}, 2545114},  // i-Lap 2545114
    {ILAP, {0x0A, 0xBE, 0x8C, 0x7C, 0xCD, 0xC4}, 5738332}  // i-Lap 5738332
};

uint8_t bit_widths[2] = {
    (uint8_t)270, // type 0, trackmate 27 uSec
    (uint8_t)256 // type 1, i-Lap 25.6 uSec
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
        //timerAlarm(timer, 270, true, 0);
        timerAlarm(timer, bit_widths[ID[0].type], true, 0);
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
    static int type_buf = ID[0].type;
    static unsigned long frameCount = 0;
    static unsigned long printDelay = DELAY_5S; // ~5 sec using 27 uSec ticks

    // Set the output state based on bit location
    if (bitCount != lastBit) {
        lastBit = bitCount;
        if (ID[useID_buf].type == TRACKMATE) {
            // Set pinState based on the bit position
            if ((bitCount == 0) || (bitCount == 10) || (bitCount == 20) || (bitCount == 30)) {
                // Start bits
                pinState = 0;
            }
            else if ((bitCount == 9) || (bitCount == 19) || (bitCount == 29)) {  // last stop bit is covered by else
                // Stop bits
                pinState = 1;
            }
            else if ((bitCount >= 1) && (bitCount <= 8)) {
                // Byte 1 of 4
                pinState = ID[useID_buf].code[0] & (1U << (bitCount - 1));
            }
            else if ((bitCount >= 11) && (bitCount <= 18)) {
                // Byte 2 of 4
                pinState = ID[useID_buf].code[1] & (1U << (bitCount - 11));
            }
            else if ((bitCount >= 21) && (bitCount <= 28)) {
                // Byte 3 of 4
                pinState = ID[useID_buf].code[2] & (1U << (bitCount - 21));
            }
            else if ((bitCount >= 31) && (bitCount <= 38)) {
                // Byte 4 of 4
                pinState = ID[useID_buf].code[3] & (1U << (bitCount - 31));
            }
            else {
                pinState = 1;
            }
        }
        else if (ID[useID_buf].type == ILAP) {
             // Set pinState based on the bit position
            if ((bitCount == 0) || (bitCount == 10) || (bitCount == 20) || (bitCount == 30) || (bitCount == 40) || (bitCount == 50)) {
                // Start bits
                pinState = 0;
            }
            else if ((bitCount == 9) || (bitCount == 19) || (bitCount == 29) || (bitCount == 39) || (bitCount == 49)) { // lst stop bit is covered by else
                // Stop bits
                pinState = 1;
            }
            else if ((bitCount >= 1) && (bitCount <= 8)) {
                // Byte 1 of 6
                pinState = ID[useID_buf].code[0] & (1U << (bitCount - 1));
            }
            else if ((bitCount >= 11) && (bitCount <= 18)) {
                // Byte 2 of 6
                pinState = ID[useID_buf].code[1] & (1U << (bitCount - 11));
            }
            else if ((bitCount >= 21) && (bitCount <= 28)) {
                // Byte 3 of 6
                pinState = ID[useID_buf].code[2] & (1U << (bitCount - 21));
            }
            else if ((bitCount >= 31) && (bitCount <= 38)) {
                // Byte 4 of 6
                pinState = ID[useID_buf].code[3] & (1U << (bitCount - 31));
            }
            else if ((bitCount >= 41) && (bitCount <= 48)) {
                // Byte 5 of 6
                pinState = ID[useID_buf].code[4] & (1U << (bitCount - 41));
            }
            else if ((bitCount >= 51) && (bitCount <= 58)) {
                // Byte 6 of 6
                pinState = ID[useID_buf].code[5] & (1U << (bitCount - 51));
            }
            else {
                pinState = 1;
            }
        }
        else { // handle out of bounds
            pinState = 1;
        }
        
        printDelay--;
    }

    // Random time interval between frames in 25-27 uSec bit counts
    // 55 is minimum for consecutive frames, 400 is about 10 mSec
    if (bitCount == 0) {
        intervalTime = random(55, 400);
        frameCount++;       // keep track of frame statistics
        useID_buf = useID;  // only update an ID change when a new frame starts

        // change the bit width based on system type, updating at bit 0 is safe since this is always a 1
        if (ID[useID].type != type_buf) {
            timerAlarm(timer, bit_widths[ID[useID].type], true, 0);
        }
        type_buf = ID[useID].type;
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
        
        if (ID[useID].type == TRACKMATE) {
            Serial.printf("Switching to Trackmate ID%d\r\n", ID[useID].IDnum);
        }
        else if (ID[useID].type == ILAP) {
            Serial.printf("Switching to I-Lap ID%d\r\n", ID[useID].IDnum);
        }
        else {
            Serial.printf("Switching to ID%d\r\n", useID);
        }
    }
}