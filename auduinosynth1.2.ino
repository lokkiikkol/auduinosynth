// AUDUINO - MIDI Upgrade v1.0
//
// MIDI programming by David Benn http://www.notesandvolts.com
// MIDI and "synth" additions by Simon Iten
// Tutorial - http://www.notesandvolts.com/2015/05/auduino-synth-midi.html
// 
// ** Requires Arduino MIDI Library v4.2 or later **
//
// Based on the Auduino Synthesizer v5 by Peter Knight http://tinker.it
// ************************************************************************
#pragma GCC optimize ("-O3")
#include <avr/io.h>
#include <avr/interrupt.h>
#include <MIDI.h> // Requires Arduino MIDI Library v4.2 or later
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define UNIQUE_ID 0x5111

// *** MIDI Parameters ***********************************************************
//#define BENDRANGE 3 // Change to set Number of Semitones for Pitch Bend (1 to 24)
#define MIDICHANNEL 6 // Can be a value from 1 to 16

// CC-Values for MIDI-Control

#define GRAIN_FREQ_CONTROL   57
#define GRAIN_DECAY_CONTROL  58
#define GRAIN2_FREQ_CONTROL  59
#define GRAIN2_DECAY_CONTROL 60
#define PRESET               62
#define SAVING               63
#define BITREDUCTION         65
#define VOLUME               66
#define ATTACK               67
#define ATTACK2              68
#define MINIMUM              71
#define MINIMUM2             72
#define DECAYTIME            73
#define SUSTAIN              74
#define DECAYTIME2           75
#define SUSTAIN2             76
#define ATTACK3              77
#define MINIMUM3             78
#define DECAYTIME3           79
#define SUSTAIN3             80
#define RELEASE              81
#define RELEASE2             82
#define RELEASE3             83
#define LFOSPEED             84
#define WAVEFORM             85
#define WAVEFORM2            86
#define LFODEPTH             87
#define LFOWAVEFORM          88
#define PULSEWIDTH           90
#define PULSEWIDTH2          91
#define LFOTRIG              92
#define LFOATTACK            93
#define LFOMIN               94
#define LFOMAX               95
#define LFOOFFSET            96
#define BIT1                 97
#define BIT2                 98
#define BIT3                 99
#define BIT4                100
#define BIT5                101
#define BIT6                102
#define BIT7                103
#define BIT8                104
#define DETUNE2             105
#define LFODESTINATION      106
#define REATTACK            113
#define DETUNE              114
#define TRANSPOSE           115
#define ATTACKTIME4         116
#define MINIMUM4            117
#define MAXIMUM4            118
#define DECAYTIME4          119
#define SUSTAIN4            70
#define RELEASE4            112
#define AMPLITUDE2          111
#define AMPLITUDE3          110
#define WAVEFORM3           109
#define WAVEFORM4            69
#define TRANSPOSE3           54
#define LFOWAVEFORM2         55
#define FILTERWAVE           56
#define AFTERTOUCHIMPACT    122
#define AFTERTOUCHIMPACT2    61
#define TRANSPOSESYNTH       56
#define PULSEWT3             107
#define PULSEWT4             108
#define AFTERTOUCH3AND4       89
//#define GLIDETIME           126
// *******************************************************************************


#define BUFFER 8 //Size of keyboard buffer

#define PWM_PIN       3
#define PWM2_PIN      11
#define PWM_VALUE     OCR2B
#define PWM2_VALUE    OCR2A
#define PWM_INTERRUPT TIMER2_OVF_vect

MIDI_CREATE_DEFAULT_INSTANCE();

uint16_t syncPhaseAcc;

uint16_t syncPhaseInc;

uint16_t sync2PhaseAcc;
uint16_t sync2PhaseInc;

uint16_t grainPhaseAcc;
uint16_t grainPhaseInc;
uint16_t grainAmp;
uint8_t grainDecay;
uint16_t grain2PhaseAcc;
uint16_t grain2PhaseInc;
uint16_t grain2Amp;
uint8_t grain2Decay;

 int amplitude = 255;
 int attacktime = 500;
 int envelope;
 int attacktime2 = 500;
 int envelope2;
 int attacktime3 = 100;
 int envelope3 = 255;
 int minimum;
 int minimum2;
 int minimum3;
 int decaytime;
 int sustain;
 int decaytime2;
 int sustain2;
 int decaytime3;
 int sustain3;
 int releasetime;
 int lastenvelope;
 int releasetime2;
 int lastenvelope2;
 int releasetime3;
 int lastenvelope3;

 int attacktime4;
 byte minimum4;
 byte maximum4;
 int decaytime4;
 byte sustain4;
 int releasetime4;
 byte lastenvelope4;
 
 int counter;
 int counter2;
 int counter3;
 int counter4;
 
 int lfodepth;
// int lfodepth2;
 int attackcounter;
 int lfooffset;
 //int lfooffset2;
 byte lfodestination = 0b00000000;
 byte transpose3 = 63;
// int lfoamp = 1;
 int lfoenvelope;
 int lfospeed = 255;
 int lfosig;
 //int lfosig2 = 0;
 int counterlfo;
// int counterlfo2;
 int lfoattack;
 byte minlfo;
 byte maxlfo;
 int amplitude2;
