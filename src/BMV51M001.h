/***************************************************************************
File:       		BMV51M001.h
Author:            	 BESTMODULES
Description:        Make BMV51M001 Library for the MIDI Shield by MIDI protocol.
History：			-
	V1.0.1	 -- initial version；2023-01-17；Arduino IDE : ≥v1.8.13

****************************************************************************/
/*
 MIDI protocol can be obtained on this website
https://www.midi.org/specifications/midi1-specifications/m1-v4-2-1-midi-1-0-detailed-specification-96-1-4
*/
#ifndef  _BMV51M001_H
#define  _BMV51M001_H

#include "Arduino.h"
#include "BM_MIDIDefine.h"


/*****************class for the MIDI*******************/
class BMV51M001
{
public:
    BMV51M001(HardwareSerial *theSerial  = &Serial);
    //default receive data on channel 0
	void begin(uint8_t inputChannel = 1);
	/******************************************MIDI OUT*************************************/
    /*CHANNEL VOICE MESSAGES*/
	void send(MidiType type, uint8_t data1, uint8_t data2, uint8_t channel);  
    void sendNoteOff(uint8_t noteNumber, uint8_t velocity, uint8_t channel);
    void sendNoteOn(uint8_t noteNumber, uint8_t velocity, uint8_t channel);
    void sendPolyPressure(uint8_t noteNumber, uint8_t pressure, uint8_t channel);
    void sendControlChange(uint8_t controlNumber, uint8_t controlValue, uint8_t channel); 
    void sendProgramChange(uint8_t programNumber, uint8_t channel);
    void sendAfterTouch(uint8_t pressure, uint8_t channel);
    void sendPitchBend(int16_t pitchValue, uint8_t channel);
    /*SYSTEM EXCLUSIVE MESSAGES*/
    void sendSysEx(uint16_t length, const uint8_t* array, bool arrayContainsBoundaries = false);
    /*SYSTEM COMMON MESSAGES*/
    void sendTimeCodeQuarterFrame(uint8_t typeNibble, uint8_t valuesNibble);
    void sendTimeCodeQuarterFrame(uint8_t data);
    void sendSongPosition(uint16_t beats);
    void sendSongSelect(uint8_t songNumber);
    void sendTuneRequest();
    void sendCommon(MidiType type, uint16_t data=0);
    /*SYSTEM REAL TIME MESSAGES*/
    void sendRealTime(MidiType type);
    void sendClock(void)         { sendRealTime(Clock); };
    void sendStart(void)         { sendRealTime(Start); };
    void sendContinue(void)      { sendRealTime(Continue);  };
    void sendStop(void)          { sendRealTime(Stop);  };
    void sendActiveSensing(void) { sendRealTime(ActiveSensing);  };
    void sendSystemReset(void)   { sendRealTime(SystemReset);  };   
    /*REGISTERED AND NON-REGISTERED PARAMETER NUMBERS(belong to the control change function)*/
    void beginRpn(uint16_t number, uint8_t channel);
    void sendRpnValue(uint16_t value, uint8_t channel);
    void sendRpnValue(uint8_t msb, uint8_t lsb, uint8_t channel);
    void sendRpnIncrement(uint8_t amount, uint8_t channel);
    void sendRpnDecrement(uint8_t amount, uint8_t channel);
    void endRpn(uint8_t channel);
    void beginNrpn(uint16_t number, uint8_t channel);
    void sendNrpnValue(uint16_t value, uint8_t channel);
    void sendNrpnValue(uint8_t msb, uint8_t lsb, uint8_t channel);
    void sendNrpnIncrement(uint8_t amount,uint8_t channel);
    void sendNrpnDecrement(uint8_t amount, uint8_t channel);
    void endNrpn(uint8_t channel);
    /******************************************MIDI IN*************************************/
    bool isMIDIMessageOK(void);
    void getMIDIMessage(uint8_t array[]);
    bool inputFilter(uint8_t channel);  
    MidiType getMessageType(void);
    uint8_t getMessageChannel(void);
    uint8_t getMessageData1(void);
    uint8_t getMessageData2(void);
    uint8_t getSysExArray(uint8_t dataBuffer[]); 
    bool checkMessageValid(void);
    uint8_t getInputChannel(void);
    void setInputChannel(uint8_t inputChannel);
/******************************************MIDI Callbacks*************************************/
public:
    void setHandleMessage(void (*fptr)(const MidiMessage&)) { mMessageCallback = fptr; };
    void setHandleNoteOff(NoteOffCallback fptr) { mNoteOffCallback = fptr; }
    void setHandleNoteOn(NoteOnCallback fptr) { mNoteOnCallback = fptr; }
    void setHandleAfterTouchPoly(AfterTouchPolyCallback fptr) { mAfterTouchPolyCallback = fptr; }
    void setHandleControlChange(ControlChangeCallback fptr) { mControlChangeCallback = fptr; }
    void setHandleProgramChange(ProgramChangeCallback fptr) { mProgramChangeCallback = fptr; }
    void setHandleAfterTouchChannel(AfterTouchChannelCallback fptr) { mAfterTouchChannelCallback = fptr; }
    void setHandlePitchBend(PitchBendCallback fptr) { mPitchBendCallback = fptr; }
    void setHandleSystemExclusive(SystemExclusiveCallback fptr) { mSystemExclusiveCallback = fptr; }
    void setHandleTimeCodeQuarterFrame(TimeCodeQuarterFrameCallback fptr) { mTimeCodeQuarterFrameCallback = fptr; }
    void setHandleSongPosition(SongPositionCallback fptr) { mSongPositionCallback = fptr; }
    void setHandleSongSelect(SongSelectCallback fptr) { mSongSelectCallback = fptr; }
    void setHandleTuneRequest(TuneRequestCallback fptr) { mTuneRequestCallback = fptr; }
    void setHandleClock(ClockCallback fptr) { mClockCallback = fptr; }
    void setHandleStart(StartCallback fptr) { mStartCallback = fptr; }
    void setHandleContinue(ContinueCallback fptr) { mContinueCallback = fptr; }
    void setHandleStop(StopCallback fptr) { mStopCallback = fptr; }
    void setHandleActiveSensing(ActiveSensingCallback fptr) { mActiveSensingCallback = fptr; }
    void setHandleSystemReset(SystemResetCallback fptr) { mSystemResetCallback = fptr; }
    void disconnectCallbackFromType(MidiType type);

private:
    void launchCallback();//Callback funtion
    void (*mMessageCallback)(const MidiMessage& message) = nullptr;
    NoteOffCallback mNoteOffCallback = nullptr;
    NoteOnCallback mNoteOnCallback = nullptr;
    AfterTouchPolyCallback mAfterTouchPolyCallback = nullptr;
    ControlChangeCallback mControlChangeCallback = nullptr;
    ProgramChangeCallback mProgramChangeCallback = nullptr;
    AfterTouchChannelCallback mAfterTouchChannelCallback = nullptr;
    PitchBendCallback mPitchBendCallback = nullptr;
    SystemExclusiveCallback mSystemExclusiveCallback = nullptr;
    TimeCodeQuarterFrameCallback mTimeCodeQuarterFrameCallback = nullptr;
    SongPositionCallback mSongPositionCallback = nullptr;
    SongSelectCallback mSongSelectCallback = nullptr;
    TuneRequestCallback mTuneRequestCallback = nullptr;
    ClockCallback mClockCallback = nullptr;
    StartCallback mStartCallback = nullptr;
    TickCallback mTickCallback = nullptr;
    ContinueCallback mContinueCallback = nullptr;
    StopCallback mStopCallback = nullptr;
    ActiveSensingCallback mActiveSensingCallback = nullptr;
    SystemResetCallback mSystemResetCallback = nullptr;
    
    //Call some things before sending
	bool beginTransmission(MidiType){return true;};
    //Call some things after sending
	void endTransmission(){};
    //is ChannelMessage?(see midi protocol)
    bool isChannelMessage(MidiType type);
    //get  type info from status(the first byte)
	static MidiType getTypeFromStatusByte(uint8_t status);
    //get  channel info from status
    static uint8_t getChannelFromStatusByte(uint8_t status);
    void resetInput(void);//Clear this receiving completion flag bit
    bool parse(void);//parse message
private:/* Internal variables */
    HardwareSerial *_serial = NULL;
    uint8_t             _mInputChannel;
    uint8_t             _mRunningStatus_TX;//Used to store status bytes
    uint8_t             _mRunningStatus_RX;//Used to store status bytes
    bool                _mRunningStatus;//true:use running status；false:not use running ststua
    unsigned            _mPendingMessageIndex;
    uint8_t             _mPendingMessage[3];
    uint8_t             _mMidiDatabytes;
    unsigned            _mCurrentRpnNumber;
    unsigned            _mCurrentNrpnNumber;
    MidiMessage         _midiMessage;
    bool                _useRunningStatus;
};

#endif

