/*
* Donut Hole v0.4 beta
* Copyright (C) 2025 @Donutswdad
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <SoftwareSerial.h>
#include <AltSoftSerial.h>  // https://github.com/PaulStoffregen/AltSoftSerial in order to have a 3rd Serial port for SW2
                            // Step 1 - Goto the github link above. Click the GREEN "<> Code" box and "Download ZIP"
                            // Step 2 - In Arudino IDE; goto "Sketch" -> "Include Library" -> "Add .ZIP Library"
/*
////////////////////
//    OPTIONS    //
//////////////////
*/

uint8_t const debugE1CAP = 0; // line ~281
uint8_t const debugE2CAP = 0; // line ~487

uint16_t const offset = 0; // Only needed if multiple Donut Holes, gSerial Enablers, Donut Dongles are connected. Set offset so 2nd, 3rd, etc don't overlap profiles. (e.g. offset = 100;) 

bool S0 = false;         // (Profile 0) 
                         //
                         //  ** Recommended to leave this option "false" if using in tandem with other Serial devices. **
                         //
                         // If set to "true", "S0_<user definted>.rt4" will load when all inputs are in-active on SW1 (and SW2 if connected). 

//
// For Extron Matrix switches that support DSVP. RGBS and HDMI/DVI video types.

bool const automatrixSW1 = false; // enable for auto matrix switching on "SW1" port
bool const automatrixSW2 = false; // enable for auto matrix switching on "SW2" port

int const amSizeSW1 = 8; // number of input ports for auto matrix switching on SW1. Ex: 8,12,16,32
int const amSizeSW2 = 8; // number of input ports for auto matrix switching on SW2. ...

uint8_t const vinMatrix[65] = {0,  // MATRIX switchers  // When auto matrix mode is enabled: (automatrixSW1 / SW2 above)
                                                        // set to 1 for the auto switched input to trigger a Preset on SW1
                                                        // set to 2 for the auto switched input to trigger a Preset on SW2
                                                        // set to 3 for both SW1 & SW2
                                                        // set to 0 to disable this feature (default - aka input goes to all outputs unless defined in voutMatrix)
                                                        //
                                                        // set the following inputs to the desired Preset #
                                                        // (by default each input # is set to the same corresponding Preset #)
                           1,  // input 1 SW1
                           2,  // input 2
                           3,  // input 3
                           4,  // input 4
                           5,  // input 5
                           6,  // input 6
                           7,  // input 7
                           8,  // input 8
                           9,  // input 9
                           10,  // input 10
                           11,  // input 11
                           12,  // input 12
                           13,  // input 13
                           14,  // input 14
                           15,  // input 15
                           16,  // input 16
                           17,  // input 17
                           18,  // input 18
                           19,  // input 19
                           20,  // input 20
                           21,  // input 21
                           22,  // input 22
                           23,  // input 23
                           24,  // input 24
                           25,  // input 25
                           26,  // input 26
                           27,  // input 27
                           28,  // input 28
                           29,  // input 29
                           30,  // input 30
                           31,  // input 31
                           32,  // input 32
                               //
                               // ONLY USE FOR 2ND MATRIX SWITCH on SW2
                           1,  // 2ND MATRIX SWITCH input 1 SW2
                           2,  // 2ND MATRIX SWITCH input 2
                           3,  // 2ND MATRIX SWITCH input 3
                           4,  // 2ND MATRIX SWITCH input 4
                           5,  // 2ND MATRIX SWITCH input 5
                           6,  // 2ND MATRIX SWITCH input 6
                           7,  // 2ND MATRIX SWITCH input 7
                           8,  // 2ND MATRIX SWITCH input 8
                           9,  // 2ND MATRIX SWITCH input 9
                           10,  // 2ND MATRIX SWITCH input 10
                           11,  // 2ND MATRIX SWITCH input 11
                           12,  // 2ND MATRIX SWITCH input 12
                           13,  // 2ND MATRIX SWITCH input 13
                           14,  // 2ND MATRIX SWITCH input 14
                           15,  // 2ND MATRIX SWITCH input 15
                           16,  // 2ND MATRIX SWITCH input 16
                           17,  // 2ND MATRIX SWITCH input 17
                           18,  // 2ND MATRIX SWITCH input 18
                           19,  // 2ND MATRIX SWITCH input 19
                           20,  // 2ND MATRIX SWITCH input 20
                           21,  // 2ND MATRIX SWITCH input 21
                           22,  // 2ND MATRIX SWITCH input 22
                           23,  // 2ND MATRIX SWITCH input 23
                           24,  // 2ND MATRIX SWITCH input 24
                           25,  // 2ND MATRIX SWITCH input 25
                           26,  // 2ND MATRIX SWITCH input 26
                           27,  // 2ND MATRIX SWITCH input 27
                           28,  // 2ND MATRIX SWITCH input 28
                           29,  // 2ND MATRIX SWITCH input 29
                           30,  // 2ND MATRIX SWITCH input 30
                           31,  // 2ND MATRIX SWITCH input 31
                           32,  // 2ND MATRIX SWITCH input 32
                           };
                                                        
