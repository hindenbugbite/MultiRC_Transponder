#include <avr/io.h>
#include <avr/interrupt.h>

#define TIMER_PIN PB0
#define IR_PIN PB4

// Global counter incremented in the Timer0 COMPA ISR
volatile uint16_t bitCount = 0;
volatile uint16_t intervalTime = 400;
volatile bool pinState = false;

uint8_t ID[4] = {0x04, 0xD2, 0x5B, 0xBB};

void setup() {
  // Set PB0 (OC0A) and PB4 (OC1B) as outputs
  DDRB |= (1 << PB0) | (1 << PB4);

  // ------------------------------------------------------------------
  // 1. TIMER 0 CONFIGURATION (Square wave on PB0 + ISR)
  // ------------------------------------------------------------------
  // Reset Timer0 Control Registers
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0  = 0;

  // Set CTC Mode (WGM01 = 1, WGM00 = 0, WGM02 = 0)
  TCCR0A |= (1 << WGM01);

  // Toggle OC0A (PB0) on Compare Match (COM0A0 = 1)
  TCCR0A |= (1 << COM0A0);

  // Set Compare Match Value for ~13.5 us half-period (27 us full period at 16.5 MHz)
  OCR0A = 222;

  // Enable Timer0 Compare Match A Interrupt
  TIMSK |= (1 << OCIE0A);

  // Start Timer0 with No Prescaling (CS00 = 1)
  TCCR0B |= (1 << CS00);


  // ------------------------------------------------------------------
  // 2. TIMER 1 CONFIGURATION (420 kHz PWM on PB4)
  // ------------------------------------------------------------------
  // Reset Timer1 Control Register
  TCCR1 = 0;
  GTCCR = 0;
  TCNT1 = 0;

  // Set Top Value for Timer1 Period (OCR1C) -> f = 16MHz / (1 * (38 + 1)) ~ 423 kHz
  OCR1C = 38;

  // Set Duty Cycle (~50% duty cycle: 9 / 38)
  OCR1B = 19;

  // Enable PWM on Channel B (OC1B / PB4) and clear on Compare Match
  GTCCR |= (1 << PWM1B) | (1 << COM1B1);

  // Start Timer1 with Prescaler = 1 (CS10 = 1)
  TCCR1 |= (1 << CS10);


  // ------------------------------------------------------------------
  // Global Interrupt Enable
  // ------------------------------------------------------------------
  sei();
}

void loop() {
  static int lastBit = 0;
  static unsigned long frameCount = 0;

  // Set the output state based on bit location
  if (bitCount != lastBit) {
    lastBit = bitCount;
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
      pinState = ID[0] & (1U << (bitCount - 1));
    }
    else if ((bitCount >= 11) && (bitCount <= 18)) {
      // Byte 2 of 4
      pinState = ID[1] & (1U << (bitCount - 11));
    }
    else if ((bitCount >= 21) && (bitCount <= 28)) {
      // Byte 3 of 4
      pinState = ID[2] & (1U << (bitCount - 21));
    }
    else if ((bitCount >= 31) && (bitCount <= 38)) {
      // Byte 4 of 4
      pinState = ID[3] & (1U << (bitCount - 31));
    }
    else {
      pinState = 1;
    }
        
  }

  // Random time interval between frames in 25-27 uSec bit counts
  // 55 is minimum for consecutive frames, 400 is about 10 mSec
  if (bitCount == 0) {
    intervalTime = random(55, 400);
    frameCount++;       // keep track of frame statistics

  }
}

// --------------------------------------------------------------------
// TIMER0 COMPA INTERRUPT SERVICE ROUTINE
// --------------------------------------------------------------------
ISR(TIMER0_COMPA_vect) {
  if (pinState) {
    DDRB &= ~(1 << IR_PIN);
  }
  else {
    DDRB |= (1 << IR_PIN);
  }

  if (bitCount == intervalTime) {
    bitCount = 0;
  }
  else {
    bitCount++;
  }
}