/*
 * LG Smart TV IR Remote — Arduino Pro-Mini 3V3
 * =============================================
 *
 * Board:    Arduino Pro Mini 3V3 (ATmega328P, 8 MHz)
 * Library:  IRremote (https://github.com/Arduino-IRremote/Arduino-IRremote)
 *
 * Wiring summary:
 *   IR LED anode  -> Pin 3 (OC2B / Timer2 PWM for 38 kHz carrier)
 *   IR LED cathode -> 100 Ohm resistor -> GND
 *
 *   Power button  -> Pin D4 -> GND  (internal pull-up, active LOW)
 *   HDMI-1 button -> Pin D5 -> GND  (internal pull-up, active LOW)
 *   HDMI-2 button -> Pin D6 -> GND  (internal pull-up, active LOW)
 *   HDMI-3 button -> Pin D7 -> GND  (internal pull-up, active LOW)
 *
 *   Power supply: 3.7 V Li-ion cell -> RAW pin (no regulator needed at 3.3 V)
 *
 * Power saving:
 *   - SLEEP_MODE_PWR_DOWN between presses
 *   - Wake on pin-change interrupt (PCINT2 / PORTD)
 *   - ADC, BOD, and unused peripherals disabled
 */

#include <IRremote.hpp>
#include <avr/sleep.h>
#include <avr/power.h>

/* ── Pin assignments ─────────────────────────────────────────────── */
static const uint8_t PIN_IR_LED     = 3;   // Timer2 OC2B output

static const uint8_t PIN_BTN_POWER  = 4;
static const uint8_t PIN_BTN_HDMI1  = 5;
static const uint8_t PIN_BTN_HDMI2  = 6;
static const uint8_t PIN_BTN_HDMI3  = 7;

static const uint8_t BUTTON_PINS[]  = {
  PIN_BTN_POWER, PIN_BTN_HDMI1, PIN_BTN_HDMI2, PIN_BTN_HDMI3
};
static const uint8_t BUTTON_COUNT   = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

/* ── LG NEC protocol constants ───────────────────────────────────── */
static const uint16_t LG_ADDRESS    = 0x04;  // device address (inverted byte handled by lib)
static const uint8_t  CMD_POWER     = 0x08;
static const uint8_t  CMD_HDMI1     = 0xCE;
static const uint8_t  CMD_HDMI2     = 0xCC;
static const uint8_t  CMD_HDMI3     = 0xE9;

/* Command lookup — index matches BUTTON_PINS order */
static const uint8_t COMMANDS[]     = {
  CMD_POWER, CMD_HDMI1, CMD_HDMI2, CMD_HDMI3
};

/* ── Timing constants ────────────────────────────────────────────── */
static const uint8_t  DEBOUNCE_MS   = 50;
static const uint8_t  NEC_REPEATS   = 2;    // total transmissions per press

/* ── Forward declarations ────────────────────────────────────────── */
static void enterSleep();
static void disableUnusedPeripherals();

/* ── Pin-change ISR (empty — only purpose is to wake the MCU) ──── */
ISR(PCINT2_vect) {
  /* intentionally empty */
}

/* ================================================================ */
void setup() {
  /* Configure button pins with internal pull-ups */
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  /* Initialise IR sender on pin 3 */
  IrSender.begin(PIN_IR_LED);

  /* Disable peripherals we don't need */
  disableUnusedPeripherals();

  /* Enable pin-change interrupts on PORTD (PCINT20..23 = D4..D7) */
  PCICR  |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT20) | (1 << PCINT21) | (1 << PCINT22) | (1 << PCINT23);
}

/* ================================================================ */
void loop() {
  /* Scan buttons (active LOW) */
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    if (digitalRead(BUTTON_PINS[i]) == LOW) {
      delay(DEBOUNCE_MS);

      /* Confirm the press is still held after debounce */
      if (digitalRead(BUTTON_PINS[i]) == LOW) {
        IrSender.sendNEC(LG_ADDRESS, COMMANDS[i], NEC_REPEATS);

        /* Wait for button release before sleeping */
        while (digitalRead(BUTTON_PINS[i]) == LOW) {
          /* spin until released */
        }
        delay(DEBOUNCE_MS);  // debounce release
      }
    }
  }

  enterSleep();
}

/* ── Power-management helpers ────────────────────────────────────── */

static void disableUnusedPeripherals() {
  /* Disable ADC */
  ADCSRA &= ~(1 << ADEN);
  power_adc_disable();

  /* Disable peripherals not needed */
  power_spi_disable();
  power_twi_disable();

  /* Disable digital input buffers on analog pins (saves ~1 uA each) */
  DIDR0 = 0x3F;  // ADC0..ADC5
  DIDR1 = 0x03;  // AIN0, AIN1
}

static void enterSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();

  /* Disable BOD during sleep (ATmega328P fuse-controlled, software assist) */
  sleep_bod_disable();

  sei();
  sleep_cpu();

  /* ── Execution resumes here after wake-up ── */
  sleep_disable();
}