//precalculated sine wave
PROGMEM const int sinetable[512] = {
0x200,0x206,0x20d,0x213,0x219,0x21f,0x226,0x22c,0x232,0x238,0x23f,0x245,0x24b,0x251,0x258,0x25e,
0x264,0x26a,0x270,0x276,0x27c,0x282,0x289,0x28f,0x295,0x29b,0x2a1,0x2a7,0x2ac,0x2b2,0x2b8,0x2be,
0x2c4,0x2ca,0x2cf,0x2d5,0x2db,0x2e1,0x2e6,0x2ec,0x2f1,0x2f7,0x2fc,0x302,0x307,0x30d,0x312,0x317,
0x31c,0x322,0x327,0x32c,0x331,0x336,0x33b,0x340,0x345,0x34a,0x34e,0x353,0x358,0x35c,0x361,0x366,
0x36a,0x36e,0x373,0x377,0x37b,0x380,0x384,0x388,0x38c,0x390,0x394,0x397,0x39b,0x39f,0x3a3,0x3a6,
0x3aa,0x3ad,0x3b1,0x3b4,0x3b7,0x3ba,0x3bd,0x3c1,0x3c4,0x3c6,0x3c9,0x3cc,0x3cf,0x3d1,0x3d4,0x3d7,
0x3d9,0x3db,0x3de,0x3e0,0x3e2,0x3e4,0x3e6,0x3e8,0x3ea,0x3ec,0x3ed,0x3ef,0x3f1,0x3f2,0x3f4,0x3f5,
0x3f6,0x3f7,0x3f8,0x3f9,0x3fa,0x3fb,0x3fc,0x3fd,0x3fe,0x3fe,0x3ff,0x3ff,0x3ff,0x400,0x400,0x400,
0x400,0x400,0x400,0x400,0x3ff,0x3ff,0x3ff,0x3fe,0x3fe,0x3fd,0x3fc,0x3fb,0x3fa,0x3f9,0x3f8,0x3f7,
0x3f6,0x3f5,0x3f4,0x3f2,0x3f1,0x3ef,0x3ed,0x3ec,0x3ea,0x3e8,0x3e6,0x3e4,0x3e2,0x3e0,0x3de,0x3db,
0x3d9,0x3d7,0x3d4,0x3d1,0x3cf,0x3cc,0x3c9,0x3c6,0x3c4,0x3c1,0x3bd,0x3ba,0x3b7,0x3b4,0x3b1,0x3ad,
0x3aa,0x3a6,0x3a3,0x39f,0x39b,0x397,0x394,0x390,0x38c,0x388,0x384,0x380,0x37b,0x377,0x373,0x36e,
0x36a,0x366,0x361,0x35c,0x358,0x353,0x34e,0x34a,0x345,0x340,0x33b,0x336,0x331,0x32c,0x327,0x322,
0x31c,0x317,0x312,0x30d,0x307,0x302,0x2fc,0x2f7,0x2f1,0x2ec,0x2e6,0x2e1,0x2db,0x2d5,0x2cf,0x2ca,
0x2c4,0x2be,0x2b8,0x2b2,0x2ac,0x2a7,0x2a1,0x29b,0x295,0x28f,0x289,0x282,0x27c,0x276,0x270,0x26a,
0x264,0x25e,0x258,0x251,0x24b,0x245,0x23f,0x238,0x232,0x22c,0x226,0x21f,0x219,0x213,0x20d,0x206,
0x200,0x1fa,0x1f3,0x1ed,0x1e7,0x1e1,0x1da,0x1d4,0x1ce,0x1c8,0x1c1,0x1bb,0x1b5,0x1af,0x1a8,0x1a2,
0x19c,0x196,0x190,0x18a,0x184,0x17e,0x177,0x171,0x16b,0x165,0x15f,0x159,0x154,0x14e,0x148,0x142,
0x13c,0x136,0x131,0x12b,0x125,0x11f,0x11a,0x114,0x10f,0x109,0x104,0xfe,0xf9,0xf3,0xee,0xe9,
0xe4,0xde,0xd9,0xd4,0xcf,0xca,0xc5,0xc0,0xbb,0xb6,0xb2,0xad,0xa8,0xa4,0x9f,0x9a,
0x96,0x92,0x8d,0x89,0x85,0x80,0x7c,0x78,0x74,0x70,0x6c,0x69,0x65,0x61,0x5d,0x5a,
0x56,0x53,0x4f,0x4c,0x49,0x46,0x43,0x3f,0x3c,0x3a,0x37,0x34,0x31,0x2f,0x2c,0x29,
0x27,0x25,0x22,0x20,0x1e,0x1c,0x1a,0x18,0x16,0x14,0x13,0x11,0xf,0xe,0xc,0xb,
0xa,0x9,0x8,0x7,0x6,0x5,0x4,0x3,0x2,0x2,0x1,0x1,0x1,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x2,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,
0xa,0xb,0xc,0xe,0xf,0x11,0x13,0x14,0x16,0x18,0x1a,0x1c,0x1e,0x20,0x22,0x25,
0x27,0x29,0x2c,0x2f,0x31,0x34,0x37,0x3a,0x3c,0x3f,0x43,0x46,0x49,0x4c,0x4f,0x53,
0x56,0x5a,0x5d,0x61,0x65,0x69,0x6c,0x70,0x74,0x78,0x7c,0x80,0x85,0x89,0x8d,0x92,
0x96,0x9a,0x9f,0xa4,0xa8,0xad,0xb2,0xb6,0xbb,0xc0,0xc5,0xca,0xcf,0xd4,0xd9,0xde,
0xe4,0xe9,0xee,0xf3,0xf9,0xfe,0x104,0x109,0x10f,0x114,0x11a,0x11f,0x125,0x12b,0x131,0x136,
0x13c,0x142,0x148,0x14e,0x154,0x159,0x15f,0x165,0x16b,0x171,0x177,0x17e,0x184,0x18a,0x190,0x196,
0x19c,0x1a2,0x1a8,0x1af,0x1b5,0x1bb,0x1c1,0x1c8,0x1ce,0x1d4,0x1da,0x1e1,0x1e7,0x1ed,0x1f3,0x1fa,
};