uint8_t const voutMatrix[66] = {1,  // MATRIX switchers // When auto matrix mode is enabled: (automatrixSW1 / SW2 above)
                                                        // set to 1 for the auto switched input to go to ALL outputs (default)
                                                        // set to 0 to select outputs to be enabled/disabled as listed below
                                                        //
                                                        // When auto matrix mode is disabled: 
                                                        // ALL input changes to any/all outputs result in a profile change
                                                        // disable specific outputs from triggering profile changes
                           1,  // output 1 SW1 (1 = enabled, 0 = disabled)
                           1,  // output 2
                           1,  // output 3
                           1,  // output 4
                           1,  // output 5
                           1,  // output 6
                           1,  // output 7
                           1,  // output 8
                           1,  // output 9
                           1,  // output 10
                           1,  // output 11
                           1,  // output 12
                           1,  // output 13
                           1,  // output 14
                           1,  // output 15
                           1,  // output 16
                           1,  // output 17
                           1,  // output 18
                           1,  // output 19
                           1,  // output 20
                           1,  // output 21
                           1,  // output 22
                           1,  // output 23
                           1,  // output 24
                           1,  // output 25
                           1,  // output 26
                           1,  // output 27
                           1,  // output 28
                           1,  // output 29
                           1,  // output 30
                           1,  // output 31
                           1,  // output 32 (1 = enabled, 0 = disabled)
                               //
                               // ONLY USE FOR 2ND MATRIX SWITCH on SW2
                           1,  // 2ND MATRIX SWITCH output 1 SW2 (1 = enabled, 0 = disabled)
                           1,  // 2ND MATRIX SWITCH output 2
                           1,  // 2ND MATRIX SWITCH output 3
                           1,  // 2ND MATRIX SWITCH output 4
                           1,  // 2ND MATRIX SWITCH output 5
                           1,  // 2ND MATRIX SWITCH output 6
                           1,  // 2ND MATRIX SWITCH output 7
                           1,  // 2ND MATRIX SWITCH output 8
                           1,  // 2ND MATRIX SWITCH output 9
                           1,  // 2ND MATRIX SWITCH output 10
                           1,  // 2ND MATRIX SWITCH output 11
                           1,  // 2ND MATRIX SWITCH output 12
                           1,  // 2ND MATRIX SWITCH output 13
                           1,  // 2ND MATRIX SWITCH output 14
                           1,  // 2ND MATRIX SWITCH output 15
                           1,  // 2ND MATRIX SWITCH output 16
                           1,  // 2ND MATRIX SWITCH output 17
                           1,  // 2ND MATRIX SWITCH output 18
                           1,  // 2ND MATRIX SWITCH output 19
                           1,  // 2ND MATRIX SWITCH output 20
                           1,  // 2ND MATRIX SWITCH output 21
                           1,  // 2ND MATRIX SWITCH output 22
                           1,  // 2ND MATRIX SWITCH output 23
                           1,  // 2ND MATRIX SWITCH output 24
                           1,  // 2ND MATRIX SWITCH output 25
                           1,  // 2ND MATRIX SWITCH output 26
                           1,  // 2ND MATRIX SWITCH output 27
                           1,  // 2ND MATRIX SWITCH output 28
                           1,  // 2ND MATRIX SWITCH output 29
                           1,  // 2ND MATRIX SWITCH output 30
                           1,  // 2ND MATRIX SWITCH output 31
                           1,  // 2ND MATRIX SWITCH output 32 (1 = enabled, 0 = disabled)
                           1,  // leave set to 1
                           };
                           
////////////////////////////////////////////////////////////////////////


// SW1 software serial port -> MAX3232 TTL IC
SoftwareSerial extronSerial = SoftwareSerial(3,4); // setup a software serial port for listening to SW1. rxPin = 3 / txPin = 4

// SW2 software serial port -> MAX3232 TTL IC
AltSoftSerial extronSerial2; // setup yet another serial port for listening to SW2. hardcoded to pins D8 / D9

// Extron Global variables
String previnput[2] = {"discon","discon"}; // used to keep track of previous input
uint8_t eoutput[2]; // used to store Extron output
String const sstack = "00000000000000000000000000000000"; // static stack of 32 "0" used for comparisons
String stack1 = "00000000000000000000000000000000"; 
String stack2 = "00000000000000000000000000000000"; 
int currentInputSW1 = -1;
int currentInputSW2 = -1;
byte VERB[5] = {0x57,0x33,0x43,0x56,0x7C}; // sets matrix switch to verbose level 3

