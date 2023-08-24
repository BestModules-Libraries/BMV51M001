/***************************************************************************
File:       		BM_MIDIDefine.h
Author:           BESTMODULE
Description:      MIDI protocol define
https://www.midi.org/specifications/midi1-specifications/m1-v4-2-1-midi-1-0-detailed-specification-96-1-4
History：		  -
	V1.0.1	 -- initial version；2022-11-11；Arduino IDE : v1.8.13

****************************************************************************/
#ifndef _BM_MIDI_DEFINE_H
#define _BM_MIDI_DEFINE_H


/**************************************************************************************
MIDI Message：Divided into Channel Message and System Message
Channel Message：Divided into Channel Voice Message and Channel Mode Message
**************************************************************************************/
//#include <Arduino.h>

#define     USE_RUNNING_STATUS      (true)
#define     NOT_USE_RUNNING_STATUS  (false)
#define     MIDI_PITCHBEND_MIN      (-8192)
#define     MIDI_PITCHBEND_MAX      (8191)

#define     MIDI_CHANNEL_OMNI       (0)
#define     MIDI_CHANNEL_OFF        (17) // and over

#define     SYS_EX_MAXSIZE          (128)


// -----------------------------------------------------------------------------
// Aliasing

using NoteOffCallback              = void (*)(uint8_t channel, uint8_t note, uint8_t velocity);
using NoteOnCallback               = void (*)(uint8_t channel, uint8_t note, uint8_t velocity);
using AfterTouchPolyCallback       = void (*)(uint8_t channel, uint8_t note, uint8_t velocity);
using ControlChangeCallback        = void (*)(uint8_t channel, uint8_t, uint8_t);
using ProgramChangeCallback        = void (*)(uint8_t channel, uint8_t);
using AfterTouchChannelCallback    = void (*)(uint8_t channel, uint8_t);
using PitchBendCallback            = void (*)(uint8_t channel, int16_t);
using SystemExclusiveCallback      = void (*)(uint8_t * array, uint16_t size);
using TimeCodeQuarterFrameCallback = void (*)(uint8_t data);
using SongPositionCallback         = void (*)(uint16_t beats);
using SongSelectCallback           = void (*)(uint8_t songnumber);
using TuneRequestCallback          = void (*)(void);
using ClockCallback                = void (*)(void);
using StartCallback                = void (*)(void);
using TickCallback                 = void (*)(void);
using ContinueCallback             = void (*)(void);
using StopCallback                 = void (*)(void);
using ActiveSensingCallback        = void (*)(void);
using SystemResetCallback          = void (*)(void);



/*! Enumeration of MIDI types */
enum MidiType: uint8_t
{
    InvalidType           = 0x00,    // For notifying errors
    NotChannelType        = 0x01,    // not channel message
    NoteOff               = 0x80,    // Channel Message - Note Off
    NoteOn                = 0x90,    // Channel Message - Note On
    AfterTouchPoly        = 0xA0,    // Channel Message - Polyphonic AfterTouch
    ControlChange         = 0xB0,    // Channel Message - Control Change / Channel Mode
    ProgramChange         = 0xC0,    // Channel Message - Program Change
    AfterTouchChannel     = 0xD0,    // Channel Message - Channel (monophonic) AfterTouch
    PitchBend             = 0xE0,    // Channel Message - Pitch Bend
    SystemExclusive       = 0xF0,    // System Exclusive
	SystemExclusiveStart  = SystemExclusive,   // System Exclusive Start
    TimeCodeQuarterFrame  = 0xF1,    // System Common - MIDI Time Code Quarter Frame
    SongPosition          = 0xF2,    // System Common - Song Position Pointer
    SongSelect            = 0xF3,    // System Common - Song Select
    Undefined_F4          = 0xF4,
    Undefined_F5          = 0xF5,
    TuneRequest           = 0xF6,    // System Common - Tune Request
	SystemExclusiveEnd    = 0xF7,    // System Exclusive End
    Clock                 = 0xF8,    // System Real Time - Timing Clock
    Undefined_F9          = 0xF9,
    Start                 = 0xFA,    // System Real Time - Start
    Continue              = 0xFB,    // System Real Time - Continue
    Stop                  = 0xFC,    // System Real Time - Stop
    Undefined_FD          = 0xFD,
    ActiveSensing         = 0xFE,    // System Real Time - Active Sensing
    SystemReset           = 0xFF,    // System Real Time - System Reset
};



