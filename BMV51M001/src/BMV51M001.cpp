/*************************************************************************
File:       	  BMV51M001.cpp
Author:          BESTMODULES
Description:    Make BMV51M001 Library for the MIDI Shield by MIDI protocol.
History：		  
	V1.0.1	 -- initial version； 2023-01-17； Arduino IDE : v1.8.19

**************************************************************************/
#include "BMV51M001.h"
/************************************************************************* 
Description:    Constructor
parameter:
    Input:          *theSerial : Wire object if your board has more than one UART interface
    Output:         
Return:         
Others:         
*************************************************************************/
BMV51M001::BMV51M001(HardwareSerial *theSerial)
{
    _serial = theSerial; 
    _mInputChannel = 0;
    _mPendingMessageIndex = 0;
    _mMidiDatabytes = 0;
    _mRunningStatus = false;
    _mRunningStatus_TX = InvalidType;
    _mRunningStatus_RX = InvalidType;
    _mCurrentRpnNumber = 0xffff;
    _mCurrentNrpnNumber = 0xffff;
    _useRunningStatus = false;
}
/************************************************************************* 
Description:    MIDI communication initialization
parameter:
    Input:          inputChannel：Set the MIDI input channel(Unique Value 1)
    Output:         
Return:         
Others:         
*************************************************************************/
void BMV51M001::begin(uint8_t inputChannel)
{
    _serial->begin(31250);  
    
    _mInputChannel = inputChannel;
    _mRunningStatus_TX = InvalidType;
    _mRunningStatus_RX = InvalidType;

    _mPendingMessageIndex = 0;
    _mMidiDatabytes = 0;

    _mCurrentRpnNumber  = 0xffff;
    _mCurrentNrpnNumber = 0xffff;

    _midiMessage.valid   = false;
    _midiMessage.type    = InvalidType;
    _midiMessage.channel = 0;
    _midiMessage.data1   = 0;
    _midiMessage.data2   = 0;
}


/************************************MIDI OUT***************************************/