// variables used for LS Time funcions
unsigned long LScurrentTime = 0; 
unsigned long LScurrentTime2 = 0;
unsigned long LSprevTime = 0;
unsigned long LSprevTime2 = 0;

// VIKI Manual Switch Variables
unsigned long sendtimer = 0;
unsigned long sendtimer2 = 0;
unsigned long modetimer = 0;
unsigned long modetimer2 = 0;
uint8_t itestatus[] = {0,0,0};
uint8_t itestatus2[] = {0,0,0};
bool sendyes = 1;
bool sendyes2 = 1;
bool mode = 0;
bool mode2 = 0;
uint8_t iteinputnum = 0;
uint8_t iteinputnum2 = 0;

////////////////////////////////////////////////////////////////////////

void setup(){

    Serial.begin(9600); // set the baud rate for the RT4K Serial Connection
    while(!Serial){;}   // allow connection to establish before continuing
    Serial.print(F("\r")); // clear RT4K Serial buffer
    extronSerial.begin(9600); // set the baud rate for the Extron sw1 Connection
    extronSerial.setTimeout(150); // sets the timeout for reading / saving into a string
    if(automatrixSW1)extronSerial.write(VERB,5); // sets extron matrix switch to Verbose level 3
    extronSerial2.begin(9600); // set the baud rate for Extron sw2 Connection
    extronSerial2.setTimeout(150); // sets the timeout for reading / saving into a string for the Extron sw2 Connection
    if(automatrixSW2)extronSerial2.write(VERB,5); // sets extron matrix switch to Verbose level 3
    pinMode(12,INPUT_PULLUP);
    delay(100);
    if(digitalRead(12) == HIGH) S0 = true; // check state of JP1 jumper. closed jumper disables S0, cut trace / open jumper enables S0.

}

void loop(){

  readExtron1(); // also reads TESmart, MT-VIKI HDMI switches

  readExtron2(); // also reads TESmart, MT-VIKI HDMI switches

}