const unsigned int LUTsize = 1<<8; // Look Up Table size: has to be power of 2 so that the modulo LUTsize
                                   // can be done by picking bits from the phase avoiding arithmetic

                               
const int8_t multitable[1664] PROGMEM = {
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,

  0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,   
  0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
  0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
  0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
  0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
  0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
  0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
  0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
  0xff,0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf1,0xef,0xed,0xeb,0xe9,0xe7,0xe5,0xe3,0xe1,
  0xdf,0xdd,0xdb,0xd9,0xd7,0xd5,0xd3,0xd1,0xcf,0xcd,0xcb,0xc9,0xc7,0xc5,0xc3,0xc1,
  0xbf,0xbd,0xbb,0xb9,0xb7,0xb5,0xb3,0xb1,0xaf,0xad,0xab,0xa9,0xa7,0xa5,0xa3,0xa1,
  0x9f,0x9d,0x9b,0x99,0x97,0x95,0x93,0x91,0x8f,0x8d,0x8b,0x89,0x87,0x85,0x83,0x81,
  0x7f,0x7d,0x7b,0x79,0x77,0x75,0x73,0x71,0x6f,0x6d,0x6b,0x69,0x67,0x65,0x63,0x61,
  0x5f,0x5d,0x5b,0x59,0x57,0x55,0x53,0x51,0x4f,0x4d,0x4b,0x49,0x47,0x45,0x43,0x41,
  0x3f,0x3d,0x3b,0x39,0x37,0x35,0x33,0x31,0x2f,0x2d,0x2b,0x29,0x27,0x25,0x23,0x21,
  0x1f,0x1d,0x1b,0x19,0x17,0x15,0x13,0x11,0x0f,0x0d,0x0b,0x09,0x07,0x05,0x03,0x01,
  
151,160,137,91,90,15,                 // Hash lookup table as defined by Ken Perlin.  This is a randomly
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,   
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    
     0,0,0,0,0,0,0,1,1,2,3,3,4,5,6,7,8,9,10,12,13,15,16,18,19,21,23,
  25,26,28,30,33,35,37,39,41,44,46,49,51,54,56,59,61,64,67,70,72,
  75,78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124,
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,
  176,179,182,184,187,190,193,195,198,200,203,205,208,210,213,215,
  217,219,221,224,226,228,229,231,233,235,236,238,239,241,242,244,
  245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
  255,254,254,254,254,254,253,253,252,251,251,250,249,248,247,246,
  245,244,242,241,239,238,236,235,233,231,229,228,226,224,221,219,
  217,215,213,210,208,205,203,200,198,195,193,190,187,184,182,179,
  176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,
  127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,
  78,75,72,70,67,64,61,59,56,54,51,49,46,44,41,39,
  37,35,33,30,28,26,25,23,21,19,18,16,15,13,12,10,
  9,8,7,6,5,4,3,3,2,1,1,0,0,0,0,
  
0x00, 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,

  
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,

0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,

0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
};

const int timerPrescale=1<<9;
struct oscillator
{
    uint32_t phase;
    int32_t phase_increment;
    uint16_t amplitude;
} o1;

struct oscillator2
{
    uint32_t phase;
    int32_t phase_increment;
    uint16_t amplitude;
} o2;



const int fractionalbits = 16; // 16 bits fractional phase
// compute a phase increment from a frequency
unsigned long phaseinc(float frequency_in_Hz)
{
   return LUTsize *(1l<<fractionalbits)* frequency_in_Hz/(F_CPU/timerPrescale);
}

const int predivide = 8;
unsigned long phaseinc_from_fractional_frequency(unsigned long frequency_in_Hz_times_256)
{
    return (1l<<(fractionalbits-predivide))* ((LUTsize*(timerPrescale/(1<<predivide))*frequency_in_Hz_times_256)/(F_CPU/(1<<predivide)));
}