/************************************************************************* 
Description:    Sends MIDI channel messages
parameter:
    Input:          type：   MIDI status bytes (excluding channels)
                    data1：  MIDI Message data byte 1
                    data2：  MIDI Message data byte 2
                    channel：The channel on which the message will be sent(1~16)
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::send(MidiType type, uint8_t data1, uint8_t data2, uint8_t channel)
{
    if (type <= PitchBend)// Channel messages
    {
        if (channel >= MIDI_CHANNEL_OFF || channel == MIDI_CHANNEL_OMNI || type < 0x80)
        {
            return;//Exit if the channel is wrong or the message is not of MIDI type
        }
        data1 &= 0x7f;//The value ranges from 0x00 to 0x7f to prevent users from sending incorrect data
        data2 &= 0x7f;
        
        uint8_t status = (type|((channel-1)&0x0f));//aaaannnn,aaaa is instruction,nnnn is channel

        if(beginTransmission(type))
        {
            if (_mRunningStatus)
            {
                if (_mRunningStatus_TX != status)//Note The status bytes have changed
                {
                    _mRunningStatus_TX = status;//Store the new status bytes for the next comparison
                    _serial->write(_mRunningStatus_TX);
                }		
            }
            else//Do not use the run state
            {
                _serial->write(status);//No running state is used, so the status bytes are sent regardless of whether they change
            }
            
            /*Sending data part*/
            _serial->write(data1);
            if (type != ProgramChange && type != AfterTouchChannel)//Except for these two state bytes,  all other channel messages contain two data bytes
            {
                _serial->write(data2);
            }
            endTransmission();
        }
    }
    else if(type >= Clock)
    {
        sendRealTime(type); // System Real-time and 1 byte.
    }
}
/************************************************************************* 
Description:    Send a Note Off message
parameter:
    Input:      noteNumber：Pitch value in the MIDI format (0 to 127).
                velocity：Release velocity (0 to 127).
                channel：The channel on which the message will be sent (1 to 16).
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendNoteOff(uint8_t noteNumber, uint8_t velocity, uint8_t channel)
{
    send(NoteOff, noteNumber, velocity, channel);
}
/************************************************************************* 
Description:    Send a Note On message
parameter:
    Input:     noteNumber：Pitch value in the MIDI format (0 to 127).
                velocity：Note attack velocity (0 to 127). A NoteOn with 0 velocity
                          is considered as a NoteOff.
                channel：The channel on which the message will be sent (1 to 16).
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendNoteOn(uint8_t noteNumber, uint8_t velocity, uint8_t channel)
{
    send(NoteOn, noteNumber, velocity, channel);
}
/************************************************************************* 
Description:    Send Polyphonic AfterTouch message (applies to a specified note)
parameter:
    Input:      noteNumber：The note to apply AfterTouch to (0 to 127).
                pressure：The amount of AfterTouch to apply (0 to 127).
                channel：The channel on which the message will be sent(1~16)
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendPolyPressure(uint8_t noteNumber, uint8_t pressure, uint8_t channel)
{
  send(AfterTouchPoly, noteNumber, pressure, channel);
}
/************************************************************************* 
Description:    Send a Control Change message
parameter:
    Input:      controlNumber：The controller number (0 to 127).
                controlValue：The value for the specified controller (0 to 127).
                channel：The channel on which the message will be sent (1 to 16).
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendControlChange(uint8_t controlNumber, uint8_t controlValue, uint8_t channel)
{
    send(ControlChange, controlNumber, controlValue, channel);
}
/************************************************************************* 
Description:    Send a Program Change message
parameter:
    Input:      programNumber：The Program to select (0 to 127).
                channel：The channel on which the message will be sent (1 to 16).
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendProgramChange(uint8_t programNumber, uint8_t channel)
{
    send(ProgramChange, programNumber, 0, channel);
}
/************************************************************************* 
Description:    Send a MonoPhonic AfterTouch message (applies to all notes)
parameter:
    Input:     pressure：The amount of AfterTouch to apply (0 to 127).
                channel：The channel on which the message will be sent(1~16)
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendAfterTouch(uint8_t pressure, uint8_t channel)
{
  send(AfterTouchChannel, pressure, 0, channel);
}
/************************************************************************* 
Description:    Send a Pitch Bend message using a signed integer value.
parameter:
    Input:      pitchValue：The amount of bend to send (in a signed integer format),
                            The range is -8192 ~ 8191
                            0 : the central (no bend) setting.
                            -8192 :the maximum downwards bend, and
                            8191 :the maximum upwards bend.
                channel：The channel on which the message will be sent(1~16)
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendPitchBend(int16_t pitchValue, uint8_t channel)
{
  const unsigned bend = unsigned(pitchValue - int(MIDI_PITCHBEND_MIN));
  send(PitchBend, (bend & 0x7f), (bend >> 7) & 0x7f, channel);	
}
/************************************************************************* 
Description:    Generate and send a System Exclusive frame
parameter:
    Input:      length：The size of the array to send
                array：The byte array containing the data to send
                arrayContainsBoundaries：When set to 'true', 0xf0 & 0xf7 bytes
                                        (start & stop SysEx) will NOT be sent
                                        (and therefore must be included in the array).
                                        default value for ArrayContainsBoundaries is set to 'false'
Output:        
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendSysEx(uint16_t length, const uint8_t* array, bool arrayContainsBoundaries)
{
    const bool writeBeginEndBytes = !arrayContainsBoundaries;

    if (beginTransmission(MidiType::SystemExclusiveStart))
    {
        if (writeBeginEndBytes)
        {
            _serial->write(MidiType::SystemExclusiveStart);
        }
            
        for (unsigned i = 0; i < length; ++i)
        {
            _serial->write(array[i]);
        }
        if (writeBeginEndBytes)
        {
            _serial->write(MidiType::SystemExclusiveEnd);
        }
        endTransmission();
    }

    if (_useRunningStatus)
    {
        _mRunningStatus_TX = InvalidType;//_mRunningStatus_TX is cleared when a System Exclusive or Common status message is received
    }      
    
}
/************************************************************************* 
Description:    Send a MIDI Time Code Quarter Frame.
parameter:
    Input:      typeNibble：Message type
                valuesNibble：MTC data
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendTimeCodeQuarterFrame(uint8_t typeNibble, uint8_t valuesNibble)
{
  const uint8_t data = uint8_t((((typeNibble & 0x07) << 4) | (valuesNibble & 0x0f)));
  sendTimeCodeQuarterFrame(data); 
}
/************************************************************************* 
Description:    Send a MIDI Time Code Quarter Frame.
parameter:
    Input:      data：if you want to encode directly the nibbles in your program,
                      you can send the byte here.
Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendTimeCodeQuarterFrame(uint8_t data)
{
  sendCommon(TimeCodeQuarterFrame, data);
}
/************************************************************************* 
Description:    Send a Song Position Pointer message.
parameter:
    Input:      beats：The number of beats since the start of the song.
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendSongPosition(uint16_t beats)
{
  sendCommon(SongPosition, beats);
}
/************************************************************************* 
Description:    Send a Song Select message
parameter:
    Input:      songNumber：The number of song.
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendSongSelect(uint8_t songNumber)
{
  sendCommon(SongSelect, songNumber);
}
/************************************************************************* 
Description:    Send a Tune Request message.
parameter:
    Input:          
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendTuneRequest()
{
  sendCommon(TuneRequest);
}
/************************************************************************* 
Description:    Send a Common message. Common messages reset the running status
parameter:
    Input:      type：The available Common types are:
                      TimeCodeQuarterFrame, SongPosition, SongSelect and TuneRequest.
                data1：The byte that goes with the common message.
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendCommon(MidiType type, uint16_t data)
{
    switch (type)
    {
        case TimeCodeQuarterFrame:
        case SongPosition:
        case SongSelect:
        case TuneRequest:
            break;
        default:
            // Invalid Common marker
            return;
    }
    if (beginTransmission(type))
    {
        _serial->write((uint8_t)type);
        switch (type)
        {
            case TimeCodeQuarterFrame:
                _serial->write(data);
                break;
            case SongPosition:
                _serial->write(data & 0x7f);
                _serial->write((data >> 7) & 0x7f);
                break;
            case SongSelect:
                _serial->write(data & 0x7f);
                break;
            case TuneRequest:
                break;
            default:
                break; // LCOV_EXCL_LINE - Coverage blind spot
        }
        endTransmission();
    }

    if (_useRunningStatus)
    {
        _mRunningStatus_TX = InvalidType;   
    }         
    
}

/************************************************************************* 
Description:    Send a Real Time (one byte) message.
parameter:
    Input:      type：The available Real Time types are:
                      Start, Stop, Continue, Clock, ActiveSensing and SystemReset.
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendRealTime(MidiType type)
{
  // Do not invalidate Running Status for real-time messages
  // as they can be interleaved within any message.

    switch (type)
    {
        case Clock:
        case Start:
        case Stop:
        case Continue:
        case ActiveSensing:
        case SystemReset:
            if (beginTransmission(type))
            {
                _serial->write((uint8_t)type);
                endTransmission();
            }
            break;
        default:
            // Invalid Real Time marker
            break;
    }   
    
}
/************************************************************************* 
Description:    Start a Registered Parameter Number frame.
parameter:
    Input:      number：The 14-bit number of the RPN you want to select.
                channel：The channel on which the message will be sent (1 to 16).
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::beginRpn(uint16_t number, uint8_t channel)
{
  if (_mCurrentRpnNumber != number)
  {
    const uint8_t numMsb = 0x7f & (number >> 7);
    const uint8_t numLsb = 0x7f & number;
    sendControlChange(RPNLSB, numLsb, channel);
    sendControlChange(RPNMSB, numMsb, channel);
    _mCurrentRpnNumber = number;
  }
}
/************************************************************************* 
Description:    Send a 14-bit value for the currently selected RPN number.
parameter:
    Input:      value：The 14-bit value of the selected RPN.
                channel：The channel on which the message will be sent (1 to 16).
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendRpnValue(uint16_t value, uint8_t channel)
{
  const uint8_t valMsb = 0x7f & (value >> 7);
  const uint8_t valLsb = 0x7f & value;
  sendControlChange(DataEntryMSB, valMsb, channel);
  sendControlChange(DataEntryLSB, valLsb, channel);
}
/************************************************************************* 
Description:    Send separate MSB/LSB values for the currently selected RPN number.
parameter:
    Input:      msb：The MSB part of the value to send. Meaning depends on RPN number.
                lsb：The LSB part of the value to send. Meaning depends on RPN number.
                channel：The channel on which the message will be sent (1 to 16).
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendRpnValue(uint8_t msb, uint8_t lsb, uint8_t channel)
{
  sendControlChange(DataEntryMSB, msb, channel);
  sendControlChange(DataEntryLSB, lsb, channel);  
}
/************************************************************************* 
Description:    Increment the value of the currently selected RPN number by the specified amount.
parameter:
    Input:      amount：The amount to add to the currently selected RPN value.
                channel：The channel on which the message will be sent (1 to 16).
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendRpnIncrement(uint8_t amount, uint8_t channel)
{
  sendControlChange(DataIncrement, amount, channel);
}
/************************************************************************* 
Description:    Decrement the value of the currently selected RPN number by the specified amount.
parameter:
    Input:      amount：The amount to subtract to the currently selected RPN value.
                channel：The channel on which the message will be sent (1 to 16).
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendRpnDecrement(uint8_t amount, uint8_t channel)
{
  sendControlChange(DataDecrement, amount, channel);
}
/************************************************************************* 
Description:    Terminate an RPN frame.
                This will send a Null Function to deselect the currently selected RPN.
parameter:
    Input:      channel：The channel on which the message will be sent (1 to 16).                
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::endRpn(uint8_t channel)
{
  sendControlChange(RPNLSB, 0x7f, channel);
  sendControlChange(RPNMSB, 0x7f, channel);
  _mCurrentRpnNumber = 0xffff;  
}
/************************************************************************* 
Description:    Start a Non-Registered Parameter Number frame.
parameter:
    Input:      number：The 14-bit number of the NRPN you want to select.
                channel：The channel on which the message will be sent (1 to 16). 
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::beginNrpn(uint16_t number, uint8_t channel)
{
  if (_mCurrentNrpnNumber != number)
  {
      const byte numMsb = 0x7f & (number >> 7);
      const byte numLsb = 0x7f & number;
      sendControlChange(NRPNLSB, numLsb, channel);
      sendControlChange(NRPNMSB, numMsb, channel);
      _mCurrentNrpnNumber = number;
  }  
}
/************************************************************************* 
Description:    Send a 14-bit value for the currently selected NRPN number.
parameter:
    Input:      value：The 14-bit value of the selected NRPN.
                channel：The channel on which the message will be sent (1 to 16). 
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendNrpnValue(uint16_t value, uint8_t channel)
{
  const byte valMsb = 0x7f & (value >> 7);
  const byte valLsb = 0x7f & value;
  sendControlChange(DataEntryMSB, valMsb, channel);
  sendControlChange(DataEntryLSB, valLsb, channel);  
}
/************************************************************************* 
Description:    Send separate MSB/LSB values for the currently selected NRPN number.
parameter:
    Input:      msb：The MSB part of the value to send. Meaning depends on NRPN number.
                lsb：The LSB part of the value to send. Meaning depends on NRPN number.
                channel：The channel on which the message will be sent (1 to 16). 
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendNrpnValue(uint8_t msb, uint8_t lsb, uint8_t channel)
{
  sendControlChange(DataEntryMSB, msb, channel);
  sendControlChange(DataEntryLSB, lsb, channel);  
}
/************************************************************************* 
Description:    Increment the value of the currently selected NRPN number by the specified amount.
parameter:
    Input:      amount：The amount to add to the currently selected NRPN value.
                channel：The channel on which the message will be sent (1 to 16). 
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendNrpnIncrement(uint8_t amount, uint8_t channel)
{
  sendControlChange(DataIncrement, amount, channel);
}
/************************************************************************* 
Description:    Decrement the value of the currently selected NRPN number by the specified amount.
parameter:
    Input:      amount：The amount to subtract to the currently selected NRPN value.
                channel：The channel on which the message will be sent (1 to 16).
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::sendNrpnDecrement(uint8_t amount, uint8_t channel)
{
  sendControlChange(DataDecrement, amount, channel);
}
/************************************************************************* 
Description:    Terminate an NRPN frame.
                This will send a Null Function to deselect the currently selected NRPN.
parameter:
    Input:      channel：The channel on which the message will be sent (1 to 16).  
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::endNrpn(uint8_t channel)
{
  sendControlChange(NRPNLSB, 0x7f, channel);
  sendControlChange(NRPNMSB, 0x7f, channel);
  _mCurrentNrpnNumber = 0xffff;
}




/************************************MIDI IN***************************************/

