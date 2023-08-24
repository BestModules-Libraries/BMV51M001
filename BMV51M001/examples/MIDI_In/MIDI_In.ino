/******************************************************************
File:             MIDI_In.ino
Description:      Receive MIDI Messages by MIDI Interface Module(BMV51M001)
                  and voice by MIDI Shield(BMV51T001)
Note:             -       
******************************************************************/
#include "BMV51T001.h"
#include "BMV51M001.h"

BMV51M001 moduleMIDIInterface(&Serial4);   //Build MIDI interface module objects
BMV51T001 shieldMIDIPlayer;    //Builds MIDI Shield objects

uint8_t parameter[10];
uint8_t velocity = 0;
uint8_t noteNumber = 0;//
uint8_t channel = 1;
uint8_t type = 0;

void setup()
{
   moduleMIDIInterface.begin();     //Initializing objects
   shieldMIDIPlayer.begin();
}


void loop()
{
     // Read the MIDI message for the MIDI Wrench APP
    if (moduleMIDIInterface.isMIDIMessageOK())    
    {
        moduleMIDIInterface.getMIDIMessage(parameter); //Gets MIDI message data

        type = parameter[0];  //The first byte is the MIDI message typeThe channel is not included here(the channel is separated to the fourth byte).
        noteNumber = parameter[1]; //The second byte is the note information, see MIDI protocol manual for details
        velocity= parameter[2]; //The third byte is the strength information, see MIDI protocol manual for details
        channel = parameter[3]; //The fourth byte is the channel information
        switch(type)
        {
            case NoteOn: 
                shieldMIDIPlayer.setNoteOn(noteNumber, velocity,channel) ;
              break;
            case NoteOff :  
                shieldMIDIPlayer.setNoteOff(noteNumber, velocity,channel) ;
              break;
            default:break ;

        }
    }
}