#define MIDITOPH
#ifdef MIDITOPH
#define mtoph(x) ( phaseinc(8.1757989156* pow(2.0, x /12.0) ))
const unsigned long midinotetophaseinc[128]=
{
  mtoph(0), mtoph(1), mtoph(2),mtoph(3), mtoph(4), mtoph(5), mtoph(6), mtoph(7),
  mtoph(8),mtoph(9), mtoph(10), mtoph(11), mtoph(12), mtoph(13), mtoph(14), mtoph(15),
   mtoph(16), mtoph(17), mtoph(18), mtoph(19), mtoph(20), mtoph(21), mtoph(22), mtoph(23),
  mtoph(24), mtoph(25), mtoph(26), mtoph(27), mtoph(28), mtoph(29), mtoph(30), mtoph(31),
   mtoph(32), mtoph(33), mtoph(34), mtoph(35), mtoph(36), mtoph(37), mtoph(38), mtoph(39),
  mtoph(40), mtoph(41), mtoph(42), mtoph(43), mtoph(44), mtoph(45), mtoph(46), mtoph(47),
   mtoph(48), mtoph(49), mtoph(50), mtoph(51), mtoph(52), mtoph(53), mtoph(54), mtoph(55),
  mtoph(56), mtoph(57), mtoph(58), mtoph(59), mtoph(60), mtoph(61), mtoph(62), mtoph(63),
  mtoph(64), mtoph(65), mtoph(66), mtoph(67), mtoph(68), mtoph(69), mtoph(70), mtoph(71),
  mtoph(72), mtoph(73), mtoph(74), mtoph(75), mtoph(76), mtoph(77), mtoph(78), mtoph(79),
   mtoph(80), mtoph(81), mtoph(82), mtoph(83), mtoph(84), mtoph(85), mtoph(86), mtoph(87),
  mtoph(88), mtoph(89), mtoph(90), mtoph(91), mtoph(92), mtoph(93), mtoph(94), mtoph(95),
   mtoph(96),mtoph(97), mtoph(98), mtoph(99), mtoph(100), mtoph(101), mtoph(102), mtoph(103),
  mtoph(104),mtoph(105), mtoph(106), mtoph(107), mtoph(108), mtoph(109), mtoph(110), mtoph(111),
   mtoph(112),mtoph(113), mtoph(114), mtoph(115), mtoph(116), mtoph(117), mtoph(118), mtoph(119),
  mtoph(120), mtoph(121), mtoph(122), mtoph(123), mtoph(124), mtoph(125), mtoph(126), mtoph(127)
 };
#undef mtoph
#endif
// Smooth logarithmic mapping
//
const uint16_t antilogTable[] = {
  64830, 64132, 63441, 62757, 62081, 61413, 60751, 60097, 59449, 58809, 58176, 57549, 56929, 56316, 55709, 55109,
  54515, 53928, 53347, 52773, 52204, 51642, 51085, 50535, 49991, 49452, 48920, 48393, 47871, 47356, 46846, 46341,
  45842, 45348, 44859, 44376, 43898, 43425, 42958, 42495, 42037, 41584, 41136, 40693, 40255, 39821, 39392, 38968,
  38548, 38133, 37722, 37316, 36914, 36516, 36123, 35734, 35349, 34968, 34591, 34219, 33850, 33486, 33125, 32768
};
uint16_t mapPhaseInc(uint16_t input) {
  return (antilogTable[input & 0x3f]) >> (input >> 6);
}

// Stepped chromatic mapping
//
const uint16_t midiTable[] = {
  17, 18, 19, 20, 22, 23, 24, 26, 27, 29, 31, 32, 34, 36, 38, 41, 43, 46, 48, 51, 54, 58, 61, 65, 69, 73,
  77, 82, 86, 92, 97, 103, 109, 115, 122, 129, 137, 145, 154, 163, 173, 183, 194, 206, 218, 231,
  244, 259, 274, 291, 308, 326, 346, 366, 388, 411, 435, 461, 489, 518, 549, 581, 616, 652, 691,
  732, 776, 822, 871, 923, 978, 1036, 1097, 1163, 1232, 1305, 1383, 1465, 1552, 1644, 1742,
  1845, 1955, 2071, 2195, 2325, 2463, 2610, 2765, 2930, 3104, 3288, 3484, 3691, 3910, 4143,
  4389, 4650, 4927, 5220, 5530, 5859, 6207, 6577, 6968, 7382, 7821, 8286, 8779, 9301, 9854,
  10440, 11060, 11718, 12415, 13153, 13935, 14764, 15642, 16572, 17557, 18601, 19708, 20879,
  22121, 23436, 24830, 26306
};
uint16_t mapMidi(uint8_t input) {
  return (midiTable[input]);
}

// Stepped Pentatonic mapping
//
/*const uint16_t pentatonicTable[54] = {
  17, 19, 22, 26, 29, 32, 38, 43, 51, 58, 65, 77, 86, 103, 115, 129, 154, 173, 206, 231, 259, 308, 346,
  411, 461, 518, 616, 691, 822, 923, 1036, 1232, 1383, 1644, 1845, 2071, 2463, 2765, 3288,
  3691, 4143, 4927, 5530, 6577, 7382, 8286, 9854, 11060, 13153, 14764, 16572, 19708, 22121, 26306
};

uint16_t mapPentatonic(uint16_t input) {
  uint8_t value = (1023 - input) / (1024 / 53);
  return (pentatonicTable[value]);
} */

//Global Varibles used by MIDI Upgrade
byte note = 0;
byte buff[BUFFER];
byte buffersize = 0;
//const float bendfactor = ((BENDRANGE * 100.0) / 8192.0);
float detune = 0;
float pitchfinal = 1;
bool noteOn = false;
int oldpot = 0;
byte transpose2 = 63; 
int grainfreq = 0;
byte graindecay = 0;
int grainfreq2 = 0;
byte graindecay2 = 0;
byte presetnr = 0;
byte mode = 0;
byte waveform = 0;
byte waveform2 = 0;
byte waveform3 = 0;
byte waveform4 = 0;
byte lfotrig = 1;

byte pulsewidth = 255;
byte pulsewidth2 = 255;
byte lfowaveform = 0;

byte bitreduction = 0b11111111;

byte aftertouch = 0;
byte aftertouchimpact = 63;
byte aftertouch2impact = 63;
byte aftertouch3 = 0;
byte aftertouch4 = 0;
byte detune2 = 0;
byte amplitudewave = 0;
byte amplitudewave2 = 0;
byte transposesynth = 48;
byte pulsewidthwt3 = 0;
byte pulsewidthwt4 = 0;
uint16_t pulsewidth3 = 0;
uint16_t pulsewidth4 = 0;
// allocate space for 52 settings
static uint8_t settings[51];