/************************************************************************* 
Description:    Read messages from the serial port using the main input channel.
parameter:
    Input:          
    Output:         
Return:         1: if a valid MIDI message
                0: false if not.
                A valid message is a message that matches the input channel.
Others:         
**************************************************************************/
bool BMV51M001::isMIDIMessageOK(void)
{
    if (_mInputChannel >= MIDI_CHANNEL_OFF)
        return false; // MIDI Input disabled.

    if (!parse())
    {
        return false;
    }

    const bool channelMatch = inputFilter(_mInputChannel);
    if (channelMatch)
    {
        launchCallback();
    }
        
    return channelMatch;
}
/************************************************************************* 
Description:    Gets the received MIDI message.
parameter:
    Input:      array[]:Stores the received MIDI message.
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::getMIDIMessage(uint8_t array[])
{
    if(checkMessageValid())
    {
        if((_midiMessage.type == SystemExclusiveStart)
            ||  (_midiMessage.type == SystemExclusiveEnd))
        {
            memcpy(array, _midiMessage.sysexArray, _midiMessage.getSysExSize());
        }
        else
        {
          array[0] = _midiMessage.type;
          array[1] = _midiMessage.data1;
          array[2] = _midiMessage.data2;
          array[3] = _midiMessage.channel;
        }
    }
}
/************************************************************************* 
Description:    MIDI parser
parameter:
    Input:          
    Output:         
Return:         false：No data available or Complete MIDI message reception is not complete
				        true：Complete MIDI message reception is complete
Others:         
**************************************************************************/
bool BMV51M001::parse(void)
{
    if (_serial->available() == 0)//The serial port did not receive the message
    {
        return false;
    }

    const uint8_t extracted =  _serial->read();//Extract data from the serial port

        // Ignore Undefined
    if (extracted == Undefined_FD)
    {
        return (MidiMessage::Use1ByteParsing) ? false : parse();
    }
    
    if (_mPendingMessageIndex == 0)//Starts new message reception without pending messages
    {
        _mPendingMessage[0] = extracted;

        // Check for running status first
        if (isChannelMessage(getTypeFromStatusByte(_mRunningStatus_RX)))
        {
            // Only these types allow Running Status

            // If the status byte is not received, prepend it
            // to the pending message

            if (extracted < 0x80)
            {
                _mPendingMessage[0]   = _mRunningStatus_RX;
                _mPendingMessage[1]   = extracted;
                _mPendingMessageIndex = 1;
            }
            // Else: well, we received another status byte,
            // so the running status does not apply here.
            // It will be updated upon completion of this message.

        }

        const MidiType pendingType = getTypeFromStatusByte(_mPendingMessage[0]);

        switch (pendingType)
        {
            // 1 byte messages
            case Start:
            case Continue:
            case Stop:
            case Clock:
            case ActiveSensing:
            case SystemReset:
            case TuneRequest:
                // Handle the message type directly here.
                _midiMessage.type    = pendingType;
                _midiMessage.channel = 0;
                _midiMessage.data1   = 0;
                _midiMessage.data2   = 0;
                _midiMessage.valid   = true;

                // Do not reset all input attributes, Running Status must remain unchanged.
                // We still need to reset these
                _mPendingMessageIndex = 0;
                _mMidiDatabytes = 0;

                return true;
                break;
            //2 bytes messages
            case ProgramChange:
            case AfterTouchChannel:
            case TimeCodeQuarterFrame:
            case SongSelect:
                _mMidiDatabytes = 1;
                break;
            
            //3 bytes messages
            case NoteOn:
            case NoteOff:
            case ControlChange:
            case PitchBend:
            case AfterTouchPoly:
            case SongPosition:
                _mMidiDatabytes = 2;
                break;   

            case SystemExclusiveStart:
            case SystemExclusiveEnd:
                // The message can be any length
                // between 3 and MidiMessage::sSysExMaxSize bytes
                _mMidiDatabytes = SYS_EX_MAXSIZE - 1;
                _mRunningStatus_RX = InvalidType;
                _midiMessage.sysexArray[0] = pendingType;
                break;
            case InvalidType:
            default:
                resetInput();
                return false;
                break;
        }
        if (_mPendingMessageIndex >= _mMidiDatabytes)
        {
                // Reception complete
            _midiMessage.type    = pendingType;
            _midiMessage.channel = getChannelFromStatusByte(_mPendingMessage[0]);
            _midiMessage.data1   = _mPendingMessage[1];
            _midiMessage.data2   = 0; // Completed new message has 1 data byte
            //  _midiMessage.length  = 1;

            _mPendingMessageIndex = 0;
            _mMidiDatabytes = 0;
            _midiMessage.valid = true;

            return true;     
        }
        else
        {
            // Waiting for more data
            _mPendingMessageIndex++;
        }
        return (MidiMessage::Use1ByteParsing) ? false : parse();//Continue receiving bytes for parsing
    }
    else//There were already messages waiting to be processed
    {
        if (extracted >= 0x80)//Determine if it is a new status byte message
        {
            // Reception of status bytes in the middle of an uncompleted message
            // are allowed only for interleaved Real Time message or EOX
            switch (extracted)
            {
                case Clock:
                case Start:
                case Continue:
                case Stop:
                case ActiveSensing:
                case SystemReset:

                    // Here we will have to extract the one-byte message,
                    // pass it to the structure for being read outside
                    // the MIDI class, and recompose the message it was
                    // interleaved into. Oh, and without killing the running status..
                    // This is done by leaving the pending message as is,
                    // it will be completed on next calls.
                    _midiMessage.type    = (MidiType)extracted;
                    _midiMessage.data1   = 0;
                    _midiMessage.data2   = 0;
                    _midiMessage.channel = 0;
                    _midiMessage.valid   = true;

                    return true;

                    // Exclusive
                case SystemExclusiveStart:
                case SystemExclusiveEnd:
                    if ((_midiMessage.sysexArray[0] == SystemExclusiveStart)
                    ||  (_midiMessage.sysexArray[0] == SystemExclusiveEnd))
                    {
                        // Store the last byte (EOX:F7)sysexArray{f0 xx xx f7}
                        _midiMessage.sysexArray[_mPendingMessageIndex++] = extracted;
                        _midiMessage.type = SystemExclusive;

                        // Get length
                        _midiMessage.data1   = _mPendingMessageIndex & 0xff; // LSB
                        _midiMessage.data2   = uint8_t(_mPendingMessageIndex >> 8);   // MSB
                        _midiMessage.channel = 0;
                        _midiMessage.valid   = true;

                        resetInput();

                        return true;
                    }
                    else
                    {
                        resetInput();
                        return false;
                    }

                default:
                    break; // LCOV_EXCL_LINE - Coverage blind spot
            }
            
        }
        // Add extracted data byte to pending message
        if ((_mPendingMessage[0] == SystemExclusiveStart)
        ||  (_mPendingMessage[0] == SystemExclusiveEnd))
        {
            _midiMessage.sysexArray[_mPendingMessageIndex] = extracted;//if system exclusive message data，store it in sysexArray
        } 
        else
        {
            _mPendingMessage[_mPendingMessageIndex] = extracted;//Otherwise, it is stored in the normal MIDI message processing array
        }
            
        // Now we are going to check if we have reached the end of the message
        if (_mPendingMessageIndex >= _mMidiDatabytes)
        {
            // SysEx larger than the allocated buffer size,
            // Split SysEx like so:
            //   first:  0xF0 .... 0xF0
            //   midlle: 0xF7 .... 0xF0
            //   last:   0xF7 .... 0xF7
            if ((_mPendingMessage[0] == SystemExclusiveStart)
            ||  (_mPendingMessage[0] == SystemExclusiveEnd))
            {
                auto lastByte = _midiMessage.sysexArray[SYS_EX_MAXSIZE - 1];
                _midiMessage.sysexArray[SYS_EX_MAXSIZE - 1] = SystemExclusiveStart;
                _midiMessage.type = SystemExclusive;

                // Get length
                _midiMessage.data1   = SYS_EX_MAXSIZE & 0xff; // LSB
                _midiMessage.data2   = uint8_t(SYS_EX_MAXSIZE >> 8); // MSB
                _midiMessage.channel = 0;
                _midiMessage.valid   = true;

                // No need to check against the inputChannel,
                // SysEx ignores input channel
                launchCallback();

                _midiMessage.sysexArray[0] = SystemExclusiveEnd;
                _midiMessage.sysexArray[1] = lastByte;

                _mPendingMessageIndex = 2;

                return false;
            }

            _midiMessage.type = getTypeFromStatusByte(_mPendingMessage[0]);

            if (isChannelMessage(_midiMessage.type))
            {
                _midiMessage.channel = getChannelFromStatusByte(_mPendingMessage[0]);
            }   
            else
            {
                _midiMessage.channel = 0;//system exclusive message are not assigned to any particular MIDI channel
            }
                
            _midiMessage.data1 = _mPendingMessage[1];
            // Save data2 only if applicable
            _midiMessage.data2 = _mMidiDatabytes == 2 ? _mPendingMessage[2] : 0;

            // Reset local variables
            _mPendingMessageIndex = 0;
            _mMidiDatabytes = 0;

            _midiMessage.valid = true;

            // Activate running status (if enabled for the received type)
            switch (_midiMessage.type)
            {
                case NoteOff:
                case NoteOn:
                case AfterTouchPoly:
                case ControlChange:
                case ProgramChange:
                case AfterTouchChannel:
                case PitchBend:
                    // Running status enabled: store it from received message
                    _mRunningStatus_RX = _mPendingMessage[0];
                    break;

                default:
                    // No running status
                    _mRunningStatus_RX = InvalidType;
                    break;
            }
            return true;
        }
        else
        {
            _mPendingMessageIndex++;//Waiting to receive a complete MIDI message
            return (MidiMessage::Use1ByteParsing) ? false : parse();//Continue receiving bytes for parsing
        }
    }
    
}
/************************************************************************* 
Description:    check if the received message is on the listened channel
parameter:
    Input:      channel：The channel on which the message will be sent (1 to 16).  
    Output:         
Return:         
Others:         
**************************************************************************/
bool BMV51M001::inputFilter(uint8_t channel)
{
  // This method handles recognition of channel
  // (to know if the message is destinated to the Arduino)

  // First, check if the received message is Channel
  if (_midiMessage.type >= NoteOff && _midiMessage.type <= PitchBend)
  {
    // Then we need to know if we listen to it
    if ((_midiMessage.channel == channel) ||
        (channel == MIDI_CHANNEL_OMNI))
    {
        return true;
    }
    else
    {
        // We don't listen to this channel
        return false;

    }
  }
  else
  {
    // System messages are always received
    return true;
  }  
}
/************************************************************************* 
Description:    reset input attributes
    Input:          
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::resetInput(void)
{
  _mMidiDatabytes = 0;
  _mPendingMessageIndex = 0;
  _mRunningStatus_RX = InvalidType;
}
/************************************************************************* 
Description:    Get the last received message's type
parameter:
    Input:      parameter1:midi message data1
                parameter2:midi message data2
                channel:midi message channel
    Output:         
Return:         Returns an enumerated type
Others:         
**************************************************************************/
MidiType BMV51M001::getMessageType(void) 
{
  return _midiMessage.type;
}
/************************************************************************* 
Description:    Get the channel of the message stored in the structure.
parameter:
    Input:          
    Output:         
Return:         Channel range is 1 to 16.
                For non-channel messages, this will return 0.
Others:         
**************************************************************************/
uint8_t BMV51M001::getMessageChannel(void)
{
  return _midiMessage.channel;
}
/************************************************************************* 
Description:    Get the first data byte of the last received message.
parameter:
    Input:          
    Output:         
Return:         return MIDI Message's data1
Others:         
**************************************************************************/
uint8_t BMV51M001::getMessageData1(void)
{
  return _midiMessage.data1;
}
/************************************************************************* 
Description:    Get the second data byte of the last received message.
parameter:
    Input:          
    Output:         
Return:         return MIDI Message's data2
Others:         
**************************************************************************/
uint8_t BMV51M001::getMessageData2(void)
{
  return _midiMessage.data2;
}
/************************************************************************* 
Description:    Get the System Exclusive byte array.
parameter:
    Input:          
    Output:     return System Exclusive byte array     
Return:         
Others:         
**************************************************************************/
uint8_t BMV51M001::getSysExArray(uint8_t dataBuffer[]) 
{
    memcpy( dataBuffer,_midiMessage.sysexArray,_midiMessage.getSysExSize());
    return _midiMessage.getSysExSize();
}

