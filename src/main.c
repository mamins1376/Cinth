
/*
 * HEADER FILE(S)
 */

// device header file
#include <pic14/pic16f627a.h>


/*
 * CONFIGURATION WORD
 */
static __code unsigned int __at (_CONFIG) config_word = 
  _HS_OSC &     // high speed osc(20MHz)
  _WDT_OFF &    // watchdog off
  _MCLRE_OFF &  // reset pin as normal I/O
  _LVP_OFF &    // disable low voltage programming
  _CP_OFF;      // disable code protection


/*
 * CONSTANTS
 */


// number of outputs; set to 1 for a monophonic synth.
// must be an unsigned integer, greater than 0 and lesser than 9
// NOTE: currently, polyphony is NOT available due to
// limited cpu. so don't touch this, or you get a
// error says there is no enough memory or a
// program that doesn't work properly.
#define CHANNELS      1

// used to generate unexact delay; experimental
#define DELAY_CONF    130

// INTCON configuration for enabled state of TMR0
#define INTCON_CONFIG 0xA0

// timer configuration for generating INT_FREQ
#define TMR_PRELOAD    0xE2

// interrupt frequency(also sample rate) in Hz
#define INT_FREQ      86207
#define INT_FREQ_HALF 43103

// fake lookup table length
#define FUNCTION_PERIOD 65536
#define FUNCTION_PERIOD_HALF FUNCTION_PERIOD / 2

/*
 * GLOBAL VARIABLES
 */

// the phase changed in every sample
// note id for each channel(C4 = 0, C#4 = 1, ...)

#if CHANNELS == 1 // if monophonic

unsigned long phase, phasediff;
unsigned char note;

#else // if polyphonic

unsigned long phase[CHANNELS], phasediff[CHANNELS];

  #if CHANNELS == 2
    unsigned char note[] = {0,4};
  #elif CHANNELS == 3
    unsigned char note[] = {0,4,11};
  #elif CHANNELS == 4
    unsigned char note[] = {0,4,11,14};
  #elif CHANNELS == 5
    unsigned char note[] = {0,4,11,14,17};
  #elif CHANNELS == 6
    unsigned char note[] = {0,4,11,14,17,21};
  #else // CHANNELS is 7
    unsigned char note[] = {0,4,11,14,17,21,25};
  #endif

#endif

// calculated sample, each bit is one channel's output
unsigned char buffer;

// phase lookup table.
// phasediff = function period * frequency / sample rate
unsigned int phasediff_table[] = {
  0x0c7, 0x0d3, 0x0df, 0x0ed, 0x0fb,
  0x109, 0x119, 0x12a, 0x13c, 0x14e,
  0x162, 0x177, 0x18e, 0x1a5, 0x1be,
  0x1d9, 0x1f5, 0x213, 0x233, 0x254,
  0x277, 0x29d, 0x2c5, 0x2ef, 0x31c
};


/*
 * FUNCTIONS
 */

// apply new phase changes according to lookup table,
// should called on every frequency change.
void update_phases(void) {



#if CHANNELS == 1 // if monophonic

  phase += phasediff;
  if (phase > FUNCTION_PERIOD)
    phase -= FUNCTION_PERIOD;

#else // it is polyphonic with at least two channels

  // some useful macros, to improve performance and write less code
  #define UPDATE_PHASE(x) \
    phase[x] += phasediff[x];\
    if (phase[x] > FUNCTION_PERIOD) \
      phase[x] -= FUNCTION_PERIOD;

  UPDATE_PHASE(0)
  UPDATE_PHASE(1)

  // insert a UPDATE_PHASE for each channel, if it is available
  // unfortunately, c preprocessor doesn't support loops, we must
  // do this manually. sorry for dirty and repeatitive code
  #if CHANNELS > 2
  UPDATE_PHASE(2)
  #endif
  #if CHANNELS > 3
  UPDATE_PHASE(3)
  #endif
  #if CHANNELS > 4
  UPDATE_PHASE(4)
  #endif
  #if CHANNELS > 5
  UPDATE_PHASE(5)
  #endif
  #if CHANNELS > 6
  UPDATE_PHASE(6)
  #endif
  #if CHANNELS > 7
  UPDATE_PHASE(7)
  #endif
  
  // UPDATE_PHASE is not useful anymore
  #undef UPDATE_PHASE

#endif

}