void readExtron1(){

    byte ecapbytes[44]; // used to store first 44 captured bytes / messages for Extron                
    String ecap = "0000000000000000000000000000000000000000"; // used to store Extron status messages for Extron in String format
    String einput = "0000000000000000000000000000000000000000"; // used to store Extron input

    if(automatrixSW1){ // if automatrixSW1 is set "true" in options, then "0LS" is sent every 250ms to see if an input has changed
      LS0time1(250);
    }

    // listens to the Extron sw1 Port for changes
    // SIS Command Responses reference - Page 77 https://media.extron.com/public/download/files/userman/XP300_Matrix_B.pdf
    if(extronSerial.available() > 0){ // if there is data available for reading, read
      extronSerial.readBytes(ecapbytes,44); // read in and store only the first 13 bytes for every status message received from 1st Extron SW port
      if(debugE1CAP){
        Serial.print(F("ecap HEX: "));
        for(int i=0;i<44;i++){
          Serial.print(ecapbytes[i],HEX);Serial.print(F(" "));
        }
        Serial.println(F("\r"));
        ecap = String((char *)ecapbytes);
        Serial.print(F("ecap ASCII: "));Serial.println(ecap);
      }
    }
    ecap = String((char *)ecapbytes); // convert bytes to String for Extron switches


    if(ecap.substring(0,3) == "Out" && !automatrixSW1){ // store only the input and output states, some Extron devices report output first instead of input
      einput = ecap.substring(6,10);
      eoutput[0] = ecap.substring(3,5).toInt();
    }
    else if(ecap.substring(0,1) == "F"){ // detect if switch has changed auto/manual states
      einput = ecap.substring(4,8);
      eoutput[0] = 65;
    }
    else if(ecap.substring(0,3) == "Rpr"){ // detect if a Preset has been used
      einput = ecap.substring(0,5);
      eoutput[0] = 65;
    }
    else if(ecap.substring(amSizeSW1 + 6,amSizeSW1 + 9) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW1 + 6,amSizeSW1 + 11);
      eoutput[0] = 65;
    }
    else if(ecap.substring(amSizeSW1 + 7,amSizeSW1 + 10) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW1 + 7,amSizeSW1 + 12);
      eoutput[0] = 65;
    }
    else if(ecap.substring(0,3) == "In0" && ecap.substring(4,7) != "All" && ecap.substring(5,8) != "All" && automatrixSW1){ // start of automatrix
      if(ecap.substring(0,4) == "In00"){
        einput = ecap.substring(5,amSizeSW1 + 5);
      }else 
        einput = ecap.substring(4,amSizeSW1 + 4);
      for(int i=0;i<amSizeSW1;i++){
        if(einput[i] != stack1[i] || einput[currentInputSW1 - 1] == '0'){ // check to see if anything changed
          stack1[i] = einput[i];
          if(einput[i] != '0'){
            currentInputSW1 = i+1;
            if(vinMatrix[0] == 1 || vinMatrix[0] == 3)
            {
              recallPreset(1,vinMatrix[currentInputSW1]);
            }
            else{
              setTie(1,currentInputSW1);
              sendSVS(currentInputSW1);
            }

          }
        }
      } //end of for loop
      if(einput.substring(0,amSizeSW1) == sstack.substring(0,amSizeSW1) 
        && stack1.substring(0,amSizeSW1) == sstack.substring(0,amSizeSW1) && currentInputSW1 != 0){ // check for all inputs being off

        currentInputSW1 = 0;
        previnput[0] = "0";
        setTie(1,currentInputSW1);

        if(S0 && (!automatrixSW2 && (previnput[1] == "0" || previnput[1] == "In0 " || previnput[1] == "In00" || previnput[1] == "discon")) 
              && (!automatrixSW2 && (previnput[1] == "discon" || voutMatrix[eoutput[1]+32]))){

          sendSVS(currentInputSW1);
        }

      }
    } // end of automatrix
    else{                             // less complex switches only report input status, no output status
      einput = ecap.substring(0,4);
      eoutput[0] = 65;
    }


    // for Extron devices, use remaining results to see which input is now active and change profile accordingly, cross-references voutMaxtrix
    if((einput.substring(0,2) == "In" && voutMatrix[eoutput[0]] && !automatrixSW1) || (einput.substring(0,3) == "Rpr")){
      if(einput.substring(0,3) == "Rpr"){
        sendSVS(einput.substring(3,5).toInt());
      }
      else if(einput != "In0 " && einput != "In00"){ // for inputs 1-99 (SVS only)
        if(einput.substring(3,4) == " ")
          sendSVS(einput.substring(2,3).toInt());
        else 
          sendSVS(einput.substring(2,4).toInt());
      }

      previnput[0] = einput;

      // Extron S0
      // when both Extron switches match In0 or In00 (no active ports), a S0 Profile can be loaded if S0 is enabled
      if(S0 && (currentInputSW2 <= 0) && ((einput == "In0 " || einput == "In00") && 
        (previnput[1] == "In0 " || previnput[1] == "In00" || previnput[1] == "discon")) && 
        voutMatrix[eoutput[0]] && (previnput[1] == "discon" || voutMatrix[eoutput[1]+32])){

          sendSVS(0);

        previnput[0] = "0";
        if(previnput[1] != "discon")previnput[1] = "0";
      
      } // end of Extron S0

      if(previnput[0] == "0" && previnput[1].substring(0,2) == "In")previnput[0] = "In00";  // changes previnput[0] "0" state to "In00" when there is a newly active input on the other switch
      if(previnput[1] == "0" && previnput[0].substring(0,2) == "In")previnput[1] = "In00";

    }

    //VIKI Manual Switch Detection (created by: https://github.com/Arthrimus)

    if(millis() - modetimer > 2000){  // Timer that disables sending SVS serial commands using the ITE mux data when there has recently been an autoswitch command (prevents duplicate commands)
      modetimer = millis(); // Resets timer to current millis() count to disable this function once the variables have been updated
      mode = 0;  // Sets mode to 0 so the ITE mux data can be used to send SVS serial commands again
      sendyes = 1; // Turns off sendyes so the SVS serial commands are not repeated if an autoswitch command preceeded the ITE commands
    }

    if(ecap.startsWith("=") && mode == 0){   // checks if the serial command from the VIKI starts with "=" This indicates that the command is an ITE mux status message
      if(ecap.substring(10,11) == "P"){        // checks if position 10 of the serial command from the VIKI contains "P" This indicates that the command is from the IT6635 chip
        itestatus[0] = ecap.substring(11,12).toInt();   // converts ITE mux status message position 10 to integer sets the itestatus array position 0 to that value
      }
      if(ecap.substring(18,20) == ">0"){       // checks if the ITE mux status message contains >0 at position 18-19
        itestatus[1] = ecap.substring(12,13).toInt();  // converts ITE mux status message position 12 to integer and updates itestatus array position 1 to that value
      }
      if(ecap.substring(18,20) == ">1"){       // checks if the ITE mux status message contains >1 at position 18-19
        itestatus[2] = ecap.substring(12,13).toInt();  // converts ITE mux status message position 12 to integer and updates itestatus array position 3 to that value
      }
      sendyes = 0;                             // sets sendyes to 0 indicating that an ITE message has been received and an SVS command can be sent once the sendtimer elapses
      sendtimer = millis();                    // resets sendtimer to millis()
      modetimer = millis();                    // resets modetimer to millis()
    }

    if((millis() - sendtimer > 300) && sendyes == 0 && mode == 0){   // Checks is all conditions are met to sendSVS based on the ITE messages
      sendtimer = millis();                     // resets sendtimer to millis()
      sendyes = 1;                              // sets sendyes to 1 to prevent the message from being resent
      if(itestatus[0] == 3){                   // Checks itestatus array position 0 to determine if port 3 of the IT6635 chip is currently selected
        if(itestatus[1] == 2) iteinputnum = 1;   // Checks itestatus array position 1 to determine if port 2 of the IT66353 DEV0 chip is selected, Sets iteinputnum to input 1
        else if(itestatus[1] == 1) iteinputnum = 2;   // Checks itestatus array position 1 to determine if port 1 of the IT66353 DEV0 chip is selected, Sets iteinputnum to input 2
        else if(itestatus[1] == 0) iteinputnum = 3;   // Checks itestatus array position 1 to determine if port 0 of the IT66353 DEV0 chip is selected, Sets iteinputnum to input 3
      }
      else if(itestatus[0] == 2){                 // Checks itestatus array position 0 to determine if port 2 of the IT6635 chip is currently selected
        if(itestatus[2] == 2) iteinputnum = 4;   // Checks itestatus array position 1 to determine if port 2 of the IT66353 DEV1 chip is selected, Sets iteinputnum to input 4
        else if(itestatus[2] == 1) iteinputnum = 5;   // Checks itestatus array position 1 to determine if port 1 of the IT66353 DEV1 chip is selected, Sets iteinputnum to input 5
        else if(itestatus[2] == 0) iteinputnum = 6;   // Checks itestatus array position 1 to determine if port 0 of the IT66353 DEV1 chip is selected, Sets iteinputnum to input 6
      }
      else if(itestatus[0] == 1) iteinputnum = 7;   // Checks itestatus array position 0 to determine if port 1 of the IT6635 chip is currently selected, Sets iteinputnum to input 7
      else if(itestatus[0] == 0) iteinputnum = 8;   // Checks itestatus array position 0 to determine if port 0 of the IT6635 chip is currently selected, Sets iteinputnum to input 8
    }

    // for TESmart 4K60 / TESmart 4K30 / MT-VIKI HDMI switch on SW1
    if(ecapbytes[4] == 17 || ecapbytes[3] == 17 || ecapbytes[4] == 95 || iteinputnum > 0){
      mode = 1;                             // Sets mode to 1 so the ITE mux data will be ignored while an autoswitch command is detected.
      modetimer = millis();                 // resets modetimer to millis()
      if(ecapbytes[6] == 22 || ecapbytes[5] == 22 || ecapbytes[11] == 48 || iteinputnum == 1){
        sendSVS(1);
      }
      else if(ecapbytes[6] == 23 || ecapbytes[5] == 23 || ecapbytes[11] == 49 || iteinputnum == 2){
        sendSVS(2);
      }
      else if(ecapbytes[6] == 24 || ecapbytes[5] == 24 || ecapbytes[11] == 50 || iteinputnum == 3){
        sendSVS(3);
      }
      else if(ecapbytes[6] == 25 || ecapbytes[5] == 25 || ecapbytes[11] == 51 || iteinputnum == 4){
        sendSVS(4);
      }
      else if(ecapbytes[6] == 26 || ecapbytes[5] == 26 || ecapbytes[11] == 52 || iteinputnum == 5){
        sendSVS(5);
      }
      else if(ecapbytes[6] == 27 || ecapbytes[5] == 27 || ecapbytes[11] == 53 || iteinputnum == 6){
        sendSVS(6);
      }
      else if(ecapbytes[6] == 28 || ecapbytes[5] == 28 || ecapbytes[11] == 54 || iteinputnum == 7){
        sendSVS(7);
      }
      else if(ecapbytes[6] == 29 || ecapbytes[5] == 29 || ecapbytes[11] == 55 || iteinputnum == 8){
        sendSVS(8);
      }
      else if(ecapbytes[6] > 29 && ecapbytes[6] < 38){
        sendSVS(ecapbytes[6] - 21);
      }
      else if(ecapbytes[5] > 29 && ecapbytes[5] < 38){
        sendSVS(ecapbytes[5] - 21);
      }
      iteinputnum = 0;                                          // Resets iteinputnum to 0 so sendSVS will not repeat after this cycle through the Void loop
    }

    // set ecapbytes to 0 for next read
    memset(ecapbytes,0,sizeof(ecapbytes)); // ecapbytes is local variable, but superstitious clearing regardless :) 


} // end of readExtron1()