/************************************************************************* 
Description:    Check if a valid message is stored in the structure
parameter:
    Input:          
    Output:         
Return:         return false：invalid message
                       true：valid message
Others:         
**************************************************************************/
bool BMV51M001::checkMessageValid(void) 
{
  bool result =   _midiMessage.valid;
  _midiMessage.valid=false;
  return result;
}
/************************************************************************* 
Description:    get MIDI Input Channel
parameter:
    Input:          
    Output:         
Return:         return MIDI Input Channel
Others:         
**************************************************************************/
uint8_t BMV51M001::getInputChannel(void)
{
  return _mInputChannel;
}
/************************************************************************* 
Description:    Set the value for the input MIDI channel
parameter:
    Input:      inputChannel：the channel value. Valid values are 1 to 16, MIDI_CHANNEL_OMNI
                if you want to listen to all channels, and MIDI_CHANNEL_OFF to disable input.
    Output:         
Return:         return MIDI Input Channel
Others:         
**************************************************************************/
void BMV51M001::setInputChannel(uint8_t inputChannel)
{
  _mInputChannel = inputChannel;
}
/************************************************************************* 
Description:    Check whether it is MIDI Channel Messages
parameter:
    Input:          
    Output:         
Return:         return true：it is MIDI Channel Messages
                       false:：not MIDI Channel Messages
Others:         
**************************************************************************/
bool BMV51M001::isChannelMessage(MidiType type)
{
    return (type == NoteOff           ||
            type == NoteOn            ||
            type == ControlChange     ||
            type == AfterTouchPoly    ||
            type == AfterTouchChannel ||
            type == PitchBend         ||
            type == ProgramChange);
}
/************************************************************************* 
Description:    Extract an enumerated MIDI type from a status byte.
parameter:
    Input:      inStatus：1byte data
    Output:         
Return:         return InvalidType：Invalid MidiType
                       or valid midi type byte
Others:         
**************************************************************************/
MidiType BMV51M001::getTypeFromStatusByte(uint8_t status)
{
  if ((status  < 0x80) ||
      (status == Undefined_F4) ||
      (status == Undefined_F5) ||
      (status == Undefined_FD))
  {
    return InvalidType; // Data bytes and undefined.        
  }

  if (status < 0xf0)
  {
      // Channel message, remove channel nibble.
      return MidiType(status & 0xf0);
  }

  return MidiType(status);

}
/************************************************************************* 
Description:    get MIDI channel
parameter:
    Input:          
    Output:         
Return:         Returns channel in the range 1-16
Others:         
**************************************************************************/
uint8_t BMV51M001::getChannelFromStatusByte(uint8_t status)
{
  return uint8_t((status & 0x0f) + 1);
}
/************************************************************************* 
Description:    Detach an external function from the given type.
                Use this method to cancel the effects of setHandle********.
parameter:
    Input:          
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::disconnectCallbackFromType(MidiType type)
{
    switch (type)
    {
        case NoteOff:               mNoteOffCallback                = nullptr; break;
        case NoteOn:                mNoteOnCallback                 = nullptr; break;
        case AfterTouchPoly:        mAfterTouchPolyCallback         = nullptr; break;
        case ControlChange:         mControlChangeCallback          = nullptr; break;
        case ProgramChange:         mProgramChangeCallback          = nullptr; break;
        case AfterTouchChannel:     mAfterTouchChannelCallback      = nullptr; break;
        case PitchBend:             mPitchBendCallback              = nullptr; break;
        case SystemExclusive:       mSystemExclusiveCallback        = nullptr; break;
        case TimeCodeQuarterFrame:  mTimeCodeQuarterFrameCallback   = nullptr; break;
        case SongPosition:          mSongPositionCallback           = nullptr; break;
        case SongSelect:            mSongSelectCallback             = nullptr; break;
        case TuneRequest:           mTuneRequestCallback            = nullptr; break;
        case Clock:                 mClockCallback                  = nullptr; break;
        case Start:                 mStartCallback                  = nullptr; break;
        case Continue:              mContinueCallback               = nullptr; break;
        case Stop:                  mStopCallback                   = nullptr; break;
        case ActiveSensing:         mActiveSensingCallback          = nullptr; break;
        case SystemReset:           mSystemResetCallback            = nullptr; break;
        default:
            break;
    }
}
/************************************************************************* 
Description:    launch callback function based on received type.
parameter:
    Input:          
    Output:         
Return:         
Others:         
**************************************************************************/
void BMV51M001::launchCallback()
{
  if (mMessageCallback != 0) 
  {
    mMessageCallback(_midiMessage);
  }
  // The order is mixed to allow frequent messages to trigger their callback faster.
  switch (_midiMessage.type)
  {
        // Notes
    case NoteOff:               if (mNoteOffCallback != nullptr)               mNoteOffCallback(_midiMessage.channel, _midiMessage.data1, _midiMessage.data2);   break;
    case NoteOn:                if (mNoteOnCallback != nullptr)                mNoteOnCallback(_midiMessage.channel, _midiMessage.data1, _midiMessage.data2);    break;

        // Real-time messages
    case Clock:                 if (mClockCallback != nullptr)                 mClockCallback();           break;
    case Start:                 if (mStartCallback != nullptr)                 mStartCallback();           break;
    case Continue:              if (mContinueCallback != nullptr)              mContinueCallback();        break;
    case Stop:                  if (mStopCallback != nullptr)                  mStopCallback();            break;
    case ActiveSensing:         if (mActiveSensingCallback != nullptr)         mActiveSensingCallback();   break;

        // Continuous controllers
    case ControlChange:         if (mControlChangeCallback != nullptr)         mControlChangeCallback(_midiMessage.channel, _midiMessage.data1, _midiMessage.data2);    break;
    case PitchBend:             if (mPitchBendCallback != nullptr)             mPitchBendCallback(_midiMessage.channel, (int)((_midiMessage.data1 & 0x7f) | ((_midiMessage.data2 & 0x7f) << 7)) + MIDI_PITCHBEND_MIN); break;
    case AfterTouchPoly:        if (mAfterTouchPolyCallback != nullptr)        mAfterTouchPolyCallback(_midiMessage.channel, _midiMessage.data1, _midiMessage.data2);    break;
    case AfterTouchChannel:     if (mAfterTouchChannelCallback != nullptr)     mAfterTouchChannelCallback(_midiMessage.channel, _midiMessage.data1);    break;

    case ProgramChange:         if (mProgramChangeCallback != nullptr)         mProgramChangeCallback(_midiMessage.channel, _midiMessage.data1);    break;
    case SystemExclusive:       if (mSystemExclusiveCallback != nullptr)       mSystemExclusiveCallback(_midiMessage.sysexArray, _midiMessage.getSysExSize());    break;

        // Occasional messages
    case TimeCodeQuarterFrame:  if (mTimeCodeQuarterFrameCallback != nullptr)  mTimeCodeQuarterFrameCallback(_midiMessage.data1);    break;
    case SongPosition:          if (mSongPositionCallback != nullptr)          mSongPositionCallback(unsigned((_midiMessage.data1 & 0x7f) | ((_midiMessage.data2 & 0x7f) << 7)));    break;
    case SongSelect:            if (mSongSelectCallback != nullptr)            mSongSelectCallback(_midiMessage.data1);    break;
    case TuneRequest:           if (mTuneRequestCallback != nullptr)           mTuneRequestCallback();    break;

    case SystemReset:           if (mSystemResetCallback != nullptr)           mSystemResetCallback();    break;

    case InvalidType:
    default:
        break; // LCOV_EXCL_LINE - Unreacheable code, but prevents unhandled case warning.
  }
}