enum MidiControlChangeNumber: uint8_t
{
    // High resolution Continuous Controllers MSB (+32 for LSB) ----------------
    BankSelect                  = 0,
    ModulationWheel             = 1,
    BreathController            = 2,
    // CC3 undefined
    FootController              = 4,
    PortamentoTime              = 5,
    DataEntryMSB                = 6,
    ChannelVolume               = 7,
    Balance                     = 8,
    // CC9 undefined
    Pan                         = 10,
    ExpressionController        = 11,
    EffectControl1              = 12,
    EffectControl2              = 13,
    // CC14 undefined
    // CC15 undefined
    GeneralPurposeController1   = 16,
    GeneralPurposeController2   = 17,
    GeneralPurposeController3   = 18,
    GeneralPurposeController4   = 19,

    DataEntryLSB                = 38,

    // Switches ----------------------------------------------------------------
    Sustain                     = 64,
    Portamento                  = 65,
    Sostenuto                   = 66,
    SoftPedal                   = 67,
    Legato                      = 68,
    Hold                        = 69,

    // Low resolution continuous controllers -----------------------------------
    SoundController1            = 70,   // Synth: Sound Variation   FX: Exciter On/Off
    SoundController2            = 71,   // Synth: Harmonic Content  FX: Compressor On/Off
    SoundController3            = 72,   // Synth: Release Time      FX: Distortion On/Off
    SoundController4            = 73,   // Synth: Attack Time       FX: EQ On/Off
    SoundController5            = 74,   // Synth: Brightness        FX: Expander On/Off
    SoundController6            = 75,   // Synth: Decay Time        FX: Reverb On/Off
    SoundController7            = 76,   // Synth: Vibrato Rate      FX: Delay On/Off
    SoundController8            = 77,   // Synth: Vibrato Depth     FX: Pitch Transpose On/Off
    SoundController9            = 78,   // Synth: Vibrato Delay     FX: Flange/Chorus On/Off
    SoundController10           = 79,   // Synth: Undefined         FX: Special Effects On/Off
    GeneralPurposeController5   = 80,
    GeneralPurposeController6   = 81,
    GeneralPurposeController7   = 82,
    GeneralPurposeController8   = 83,
    PortamentoControl           = 84,
    // CC85 to CC90 undefined
    Effects1                    = 91,   // Reverb send level
    Effects2                    = 92,   // Tremolo depth
    Effects3                    = 93,   // Chorus send level
    Effects4                    = 94,   // Celeste depth
    Effects5                    = 95,   // Phaser depth
    DataIncrement               = 96,
    DataDecrement               = 97,
    NRPNLSB                     = 98,   // Non-Registered Parameter Number (LSB)
    NRPNMSB                     = 99,   // Non-Registered Parameter Number (MSB)
    RPNLSB                      = 100,  // Registered Parameter Number (LSB)
    RPNMSB                      = 101,  // Registered Parameter Number (MSB)

    // Channel Mode messages ---------------------------------------------------
    AllSoundOff                 = 120,
    ResetAllControllers         = 121,
    LocalControl                = 122,
    AllNotesOff                 = 123,
    OmniModeOff                 = 124,
    OmniModeOn                  = 125,
    MonoModeOn                  = 126,
    PolyModeOn                  = 127
};


/*MIDI Channel Message parameter*/
struct MidiMessage{
    uint8_t channel;       // MIDI channel
    MidiType type;          // MIDI type
    uint8_t data1;         // MIDI data
    uint8_t data2;         // MIDI type
    bool valid;         // Identifies whether a MIDI message is valid
   // uint8_t length;
    uint8_t sysexArray[SYS_EX_MAXSIZE];  //System Exclusive dedicated byte array
    static const bool  Use1ByteParsing = true;

    unsigned getSysExSize() const
    {
        const unsigned size = unsigned(data2) << 8 | data1;
        return size > SYS_EX_MAXSIZE ? SYS_EX_MAXSIZE : size;
    }
};


#endif