// calculate buffer
void fill_buffer(void) {
  // for each bit: if it's phase is more than
  // lookup table length, set, unset else.
  // originally fakes an lookup table for generating
  // square wave(1 bit)

  // unset all bits
  buffer = 0;

#if CHANNELS == 1 // if monophonic

  if (phase > FUNCTION_PERIOD_HALF)
    buffer = 0x01;

#else // if polyphonic

  // same process as update_phases
  #define FILL_BUFFER(x) \
    if (phase[x] > FUNCTION_PERIOD_HALF) \
      buffer |= 1 << x;

  FILL_BUFFER(0)
  FILL_BUFFER(1)

  #if CHANNELS > 2
  FILL_BUFFER(2)
  #endif
  #if CHANNELS > 3
  FILL_BUFFER(3)
  #endif
  #if CHANNELS > 4
  FILL_BUFFER(4)
  #endif
  #if CHANNELS > 5
  FILL_BUFFER(5)
  #endif
  #if CHANNELS > 6
  FILL_BUFFER(6)
  #endif
  #if CHANNELS > 7
  FILL_BUFFER(7)
  #endif

  #undef FILL_BUFFER

#endif

}

// set new phasediffs' value whenever note changed
void set_phasediffs(void) {

#if CHANNELS == 1 // if monophonic

  phasediff = phasediff_table[note];

#else // if monophonic

  #define SET_PHASEDIFFS(x) \
    phasediff[x] = phasediff_table[note[x]];

  SET_PHASEDIFFS(0)
  SET_PHASEDIFFS(1)

  #if CHANNELS > 2
  SET_PHASEDIFFS(2)
  #endif
  #if CHANNELS > 3
  SET_PHASEDIFFS(3)
  #endif
  #if CHANNELS > 4
  SET_PHASEDIFFS(4)
  #endif
  #if CHANNELS > 5
  SET_PHASEDIFFS(5)
  #endif
  #if CHANNELS > 6
  SET_PHASEDIFFS(6)
  #endif
  #if CHANNELS > 7
  SET_PHASEDIFFS(7)
  #endif

  #undef SET_PHASEDIFFS

#endif

}

// interrupt handler
void isr(void) __interrupt 0 {

  INTCON = 0x00;

  // if skips is zero toggle RA0, else decrease it
  PORTB = buffer;

  update_phases();
  fill_buffer();
  
  TMR0 = TMR_PRELOAD;
  INTCON = INTCON_CONFIG;

}

// wait a little
void some_delay(void) {
  unsigned int a,b;
  for (a=0; a < DELAY_CONF; a++)
    for (b=0; b < DELAY_CONF; b++);
}

// apply system configuration and enable TMR0 interrupts
void setup(void) {

  PORTB = 0x00;

  // set audio outputs, depending on count of channels
  // calculation: 2 ^ CHANNELS - 1
  // shift is better, and then toggle add bits
  TRISB = ((1 << CHANNELS) - 1) ^ 0xFF;

  // pullups off
  OPTION_REG = 0x80;

  // finally enable interrupt
  TMR0 = TMR_PRELOAD;
  INTCON = INTCON_CONFIG;

}

// program start point
void main(void) {

  // if monophonic, play a simple music
# if CHANNELS == 1

  unsigned char n;
  unsigned char song[] = {
    4,4,5,7,7,5,4,2,0,0,2,4,4,2,2,2,4,4,5,7,7,5,4,2,0,0,2,4,2,0,0
  };

  // config system
  setup();

  // loop forever
  while(1) {
    for (n=0; n < 31; n++) {
      note = song[n];
      set_phasediffs();
      some_delay();
    }
  }

#else // polyphonic, play a chord. default is set above

  // update phasediffs
  set_phasediffs();

  // config system
  setup();

#endif
}
