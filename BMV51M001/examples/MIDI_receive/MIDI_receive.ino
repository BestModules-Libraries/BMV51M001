/******************************************************************
File:             MIDI_receive.ino
Description:      Receive MIDI Messages by MIDI Interface Module(BMV51M001)
                  and switch led
Note:             -       
******************************************************************/
#include "BMV51M001.h"
BMV51M001 myMIDIInterface;
uint8_t type = 0;
uint8_t parameter[10];
void setup() 
{
         pinMode(LED_BUILTIN, OUTPUT);   //LED initial
         myMIDIInterface.begin();  //Interface initial
}
void loop() 
{
    if (myMIDIInterface. isMIDIMessageOK()) // Read the MIDI message for the MIDI Wrench APP
    {
        myMIDIInterface. getMIDIMessage(parameter); //Gets MIDI message data
        type = parameter[0];//The first byte is the MIDI message typeThe channel is not included here(the channel is separated to the fourth byte).
        switch(type)
        {
            case NoteOn: 
                digitalWrite(LED_BUILTIN, HIGH);   //LED on;
                break ;
            case NoteOff: 
                digitalWrite(LED_BUILTIN, LOW);   //LED  off    
                break ; 
            default :break ;
        }
    }
}