void audioOn() {

  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
}


// Save patch data to EEPROM
bool savePatch( uint16_t id, uint8_t slot, void* data, uint16_t size ) {
  uint16_t n;
  uint16_t addr = slot * 53;
  if(slot < 20) {
  	eeprom_write_byte((uint8_t*)(addr++), id >> 8);
  	eeprom_write_byte((uint8_t*)(addr++), id);
    for(n = 0; n < size; n++) {
  		eeprom_write_byte((uint8_t*)(addr++), ((uint8_t*)data)[n]);
    }
    return true;
  }
  return false;
}

// Load patch data from EEPROM
bool loadPatch( uint16_t id, uint8_t slot, void* data, uint16_t size ) {
  uint16_t n;
  uint16_t addr = slot * 53;
  if(slot < 20) {
    if(eeprom_read_byte((uint8_t*)(addr++)) == (uint8_t)(id >> 8)) {
      if(eeprom_read_byte((uint8_t*)(addr++)) == (uint8_t)id) {
        for(n = 0; n < size; n++) {
          ((uint8_t*)data)[n] = eeprom_read_byte((uint8_t*)(addr++));
        }
        return true;
      }
    }
  }
  return false;
}

void ProgramChangeMidi(byte channel, byte number) {
  loadPatch(UNIQUE_ID, number , settings, sizeof(settings));
  load_settings();
}

void ControlChangeMidi(byte channel, byte number, byte value) {
  switch (number) {
    case GRAIN_FREQ_CONTROL:
          settings[0] = value;
          grainfreq = (128 - value) * 8;
          break;
    case GRAIN_DECAY_CONTROL:
          settings[1] = value;
          graindecay = value;
          break;
    case GRAIN2_FREQ_CONTROL:
          settings[2] = value;
          grainfreq2 = (128 - value) * 8;
          break;
    case GRAIN2_DECAY_CONTROL:
          settings[3] = value;
          graindecay2 = value;
          break;
    case PRESET:
          presetnr = value;
          break;   
    case SAVING:
         if (value == 127) {
           settings[23] = (waveform << 4) | waveform2;
           settings[24] = (lfowaveform << 4) | lfotrig;
           settings[25] = (waveform3 << 4) | waveform4;
          // settings[44] = lfodestination;
         //  settings[45] = bitreduction;
          savePatch(UNIQUE_ID, presetnr, settings, sizeof(settings));
         } else if (value == 80) SysexSend();
        break; 
    case VOLUME:
          settings[5] = value;
          amplitude = value*value >> 1;
          break;
    case ATTACK:
          settings[6] = value;
          attacktime = value*value;
          break;
    case ATTACK2:
         settings[7] = value;
         attacktime2 = value*value;
         break;
    case MINIMUM:
         settings[8] = value;
          minimum = (128 - value) * 8;
          break;
    case MINIMUM2:
          settings[9] = value;
          minimum2 = (128 - value) * 8;
          break;
    case DECAYTIME:
         settings[10] = value;
         decaytime = value*value;
         break;
    case SUSTAIN:
         settings[11] = value;
         sustain = (128 - value) * 8;
         break; 
    case DECAYTIME2:
          settings[12] = value;
          decaytime2 = value*value;
          break;
    case SUSTAIN2:
          settings[13] = value;
          sustain2 = (128 - value) * 8;
          break;
    case MINIMUM3:
          settings[14] = value;
          minimum3 = value*value >> 1 ;
          break;
    case ATTACK3:
          settings[15] = value;
          attacktime3 = value * 8;
           break;
    case DECAYTIME3:
          settings[16] = value;
          decaytime3 = value * 8;
          break;
    case SUSTAIN3:
          settings[17] = value;
          sustain3 = value*value >> 1;
          break;
    case RELEASE:
          settings[18] = value;
          releasetime = value*value;
          break;
    case RELEASE2:
          settings[19] = value;
          releasetime2 = value*value;
          break;
    case RELEASE3:
          settings[20] = value;
          releasetime3 = value*value;
          break;
    case LFOSPEED:
          settings[21] = value;
          lfospeed = (128 - value)*(128 - value);
          break;
    case LFODEPTH:
          settings[22] = value;
          lfodepth = (128 - value)*(128 - value);
          break;
    case WAVEFORM:
         waveform = value >> 5;
         break;
    case WAVEFORM2:
         waveform2 = value >> 5;
         break;
    case LFOWAVEFORM:
         lfowaveform = value >> 4;
         break;
    case PULSEWIDTH:
         settings[26] = value;
         pulsewidth = value * 2;
         break;
    case PULSEWIDTH2:
          settings[27] = value;
          pulsewidth2 = value * 2;
          break;
    case LFOTRIG:
         lfotrig = value >> 5;
         break;    
    case LFOATTACK:
         settings[28] = value;
         lfoattack = value*value;
         break;
    case LFOMIN:
         settings[29] = value;
         minlfo = value;
         break;
    case LFOMAX:
         settings[30] = value;
         maxlfo = value;
         break;
    case LFOOFFSET:
         settings[31] = value;
         lfooffset = value * 4;
         break;
    case DETUNE2:
         settings[32] = value;
         detune2 = value;
         break;
    case LFODESTINATION:
          settings[44] = value;
         lfodestination = value;
         break;
  
    case REATTACK:
        if (value == 127) {
          counter = 0;
          counter2 = 0;
          counter3 = 0;
          counter4 = 0;
        }
        break;
    case DETUNE:
         settings[33] = value;
         detune = (value - 63) * 0.2;
         break;
    case TRANSPOSE:
         settings[34] = value;
         transpose2 = value;
         break; 
    case ATTACKTIME4:
         settings[35] = value;
         attacktime4 = value * 4;
        break; 
    case MINIMUM4:
         settings[36] = value;
         minimum4 = value;
         break;
    case MAXIMUM4:
         settings[37] = value;
         maximum4 = value;
         break;
    case DECAYTIME4:
          settings[38] = value;
          decaytime4 = value * 4;
          break;
    case SUSTAIN4:
         settings[39] = value;
         sustain4 = value;
         break;
    case RELEASE4:
         settings[40] = value;
         releasetime4 = value * 8;
         break;
    case PULSEWT3:
         settings[48] = value;
         pulsewidthwt3 = value;
         break;
    case PULSEWT4:
         settings[49] = value;
         pulsewidthwt4 = value;
         break;
    case AMPLITUDE2:
         settings[41] = value;
         amplitudewave = value;
         break;
    case AMPLITUDE3:
         settings[42] = value;
         amplitudewave2 = value;
         break;
    case WAVEFORM3:
         waveform3 = value >> 4;
         break;  
    case WAVEFORM4:
         waveform4 = value >> 4;
         break; 
   case TRANSPOSE3:
         settings[43] = value;
         transpose3 = value;   
         break;
   case AFTERTOUCHIMPACT:
        settings[46] = value;
         aftertouchimpact = value;
         break;
   case AFTERTOUCHIMPACT2:
          settings[47] = value;
          aftertouch2impact = value;
          break;       
   case TRANSPOSESYNTH:
          if (value < 97) {
          settings[45] = value;
          transposesynth = value;
          }
          break;
  case AFTERTOUCH3AND4:
        settings[50] = value;
        aftertouch3 = value >> 2;
        aftertouch4 = value & 3;
        break;       
  case BITREDUCTION:
        settings[4] = value;
        bitreduction = value + 128;
         break;

  }
}