void readExtron2(){
    
    byte ecapbytes[44]; // used to store first 44 captured bytes / messages for Extron                
    String ecap = "0000000000000000000000000000000000000000"; // used to store Extron status messages for Extron in String format
    String einput = "0000000000000000000000000000000000000000"; // used to store Extron input

    if(automatrixSW2){ // if automatrixSW2 is set "true" in options, then "0LS" is sent every 250ms to see if an input has changed
      LS0time2(250);
    }

    // listens to the Extron sw2 Port for changes
    if(extronSerial2.available() > 0){ // if there is data available for reading, read
      extronSerial2.readBytes(ecapbytes,44); // read in and store only the first 13 bytes for every status message received from 2nd Extron port
      if(debugE2CAP){
        Serial.print(F("ecap2 HEX: "));
        for(int i=0;i<44;i++){
          Serial.print(ecapbytes[i],HEX);Serial.print(F(" "));
        }
        Serial.println(F("\r"));
        ecap = String((char *)ecapbytes);
        Serial.print(F("ecap2 ASCII: "));Serial.println(ecap);
      }
    }
    ecap = String((char *)ecapbytes);

    if(ecap.substring(0,3) == "Out"){ // store only the input and output states, some Extron devices report output first instead of input
      einput = ecap.substring(6,10);
      eoutput[1] = ecap.substring(3,5).toInt();
    }
    else if(ecap.substring(0,1) == "F"){ // detect if switch has changed auto/manual states
      einput = ecap.substring(4,8);
      eoutput[1] = 65;
    }
    else if(ecap.substring(0,3) == "Rpr"){ // detect if a Preset has been used
      einput = ecap.substring(0,5);
      eoutput[1] = 65;
    }
    else if(ecap.substring(amSizeSW2 + 6,amSizeSW2 + 9) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW2 + 6,amSizeSW2 + 11);
      eoutput[1] = 65;
    }
    else if(ecap.substring(amSizeSW2 + 7,amSizeSW2 + 10) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW2 + 7,amSizeSW2 + 12);
      eoutput[1] = 65;
    }
    else if(ecap.substring(0,3) == "In0" && ecap.substring(4,7) != "All" && ecap.substring(5,8) != "All" && automatrixSW2){ // start of automatrix
      if(ecap.substring(0,4) == "In00"){
        einput = ecap.substring(5,amSizeSW2 + 5);
      }else 
        einput = ecap.substring(4,amSizeSW2 + 4);
      for(int i=0;i<amSizeSW2;i++){
        if(einput[i] != stack2[i] || einput[currentInputSW2 - 1] == '0'){ // check to see if anything changed
          stack2[i] = einput[i];
          if(einput[i] != '0'){
            currentInputSW2 = i+1;
            if(vinMatrix[0] == 2 || vinMatrix[0] == 3)
            {
              recallPreset(2,vinMatrix[currentInputSW2 + 32]);
            }
            else{
              setTie(2,currentInputSW2);
              sendSVS(currentInputSW2 + 100);
            }
          }
        }
      } //end of for loop
      if(einput.substring(0,amSizeSW2) == sstack.substring(0,amSizeSW2) 
        && stack2.substring(0,amSizeSW2) == sstack.substring(0,amSizeSW2) && currentInputSW2 != 0){ // check for all inputs being off
        
        currentInputSW2 = 0;
        previnput[1] = "0";
        setTie(2,currentInputSW2);  

        if(S0 && (!automatrixSW1 && (previnput[0] == "0" || previnput[0] == "In0 " || previnput[0] == "In00" || previnput[0] == "discon")) 
              && (!automatrixSW1 && (previnput[0] == "discon" || voutMatrix[eoutput[0]]))){

          sendSVS(currentInputSW2);
        }

      }
    } // end of automatrix
    else{                              // less complex switches only report input status, no output status
      einput = ecap.substring(0,4);
      eoutput[1] = 65;
    }


    // For Extron devices, use remaining results to see which input is now active and change profile accordingly, cross-references voutMaxtrix
    if((einput.substring(0,2) == "In" && voutMatrix[eoutput[1]+32] && !automatrixSW2) || (einput.substring(0,3) == "Rpr")){
      if(einput.substring(0,3) == "Rpr"){
        sendSVS(einput.substring(3,5).toInt()+100);
      }
      else if(einput != "In0 " && einput != "In00"){
        if(einput.substring(3,4) == " ") 
          sendSVS(einput.substring(2,3).toInt()+100);
        else 
          sendSVS(einput.substring(2,4).toInt()+100);
      }

      previnput[1] = einput;
      
      // Extron2 S0
      // when both Extron switches match In0 or In00 (no active ports), a Profile 0 can be loaded if S0 is enabled
      if(S0 && (currentInputSW1 <= 0) && ((einput == "In0 " || einput == "In00") && 
        (previnput[0] == "In0 " || previnput[0] == "In00" || previnput[0] == "discon")) && 
        (previnput[0] == "discon" || voutMatrix[eoutput[0]]) && voutMatrix[eoutput[1]+32]){

          sendSVS(0);

        previnput[1] = "0";
        if(previnput[0] != "discon")previnput[0] = "0";
      
      } // end of Extron2 S0

      if(previnput[0] == "0" && previnput[1].substring(0,2) == "In")previnput[0] = "In00";  // changes previnput[0] "0" state to "In00" when there is a newly active input on the other switch
      if(previnput[1] == "0" && previnput[0].substring(0,2) == "In")previnput[1] = "In00";

    }


    //VIKI Manual Switch Detection (created by: https://github.com/Arthrimus)

    if(millis() - modetimer2 > 2000){  // Timer that disables sending SVS serial commands using the ITE mux data when there has recently been an autoswitch command (prevents duplicate commands)
      modetimer2 = millis(); // Resets timer to current millis() count to disable this function once the variables hav been updated
      mode2 = 0;  // Sets mode to 0 so the ITE mux data can be used to send SVS serial commands again
      sendyes2 = 1; // Turns off sendyes so the SVS serial commands are not repeated if an autoswitch command preceeded the ITE commands
    }

    if(ecap.startsWith("=") && mode2 == 0){   // checks if the serial command from the VIKI starts with "=" This indicates that the command is an ITE mux status message
      if(ecap.substring(10,11) == "P"){       // checks if position 10 of the serial command from the VIKI contains "P" This indicates that the command is from the IT6635 chip
        itestatus2[0] = ecap.substring(11,12).toInt();   // converts ITE mux status message position 10 to integer sets the itestatus array position 0 to that value
      }
      if(ecap.substring(18,20) == ">0"){       // checks if the ITE mux status message contains >0 at position 18-19
        itestatus2[1] = ecap.substring(12,13).toInt();  // converts ITE mux status message position 12 to integer and updates itestatus array position 1 to that value
      }
      if(ecap.substring(18,20) == ">1"){       // checks if the ITE mux status message contains >1 at position 18-19
        itestatus2[2] = ecap.substring(12,13).toInt();  // converts ITE mux status message position 12 to integer and updates itestatus array position 3 to that value
      }
      sendyes2 = 0;                             // sets sendyes to 0 indicating that an ITE message has been received and an SVS command can be sent once the sendtimer elapses
      sendtimer2 = millis();                    // resets sendtimer to millis()
      modetimer2 = millis();                    // resets modetimer to millis()
    }
    if((millis() - sendtimer2 > 300) && sendyes2 == 0 && mode2 == 0){   // Checks is all conditions are met to sendSVS based on the ITE messages
      sendtimer2 = millis();                     // resets sendtimer to millis()
      sendyes2 = 1;                              // sets sendyes to 1 to prevent the message from being resent
      if(itestatus2[0] == 3){                   // Checks itestatus array position 0 to determine if port 3 of the IT6635 chip is currently selected
        if(itestatus2[1] == 2) iteinputnum2 = 1;   // Checks itestatus array position 1 to determine if port 2 of the IT66353 DEV0 chip is selected, Sets iteinputnum to input 1
        else if(itestatus2[1] == 1) iteinputnum2 = 2;   // Checks itestatus array position 1 to determine if port 1 of the IT66353 DEV0 chip is selected, Sets iteinputnum to input 2
        else if(itestatus2[1] == 0) iteinputnum2 = 3;   // Checks itestatus array position` 1 to determine if port 0 of the IT66353 DEV0 chip is selected, Sets iteinputnum to input 3
      }
      else if(itestatus2[0] == 2){                 // Checks itestatus array position 0 to determine if port 2 of the IT6635 chip is currently selected
        if(itestatus2[2] == 2) iteinputnum2 = 4;   // Checks itestatus array position 1 to determine if port 2 of the IT66353 DEV1 chip is selected, Sets iteinputnum to input 4
        else if(itestatus2[2] == 1) iteinputnum2 = 5;   // Checks itestatus array position 1 to determine if port 1 of the IT66353 DEV1 chip is selected, Sets iteinputnum to input 5
        else if(itestatus2[2] == 0) iteinputnum2 = 6;   // Checks itestatus array position 1 to determine if port 0 of the IT66353 DEV1 chip is selected, Sets iteinputnum to input 6
      }
      else if(itestatus2[0] == 1) iteinputnum2 = 7;   // Checks itestatus array position 0 to determine if port 1 of the IT6635 chip is currently selected, Sets iteinputnum to input 7
      else if(itestatus2[0] == 0) iteinputnum2 = 8;   // Checks itestatus array position 0 to determine if port 0 of the IT6635 chip is currently selected, Sets iteinputnum to input 8
    }

    // for TESmart 4K60 / TESmart 4K30 / MT-VIKI HDMI switch on SW2
    if(ecapbytes[4] == 17 || ecapbytes[3] == 17 || ecapbytes[4] == 95 || iteinputnum2 > 0){
      mode2 = 1;                             // Sets mode to 1 so the ITE mux data will be ignored while an autoswitch command is detected.
      modetimer2 = millis();                 // resets modetimer to millis()
      if(ecapbytes[6] == 22 || ecapbytes[5] == 22 || ecapbytes[11] == 48 || iteinputnum2 == 1){
        sendSVS(101);
      }
      else if(ecapbytes[6] == 23 || ecapbytes[5] == 23 || ecapbytes[11] == 49 || iteinputnum2 == 2){
        sendSVS(102);
      }
      else if(ecapbytes[6] == 24 || ecapbytes[5] == 24 || ecapbytes[11] == 50 || iteinputnum2 == 3){
        sendSVS(103);
      }
      else if(ecapbytes[6] == 25 || ecapbytes[5] == 25 || ecapbytes[11] == 51 || iteinputnum2 == 4){
        sendSVS(104);
      }
      else if(ecapbytes[6] == 26 || ecapbytes[5] == 26 || ecapbytes[11] == 52 || iteinputnum2 == 5){
        sendSVS(105);
      }
      else if(ecapbytes[6] == 27 || ecapbytes[5] == 27 || ecapbytes[11] == 53 || iteinputnum2 == 6){
        sendSVS(106);
      }
      else if(ecapbytes[6] == 28 || ecapbytes[5] == 28 || ecapbytes[11] == 54 || iteinputnum2 == 7){
        sendSVS(107);
      }
      else if(ecapbytes[6] == 29 || ecapbytes[5] == 29 || ecapbytes[11] == 55 || iteinputnum2 == 8){
        sendSVS(108);
      }
      else if(ecapbytes[6] > 29 && ecapbytes[6] < 38){
        sendSVS(ecapbytes[6] + 79);
      }
      else if(ecapbytes[5] > 29 && ecapbytes[5] < 38){
        sendSVS(ecapbytes[5] + 79);
      }
      iteinputnum2 = 0;                                           // Resets iteinputnum to 0 so sendSVS will not repeat after this cycle through the Void loop
    }

    // set ecapbytes to 0 for next read
    memset(ecapbytes,0,sizeof(ecapbytes)); // ecapbytes is local variable, but superstitious clearing regardless :) 


}// end of readExtron2()

