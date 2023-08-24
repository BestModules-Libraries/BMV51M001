/******************************************************************
File:             MIDIOut.ino
Description:      Receive MIDI Messages from the MIDI Wrench APP by BMV51M001
                  then send to MIDI Device which with five pin standard digital instrument interface
Note:             -       
******************************************************************/
#include "BMV51M001.h"

BMV51M001 moduleMIDIInterface;  //To build the object

uint8_t parameter[10];
uint8_t velocity = 0;
uint8_t noteNumber = 0;
uint8_t channel = 1;
uint8_t type = 0;

void setup()
{
    moduleMIDIInterface.begin();  //Initialize the object, start MIDI, default listening channel is 1
}

void loop()
{
     // Read the MIDI message for the MIDI Wrench APP
    if (moduleMIDIInterface.isMIDIMessageOK())    
    {
        moduleMIDIInterface.getMIDIMessage(parameter); //Gets MIDI message data
        //Sends a MIDI message to the connected MIDI device
        type = parameter[0];//The first byte is the MIDI message typeThe channel is not included here(the channel is separated to the fourth byte).
        noteNumber = parameter[1]; //The second byte is the note information, see MIDI protocol manual for details
        velocity= parameter[2]; //The third byte is the strength information, see MIDI protocol manual for details
        channel = parameter[3]; //The fourth byte is the channel information

        switch(type)
        {
            case NoteOn:  
                moduleMIDIInterface.sendNoteOn(noteNumber, velocity, channel);
                break;
            case NoteOff:  
                moduleMIDIInterface.sendNoteOff(noteNumber, velocity, channel);
                break;
            default:
                break;
        }
    }
}