// NoteOnMidi function is called when a Note On event is detected
// Put note in key buffer and set note variable
void NoteOnMidi(byte channel, byte pitch, byte velocity) {
  //counter5 = 0;
  if (buffersize < BUFFER) {
  note = pitch + transposesynth - 48;
    buff[buffersize] = note;
    buffersize++;
  }
  
  noteOn = true;
  if (buffersize < 2) {
//    portamento = false;
  counter = 0;
  counter2 = 0;
  counter3 = 0;
  counter4 = 0;
  if (lfotrig == 1 || lfotrig == 3) {
    counterlfo = 0;
    attackcounter = 0;

  }
 } else {
 // portamento = true;
 // counter5 = 0;
 }
}

// NoteOffMidi function is called when a Note Off event is detected
// If there are other notes in the key buffer (more than one key held)
// re-trigger old note. If key buffer is empty - set note to zero (off).
void NoteOffMidi(byte channel, byte pitch, byte velocity) {
 // counter5 = 0;
  if (buffersize > 1) {
    for (int ctr = 0; ctr < buffersize; ctr++) {
      if (buff[ctr] == pitch + transposesynth - 48)  {

        ShiftLeft(ctr + 1);
        break;
      }
    }
    note = buff[buffersize - 1];
   /* counter = 0;
    counter2 = 0;
    counter3 = 0;
    counter4 = 0; */
  //  counter5 = 0;
  //  portamento = true;
  }
  else {
    noteOn = false;
//    portamento = false;
    if (lfotrig == 2) {
      counterlfo = 0;
      attackcounter = 0;

    }
    counter = 0;
    counter2 = 0;
    counter3 = 0;
    counter4 = 0;
    lastenvelope = envelope;
    lastenvelope2 = envelope2;
    lastenvelope3 = envelope3;
    lastenvelope4 = amplitude2;
   // note = 0;
    buff[buffersize - 1] = 0;
    buffersize--;
  }
//  pot = false;
  
}

void AfterTouchMidi(byte channel, byte pressure) {
  aftertouch = pressure;
  
}

// ShiftLeft closes gap in key buffer when note is removed.
void ShiftLeft(int index) {
  int ctr = (index - 1);
  for (ctr; ctr < buffersize - 1; ctr++) {
    buff[ctr] = buff[ctr + 1];
  }
  buff[ctr + 1] = 0;
  buffersize--;
}

// Pitchbend function is called when Pitchbend data is received.
// This function coverts the Pitchbend number into cents (100 cents = 1 semitone)
// In the main loop, the frequency of the note plus the pitchbend is calculated
// using this formula: Final Frequency = note * 2^(cents/1200)
/*void Pitchbend (byte channel, int bend) {
  //cents = bend*bendfactor;
  pitchfinal = (pow(2, ((bend*bendfactor) / 1200)));

} */