void sendSVS(uint16_t num){
  Serial.print(F("\rSVS NEW INPUT="));
  if(num != 0)Serial.print(num + offset);
  else Serial.print(num);
  Serial.println(F("\r"));
  delay(1000);
  Serial.print(F("\rSVS CURRENT INPUT="));
  if(num != 0)Serial.print(num + offset);
  else Serial.print(num);
  Serial.println(F("\r"));
} // end of sendSVS()

void LS0time1(unsigned long eTime){
  LScurrentTime = millis();  // Init timer
  if(LSprevTime == 0)       // If previous timer not initialized, do so now.
    LSprevTime = millis();
  if((LScurrentTime - LSprevTime) >= eTime){ // If it's been longer than eTime, send "0LS" and reset the timer.
    LScurrentTime = 0;
    LSprevTime = 0;
    extronSerial.print("0LS");
    delay(20);
 }
}  // end of LS0time1()

void LS0time2(unsigned long eTime){
  LScurrentTime2 = millis();  // Init timer
  if(LSprevTime2 == 0)       // If previous timer not initialized, do so now.
    LSprevTime2 = millis();
  if((LScurrentTime2 - LSprevTime2) >= eTime){ // If it's been longer than eTime, send "0LS" and reset the timer.
    LScurrentTime2 = 0;
    LSprevTime2 = 0;
    extronSerial2.print("0LS");
    delay(20);
 }
}  // end of LS0time2()

