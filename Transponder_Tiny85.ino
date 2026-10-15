#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_PIN PB1
#define IR_PIN PB0
#define SELECT_PIN PB2
#define DELAY_5S 186000

// Global counter incremented in the Timer0 COMPA ISR
volatile uint16_t bitCount = 0;
volatile uint16_t intervalTime = 400;
volatile bool pinState = false;
volatile unsigned long blinkCount = 0;
volatile int useID = 0;

uint8_t ID[2][4] = {
  {0x04, 0xD2, 0x5B, 0xBB}, // trackmate 1234
  {0x0A, 0xCA, 0xEB, 0x8D}  // trackmate 2762
};

void setup() {
  // Set PB0 (OC0A) and PB1 (OC1A) as outputs
  DDRB |= (1 << IR_PIN) | (1 << LED_PIN);
  // Set pull-up on PB2 input
  PORTB |= (1 << SELECT_PIN);

  // if (PINB & (1 << SELECT_PIN)) {
  //   useID = 0;
  // }
  // else {
  //   useID = 1;
  // }

  // ------------------------------------------------------------------
  // 1. TIMER 0 CONFIGURATION (Square wave on PB0)
  // ------------------------------------------------------------------
  // Reset Timer0 Control Registers
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0  = 0;

  // Set CTC Mode (WGM01 = 1, WGM00 = 0, WGM02 = 0)
  TCCR0A |= (1 << WGM01);

  // Toggle OC0A (PB0) on Compare Match (COM0A0 = 1)
  TCCR0A |= (1 << COM0A0);

  // Set Compare Match Value for 425kHz period at 16.5 MHz
  OCR0A = 18;

  // Start Timer0 with No Prescaling (CS00 = 1)
  TCCR0B |= (1 << CS00);


  // ------------------------------------------------------------------
  // 2. TIMER 1 CONFIGURATION (27us on PB1 OC1A)
  // ------------------------------------------------------------------
  // Reset Timer1 Control Register
  TCCR1 = 0;
  GTCCR = 0;
  TCNT1 = 0;

  // Set Top Value for Timer1 Period (OCR1C) -> 27 uSec at 8.25 MHz
  OCR1C = 222;

  // Set compare A
  OCR1A = 0;

  // Enable CTC on Channel A (OC1A / PB1) and toggle on Compare Match
  //TCCR1 |= (1 << CTC1) | (1 << COM1A0);
  TCCR1 |= (1 << CTC1);

  // Enable Timer0 Compare Match A Interrupt
  TIMSK |= (1 << OCIE1A);

  // Start Timer1 with Prescaler = 2 (CS11 = 1)
  TCCR1 |= (1 << CS11);


  // ------------------------------------------------------------------
  // Global Interrupt Enable
  // ------------------------------------------------------------------
  sei();
}

void loop() {
  static int lastBit = 0;

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
      pinState = ID[useID][0] & (1U << (bitCount - 1));
    }
    else if ((bitCount >= 11) && (bitCount <= 18)) {
      // Byte 2 of 4
      pinState = ID[useID][1] & (1U << (bitCount - 11));
    }
    else if ((bitCount >= 21) && (bitCount <= 28)) {
      // Byte 3 of 4
      pinState = ID[useID][2] & (1U << (bitCount - 21));
    }
    else if ((bitCount >= 31) && (bitCount <= 38)) {
      // Byte 4 of 4
      pinState = ID[useID][3] & (1U << (bitCount - 31));
    }
    else {
      pinState = 1;
    }
    
    // Piggyback on the bitCount loop so it only runs when the ISR does
    if ((blinkCount == 0) || ((blinkCount == 30000) && (useID == 1))) {
      PORTB |= (1 << LED_PIN);
    }
    else if ((blinkCount == 5000) || (blinkCount == 35000)) {
      PORTB &= ~(1 << LED_PIN);
    }
  }

  // Random time interval between frames in 25-27 uSec bit counts
  // 150 is about 4 mSec, 400 is about 10 mSec
  if (bitCount == 0) {
    intervalTime = random(150, 400);

  }

  
}

// --------------------------------------------------------------------
// TIMER0 COMPA INTERRUPT SERVICE ROUTINE
// --------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) {
  if (pinState) {
    // Set it to input to gate PWM for 1
    DDRB &= ~(1 << IR_PIN);
  }
  else {
    // Set as output to let PWM through for 0
    DDRB |= (1 << IR_PIN);
  }

  // Count bits up to random frame time
  if (bitCount == intervalTime) {
    bitCount = 0;
  }
  else {
    bitCount++;
  }

  // Long timer for LED blink pattern
  if (blinkCount == DELAY_5S) {
    blinkCount = 0;
  }
  else {
    blinkCount++;
  }
}