void  load_settings() {
  cli();
  grainfreq = (128 - settings[0]) * 8;
  graindecay = settings[1];
  grainfreq2 = (128 - settings[2]) * 8;
  graindecay2 = settings[3];
  amplitude = settings[5]*settings[5] >> 1;
  attacktime = settings[6]*settings[6];
  attacktime2 = settings[7]*settings[7];
  minimum = (128 - settings[8]) * 8;
  minimum2 = (128 - settings[9]) * 8;
  decaytime = settings[10]*settings[10];
  sustain = (128 - settings[11]) * 8;
  decaytime2 = settings[12]*settings[12];
  sustain2 = (128 - settings[13]) * 8;
  minimum3 = settings[14]*settings[14] >> 1;
  attacktime3 = settings[15] * 8;
  decaytime3 = settings[16] * 8;
  sustain3 = settings[17]*settings[17] >> 1;
  releasetime = settings[18]*settings[18];
  releasetime2 = settings[19]*settings[19];
  releasetime3 = settings[20]*settings[20];
  lfospeed = (128 - settings[21])*(128 - settings[21]);
  lfodepth = (128 - settings[22])*(128 - settings[22]);
  waveform = settings[23] >> 4;
  waveform2 = settings[23] & 15;
  lfowaveform = settings[24] >> 4;
  lfotrig = settings[24] & 15;
  waveform3 = settings[25] >> 4;
  waveform4 = settings[25] & 15;
  pulsewidth = settings[26] * 2;
  pulsewidth2 = settings[27] * 2;
  lfoattack = settings[28]*settings[28];
  minlfo = settings[29];
  maxlfo = settings[30];
  lfooffset = settings[31] * 4;
  detune2 = settings[32];
  detune = (settings[33] - 63) * 0.2;
  transpose2 = settings[34];
  attacktime4 = settings[35] * 4;
  minimum4 = settings[36];
  maximum4 = settings[37];
  decaytime4 = settings[38] * 4;
  sustain4 = settings[39];
  releasetime4 = settings[40] * 8;
  amplitudewave = settings[41];
  amplitudewave2 = settings[42];
  transpose3 = settings[43];
  lfodestination = settings[44];
  transposesynth = settings[45];
  aftertouchimpact = settings[46];
  aftertouch2impact = settings[47];
  bitreduction = settings[4] + 128;
  pulsewidthwt3 = settings[48];
  pulsewidthwt4 = settings[49];
  aftertouch3 = settings[50] >> 2;
  aftertouch4 = settings[50] & 3; 
  sei();
}

void  adsr() {
    if(noteOn == true) {
    if (counter <= attacktime){
      counter = counter + 1;   
      envelope =  map(counter, 0, attacktime, minimum, grainfreq);
    } else {
      if (counter <= (decaytime + attacktime)){
        counter = counter + 1;
        envelope = map(counter, attacktime, (decaytime + attacktime), grainfreq, sustain);
      }
    }
  } else {
    if(counter < releasetime){
    counter = counter + 1;
    envelope = map(counter, 0, releasetime, lastenvelope, 0);
    }
  }
}


void  adsr2() {
     if(noteOn == true) {
    if (counter2 <= attacktime2){
      counter2 = counter2 + 1;
      envelope2 =  map(counter2, 0, attacktime2, minimum2, grainfreq2);
    } else {
    if (counter2 <= (decaytime2 + attacktime2)){
        counter2 = counter2 + 1;
        envelope2 = map(counter2, attacktime2, (decaytime2 + attacktime2), grainfreq2, sustain2);
     }
   }
     } else {
       if(counter2 < releasetime2) {
    counter2 = counter2 + 1;
    envelope2 = map(counter2, 0, releasetime2, lastenvelope2, 0);
     }
     }
}

void adsr3() {
    if(noteOn == true) {
    if (counter3 <= attacktime3){
      counter3 = counter3 + 1;   
      envelope3 =  map(counter3, 0, attacktime3, minimum3, amplitude);
    } else {
    if (counter3 <= (decaytime3 + attacktime3)){
        counter3 = counter3 + 1;
        envelope3 = map(counter3, attacktime3, (decaytime3 + attacktime3), amplitude, sustain3);
     }
   }
  } else {
    if(counter3 < releasetime3){
    counter3 = counter3 + 1;
    envelope3 = map(counter3, 0, releasetime3, lastenvelope3, 0);
    } else {
      envelope3 = 0;
    }  
   } 
}

void adsr4() {
   if(noteOn == true) {
    if (counter4 <= attacktime4){
      counter4 = counter4 + 1;   
      amplitude2 =  map(counter4, 0, attacktime4, minimum4, maximum4);
    } else {
      if (counter4 <= (decaytime4 + attacktime4)){
        counter4 = counter4 + 1;
        amplitude2 = map(counter4, attacktime4, (decaytime4 + attacktime4), maximum4, sustain4);
      }
    }
  } else {
    if(counter4 < releasetime4){
    counter4 = counter4 + 1;
    amplitude2 = map(counter4, 0, releasetime4, lastenvelope4, 0);
    } else {
    amplitude2 = 0;
  }
   }
}