void setTie(uint8_t sw, uint8_t num){
  if(sw == 1){
    if(voutMatrix[0]){
      extronSerial.print(num);
      extronSerial.print(F("*"));
      extronSerial.print(F("!"));
    }
    else{
      for(int i=1;i<(amSizeSW1 + 1);i++){
        if(voutMatrix[i]){
          extronSerial.print(num);
          extronSerial.print(F("*"));
          extronSerial.print(i);
          extronSerial.print(F("!"));
        }
      }
    }
  }
  else if(sw == 2){
    if(voutMatrix[0]){
      extronSerial2.print(num);
      extronSerial2.print(F("*"));
      extronSerial2.print(F("!"));
    }
    else{
      for(int i=33;i<(amSizeSW2 + 33);i++){
        if(voutMatrix[i]){
          extronSerial2.print(num);
          extronSerial2.print(F("*"));
          extronSerial2.print(i - 32);
          extronSerial2.print(F("!"));
        }
      }
    }
  }
} // end of setTie()

void recallPreset(uint8_t sw, uint8_t num){
  if(sw == 1){
    extronSerial.print(num);
    extronSerial.print(F("."));
  }
  else if(sw == 2){
    extronSerial2.print(num);
    extronSerial2.print(F("."));
  }
} // end of recallPreset()