void lfo(){
 
     if (attackcounter <= lfoattack) {
       attackcounter = attackcounter + 1;
       lfoenvelope = map(attackcounter, 0, lfoattack, minlfo, maxlfo);
       }
    
    if (counterlfo <= lfospeed) {
  counterlfo = counterlfo + 1;
switch (lfowaveform) {
  case 0:
    lfosig = pgm_read_word_near(sinetable + map(counterlfo, 0, lfospeed, 0, 511)) - 512 ;
   break; 
  case 1:
    if (counterlfo <= lfospeed / 2) {
      lfosig = map(counterlfo, 0, lfospeed / 2, -512, 512);
      } else {
      lfosig = map(counterlfo, lfospeed / 2, lfospeed, 512, -512);
      }
  break;
  case 2:
    lfosig = map(counterlfo, 0, lfospeed, -512, 512);
  break;
  case 3:
     lfosig = map(counterlfo, 0, lfospeed, 512, -512);
  break;
  case 4:
     if (counterlfo <= lfospeed / 2) {
     lfosig = -512;
     } else {
     lfosig = 512;
     }
  break;
  case 5:
     if (counterlfo <= lfospeed / 2) {
     lfosig = 512;
     } else {
     lfosig = -512;
     }
}
if (noteOn == true && lfotrig == 2) lfoenvelope = 0;
if (noteOn == false && lfotrig == 3) lfoenvelope = 0;
if (lfotrig == 0) lfoenvelope = minlfo;
lfosig = ((lfosig + lfooffset) * lfoenvelope >> 3)/ lfodepth;

 
} else {
  counterlfo = 0;
}
}

void SysexSend() {
  cli();
  Serial.write(0xF0);

  for (byte i=0; i<51; i++){
      Serial.write(settings[i]);
    }
    Serial.write(0xF7);
    sei();
}
void setup() {
  o1.phase = 0;
   o1.phase_increment = 0;
   o2.phase = 0;
   o2.phase_increment = 0;
  pinMode(PWM_PIN, OUTPUT);
  pinMode(PWM2_PIN, OUTPUT);
  audioOn();
  MIDI.begin(MIDICHANNEL);
  MIDI.turnThruOff();
  MIDI.setHandleNoteOn(NoteOnMidi);
  MIDI.setHandleNoteOff(NoteOffMidi);
//  MIDI.setHandlePitchBend(Pitchbend);
  MIDI.setHandleControlChange(ControlChangeMidi);
  MIDI.setHandleProgramChange(ProgramChangeMidi);
   MIDI.setHandleAfterTouchChannel(AfterTouchMidi);
 loadPatch(UNIQUE_ID, 0, settings, sizeof(settings));
  load_settings();
 // lfospeed2 = 20000;
}

void loop() { // *** MAIN LOOP ***

   MIDI.read();
  
  adsr();
  adsr2();
  adsr3();
  adsr4();
  lfo();
  o1.amplitude = amplitudewave; 
  o2.amplitude = amplitudewave2;


  o1.phase_increment = midinotetophaseinc[note];
  o2.phase_increment = (midinotetophaseinc[note + (transpose3 - 63) + bitRead(lfodestination,6) * lfosig] + bitRead(lfodestination,0)*lfosig + detune2*4);
  syncPhaseInc = mapMidi(note);
  sync2PhaseInc = (mapMidi(note + (transpose2 - 63) + bitRead(lfodestination,1)*lfosig) + detune);

  grainPhaseInc  = mapPhaseInc(envelope + ((aftertouch*(aftertouchimpact - 63)) >> 5) + bitRead(lfodestination,2)*lfosig)  >> 1;
  grainDecay     = graindecay +  bitRead(lfodestination,4)*lfosig ;
  grain2PhaseInc = mapPhaseInc(envelope2 + ((aftertouch*(aftertouch2impact - 63)) >> 5) + bitRead(lfodestination,3)*lfosig) >> 1;
  grain2Decay    = (graindecay2 + bitRead(lfodestination,5)*lfosig) << 1;
  pulsewidth3 = pulsewidthwt3 + aftertouch*(aftertouch3 -1) + 128  + (waveform3 << 8); 
   pulsewidth4 = pulsewidthwt4 + aftertouch*(aftertouch4 -1) + 128 + (waveform4 << 8);
}
  uint16_t outputvalue;  
ISR(PWM_INTERRUPT)
{

  uint8_t value;
  uint16_t output;
 
 
  outputvalue = (o1.amplitude  * pgm_read_byte(multitable+pulsewidth3+((o1.phase>>16)%LUTsize)))>>8;
  outputvalue += (o2.amplitude * pgm_read_byte(multitable+pulsewidth4+((o2.phase>>16)%LUTsize)))>>8;
 
  outputvalue =  (outputvalue * amplitude2) >> 8;
 

  PWM2_VALUE = outputvalue & bitreduction;

 o1.phase += (uint32_t)o1.phase_increment;
 o2.phase += (uint32_t)o2.phase_increment;

 
  syncPhaseAcc += syncPhaseInc;
  sync2PhaseAcc += sync2PhaseInc;
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
  }
 
 if (sync2PhaseAcc < sync2PhaseInc) { 
   
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
  }
  
  
  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;
  
  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  switch (waveform) {
    case 0:
      if (grainPhaseAcc & 0x8000) value = ~value;
    break;
    case 1:
    if (value <= pulsewidth) {
       value = 255;
     } else {
       value = 0;
     }
    break;  
  }
  
  // Multiply by current grain amplitude to get sample
  
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  switch (waveform2) {
    case 0:
      if (grain2PhaseAcc & 0x8000) value = ~value;
    break;
    case 1:
         if (value <= pulsewidth2) {
       value = 255;
     } else {
       value = 0;
     }
    break;
  }

  
  output += value * (grain2Amp >> 8);
  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;

  // Scale output to the available range, volume control with ability to distort and bitreduction!
output >>= 9 & bitreduction;

 output *= envelope3;
 output >>= 9;
 // if (output > 255) output = 255;

 //samplecount = samplecount + 1;
 // if (samplecount >= sampleredux) {
 //   samplecount = 0;
  // Output to PWM (this is faster than using analogWrite)
  PWM_VALUE = output;
 // }
}


