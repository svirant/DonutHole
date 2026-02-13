/*
* Donut Hole beta v0.6d
* Copyright (C) 2026 @Donutswdad
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

#define EXTRON1 0
#define EXTRON2 1

struct profileorder {
  int Prof;
  uint8_t On;
  uint8_t King;
};

profileorder mswitch[2] = {{0,0,0},{0,0,0}};
uint8_t mswitchSize = 2;

/*
////////////////////
//    OPTIONS    //
//////////////////
*/

uint8_t const debugE1CAP = 0; // line ~239
uint8_t const debugE2CAP = 0; // line ~479

uint16_t const offset = 0; // Only needed if multiple Donut Holes, gSerial Enablers, Donut Dongles are connected. Set offset so 2nd, 3rd, etc don't overlap profiles. (e.g. offset = 100;) 

bool S0 = false;   // (Profile 0) default is false
                         //  ** Recommended to leave this option "false" if using in tandem with other Serial devices. **
                         // If set to "true", "S0_<user definted>.rt4" will load when all inputs are in-active on SW1 (and SW2 if connected). 
                         // ** Does not work with MT-VIKI / TESmart HDMI switches **

// For Extron Matrix switches that support DSVP. RGBS and HDMI/DVI video types.
#define automatrixSW1 false // set true for auto matrix switching on "SW1" port
#define automatrixSW2 false // set true for auto matrix switching on "SW2" port

uint8_t ExtronVideoOutputPortSW1 = 1; // For older (E-series,non Plus/Ultra) Extron Matrix models, must specify the video output port that connects to RT4K
uint8_t ExtronVideoOutputPortSW2 = 1; 

uint8_t const vinMatrix[] = {0,  // MATRIX switchers  // When auto matrix mode is enabled: (automatrixSW1 / SW2 defined above)
                                                        // set to 0 for the auto switched input to tie to all outputs
                                                        // set to 1 for the auto switched input to trigger a Preset
                                                        // set to 2 for the auto switched input to tie to "ExtronVideoOutputPortSW1" / "ExtronVideoOutputPortSW2"
                                                        //
                                                        // For option 1, set the following inputs to the desired Preset #
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
                           30,  // input 31
                           30,  // input 32
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
                           30,  // 2ND MATRIX SWITCH input 31
                           30,  // 2ND MATRIX SWITCH input 32
                           };

                           
////////////////////////////////////////////////////////////////////////

// automatrix variables
#if automatrixSW1 || automatrixSW2
uint8_t AMstate[32];
uint32_t prevAMstate = 0;
int  AMstateTop = -1;
uint8_t amSizeSW1 = 8; // 8 by default, but updates if a different size is discovered
uint8_t amSizeSW2 = 8; // ...

#endif

// SW1 software serial port -> MAX3232 TTL IC
SoftwareSerial extronSerial = SoftwareSerial(3,4); // setup a software serial port for listening to SW1. rxPin = 3 / txPin = 4

// SW2 software serial port -> MAX3232 TTL IC
AltSoftSerial extronSerial2; // setup yet another serial port for listening to SW2. hardcoded to pins D8 / D9

// Extron Global variables
uint8_t eoutput[2]; // used to store Extron output
int currentInputSW1 = -1;
int currentInputSW2 = -1;
int currentProf = 0;
String ecap = "00000000000000000000000000000000000000000000"; // used to store Extron status messages for Extron in String format
String einput = "000000000000000000000000000000000000"; // used to store Extron input
byte ecapbytes[44] = {0}; // used to store first 44 bytes / messages for Extron capture
byte const VERB[5] = {0x57,0x33,0x43,0x56,0x7C}; // sets matrix switch to verbose level 3

// MT-VIKI serial command
byte viki[4] = {0xA5,0x5A,0x00,0xCC};

// LS Time variables
unsigned long LScurrentTime = 0; 
unsigned long LScurrentTime2 = 0;
unsigned long LSprevTime = 0;
unsigned long LSprevTime2 = 0;

#if !automatrixSW1
// MT-VIKI Time variables
unsigned long MTVcurrentTime = 0; 
unsigned long MTVprevTime = 0;
unsigned long sendtimer = 0;
unsigned long ITEtimer = 0;
#endif

#if !automatrixSW2
// MT-VIKI Time variables
unsigned long MTVcurrentTime2 = 0;
unsigned long MTVprevTime2 = 0;
unsigned long sendtimer2 = 0;
unsigned long ITEtimer2 = 0;
#endif

// VIKI Manual Switch variables
uint8_t ITEstatus[] = {3,0,0};
uint8_t ITEstatus2[] = {3,0,0};
bool ITErecv[2] = {0,0};
bool listenITE[2] = {1,1};
uint8_t ITEinputnum[2] = {0,0};
uint8_t currentMTVinput[2] = {0,0};
bool MTVdiscon[2] = {false,false};
bool MTVddSW1 = false;
bool MTVddSW2 = false;

////////////////////////////////////////////////////////////////////////

void setup(){

  Serial.begin(9600); // set the baud rate for the RT4K Serial Connection
  while(!Serial){;}   // allow connection to establish before continuing
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

  #if automatrixSW1 // if automatrixSW1 is set "true" in options, then "0LS" is sent every 500ms to see if an input has changed
      LS0time1(500);
  #endif

  #if !automatrixSW1
    if(MTVddSW1){            // if a MT-VIKI switch has been detected on SW1, then the currently active MT-VIKI hdmi port is checked for disconnection
      MTVtime1(1500);
    }
  #endif

    // listens to the Extron sw1 Port for changes
    // SIS Command Responses reference - Page 77 https://media.extron.com/public/download/files/userman/XP300_Matrix_B.pdf
    if(extronSerial.available() > 0){ // if there is data available for reading, read
      extronSerial.readBytes(ecapbytes,44); // read in and store only the first 44 bytes for every status message received from 1st Extron SW port
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
    if(!debugE1CAP) ecap = String((char *)ecapbytes); // convert bytes to String for Extron switches


    if((ecap.substring(0,3) == "Out" || ecap.substring(0,3) == "OUT") && !automatrixSW1){ // store only the input and output states, some Extron devices report output first instead of input
      if(ecap.substring(4,5) == " "){
        einput = ecap.substring(5,9);
        if(ecap.substring(3,4).toInt() == ExtronVideoOutputPortSW1) eoutput[0] = 1;
        else eoutput[0] = 0;
      }
      else{
        einput = ecap.substring(6,10);
        if(ecap.substring(3,5).toInt() == ExtronVideoOutputPortSW1) eoutput[0] = 1;
        else eoutput[0] = 0;
      }
    }
    else if(ecap.substring(0,1) == "F"){ // detect if switch has changed auto/manual states
      einput = ecap.substring(4,8);
      eoutput[0] = 1;
    }
    else if(ecap.substring(0,3) == "Rpr"){ // detect if a Preset has been used
      einput = ecap.substring(0,5);
      eoutput[0] = 1;
    }
    else if(ecap.substring(0,8) == "RECONFIG"){      // This is received everytime a change is made on older Extron Crosspoints
      ExtronOutputQuery(ExtronVideoOutputPortSW1,1); // Read current input for "ExtronVideoOutputPortSW1" that is connected to port 1 of the DD
    }
#if automatrixSW1
    else if(ecap.substring(amSizeSW1 + 6,amSizeSW1 + 9) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW1 + 6,amSizeSW1 + 11);
      eoutput[0] = 1;
    }
    else if(ecap.substring(amSizeSW1 + 7,amSizeSW1 + 10) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW1 + 7,amSizeSW1 + 12);
      eoutput[0] = 1;
    }
    else if(ecap.substring(0,3) == "In0" && ecap.substring(4,7) != "All" && ecap.substring(5,8) != "All"){ // start of automatrix
      if(ecap.substring(0,4) == "In00"){
        amSizeSW1 = ecap.length() - 7;
        einput = ecap.substring(5,amSizeSW1 + 5);
      }
      else{
        amSizeSW1 = ecap.length() - 6;
        einput = ecap.substring(4,amSizeSW1 + 4);
      }
      uint8_t check = readAMstate(einput,amSizeSW1);
      if(check != currentInputSW1){
        currentInputSW1 = check;
        if(currentInputSW1 == 0){
          setTie(currentInputSW1,1);
          sendProfile(0,EXTRON1,1);
        }
        else if(vinMatrix[0] == 1){
          recallPreset(vinMatrix[currentInputSW1],1);
        }
        else if(vinMatrix[0] == 0 || vinMatrix[0] == 2){
          setTie(currentInputSW1,1);          
          sendProfile(currentInputSW1,EXTRON1,1);
        }
      }
    } // end of automatrix
#endif
    else{                             // less complex switches only report input status, no output status
      einput = ecap.substring(0,4);
      eoutput[0] = 1;
    }

    // For older Extron Crosspoints, where "RECONFIG" is sent when changes are made, the profile is only changed when a different input is selected for the defined output. (ExtronVideoOutputPortSW1)
    // Without this, the profile would be resent when changes to other outputs are selected.
    if(einput.substring(0,2) == "IN" && einput.substring(2,4).toInt() == currentProf) einput = "XX00";

    // for Extron devices, use remaining results to see which input is now active and change profile accordingly, cross-references eoutput[0]
    if(((einput.substring(0,2) == "In" || einput.substring(0,2) == "IN") && eoutput[0] && !automatrixSW1) || (einput.substring(0,3) == "Rpr")){
      if(einput.substring(0,3) == "Rpr"){
        sendProfile(einput.substring(3,5).toInt(),EXTRON1,1);
      }
      else if(einput != "IN0 " && einput != "In0 " && einput != "In00"){ // for inputs 1-99 (SVS only)
        sendProfile(einput.substring(2,4).toInt(),EXTRON1,1);
      }
      else if(einput == "IN0" || einput == "In0 " || einput == "In00"){
        sendProfile(0,EXTRON1,1);
      }
    }

#if !automatrixSW1
    // VIKI Manual Switch Detection (created by: https://github.com/Arthrimus)
    // ** hdmi output must be connected when powering on switch for ITE messages to appear, thus manual button detection working **

    if(millis() - ITEtimer > 1200){  // Timer that re-enables sending SVS serial commands using the ITE mux data after an autoswitch command (prevents duplicate commands)
      listenITE[0] = 1;  // Sets listenITE[0] to 1 so the ITE mux data can be used to send SVS serial commands again
      ITErecv[0] = 0; // Turns off ITErecv[0] so the SVS serial commands are not repeated if an autoswitch command preceeded the ITE commands
      ITEtimer = millis(); // Resets timer to current millis() count to disable this function once the variables have been updated
    }


    if((ecap.substring(0,3) == "==>" || ecap.substring(15,18) == "==>") && listenITE[0]){   // checks if the serial command from the VIKI starts with "==>" This indicates that the command is an ITE mux status message
      if(ecap.substring(10,11) == "P"){        // checks the last value of the IT6635 mux. P3 points to inputs 1,2,3 / P2 points to inputs 4,5,6 / P1 input 7 / P0 input 8
        ITEstatus[0] = ecap.substring(11,12).toInt();
      }
      if(ecap.substring(25,26) == "P"){        // checks the last value of the IT6635 mux. P3 points to inputs 1,2,3 / P2 points to inputs 4,5,6 / P1 input 7 / P0 input 8
        ITEstatus[0] = ecap.substring(26,27).toInt();
      }
      if(ecap.substring(18,20) == ">0"){       // checks the value of the IT66535 IC that points to Dev->0. P2 is input 1 / P1 is input 2 / P0 is input 3
        ITEstatus[1] = ecap.substring(12,13).toInt();
      }
      if(ecap.substring(33,35) == ">0"){       // checks the value of the IT66535 IC that points to Dev->0. P2 is input 1 / P1 is input 2 / P0 is input 3
        ITEstatus[1] = ecap.substring(27,28).toInt();
      }
      if(ecap.substring(18,20) == ">1"){       // checks the value of the IT66535 IC that points to Dev->1. P2 is input 4 / P1 is input 5 / P0 is input 6
        ITEstatus[2] = ecap.substring(12,13).toInt();
      }
      if(ecap.substring(33,35) == ">1"){       // checks the value of the IT66535 IC that points to Dev->1. P2 is input 4 / P1 is input 5 / P0 is input 6
        ITEstatus[2] = ecap.substring(27,28).toInt();
      }

      ITErecv[0] = 1;                            // sets ITErecv[0] to 1 indicating that an ITE message has been received and an SVS command can be sent once the sendtimer elapses
      sendtimer = millis();                   // resets sendtimer to millis() 
      ITEtimer = millis();                    // resets ITEtimer to millis()
      MTVprevTime = millis();                 // delays disconnection detection timer so it wont interrupt
    }


    if((millis() - sendtimer > 300) && ITErecv[0]){ // wait 300ms to make sure all ITE messages are received in order to complete ITEstatus
      if(ITEstatus[0] == 3){                   // Checks if port 3 of the IT6635 chip is currently selected
        if(ITEstatus[1] == 2) ITEinputnum[0] = 1;   // Checks if port 2 of the IT66353 DEV0 chip is selected, Sets ITEinputnum[0] to input 1
        else if(ITEstatus[1] == 1) ITEinputnum[0] = 2;   // Checks if port 1 of the IT66353 DEV0 chip is selected, Sets ITEinputnum[0] to input 2
        else if(ITEstatus[1] == 0) ITEinputnum[0] = 3;   // Checks if port 0 of the IT66353 DEV0 chip is selected, Sets ITEinputnum[0] to input 3
      }
      else if(ITEstatus[0] == 2){                 // Checks if port 2 of the IT6635 chip is currently selected
        if(ITEstatus[2] == 2) ITEinputnum[0] = 4;   // Checks if port 2 of the IT66353 DEV1 chip is selected, Sets ITEinputnum[0] to input 4
        else if(ITEstatus[2] == 1) ITEinputnum[0] = 5;   // Checks if port 1 of the IT66353 DEV1 chip is selected, Sets ITEinputnum[0] to input 5
        else if(ITEstatus[2] == 0) ITEinputnum[0] = 6;   // Checks if port 0 of the IT66353 DEV1 chip is selected, Sets ITEinputnum[0] to input 6
      }
      else if(ITEstatus[0] == 1) ITEinputnum[0] = 7;   // Checks if port 1 of the IT6635 chip is currently selected, Sets ITEinputnum[0] to input 7
      else if(ITEstatus[0] == 0) ITEinputnum[0] = 8;   // Checks if port 0 of the IT6635 chip is currently selected, Sets ITEinputnum[0] to input 8
      
      ITErecv[0] = 0;                              // Turns off ITErecv[0] so the SVS serial commands are not repeated if an autoswitch command preceeded the ITE commands
      sendtimer = millis();                     // resets sendtimer to millis()
    }

    if(ecap.substring(0,5) == "Auto_" || ecap.substring(15,20) == "Auto_" || ITEinputnum[0] > 0) MTVddSW1 = true; // enable MT-VIKI disconnection detection if MT-VIKI switch is present

    // for TESmart 4K60 / TESmart 4K30 / MT-VIKI HDMI switch on SW1
    if(ecapbytes[4] == 17 || ecapbytes[3] == 17 || ecap.substring(0,5) == "Auto_" || ecap.substring(15,20) == "Auto_" || ITEinputnum[0] > 0){
      if(ecapbytes[6] == 22 || ecapbytes[5] == 22 || ecapbytes[11] == 48 || ecapbytes[26] == 48 || ITEinputnum[0] == 1){
        sendProfile(1,EXTRON1,1);
        currentMTVinput[0] = 1;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] == 23 || ecapbytes[5] == 23 || ecapbytes[11] == 49 || ecapbytes[26] == 49 || ITEinputnum[0] == 2){
        sendProfile(2,EXTRON1,1);
        currentMTVinput[0] = 2;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] == 24 || ecapbytes[5] == 24 || ecapbytes[11] == 50 || ecapbytes[26] == 50 || ITEinputnum[0] == 3){
        sendProfile(3,EXTRON1,1);
        currentMTVinput[0] = 3;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] == 25 || ecapbytes[5] == 25 || ecapbytes[11] == 51 || ecapbytes[26] == 51 || ITEinputnum[0] == 4){
        sendProfile(4,EXTRON1,1);
        currentMTVinput[0] = 4;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] == 26 || ecapbytes[5] == 26 || ecapbytes[11] == 52 || ecapbytes[26] == 52 || ITEinputnum[0] == 5){
        sendProfile(5,EXTRON1,1);
        currentMTVinput[0] = 5;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] == 27 || ecapbytes[5] == 27 || ecapbytes[11] == 53 || ecapbytes[26] == 53 || ITEinputnum[0] == 6){
        sendProfile(6,EXTRON1,1);
        currentMTVinput[0] = 6;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] == 28 || ecapbytes[5] == 28 || ecapbytes[11] == 54 || ecapbytes[26] == 54 || ITEinputnum[0] == 7){
        sendProfile(7,EXTRON1,1);
        currentMTVinput[0] = 7;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] == 29 || ecapbytes[5] == 29 || ecapbytes[11] == 55 || ecapbytes[26] == 55 || ITEinputnum[0] == 8){
        sendProfile(8,EXTRON1,1);
        currentMTVinput[0] = 8;
        MTVdiscon[0] = false;
      }
      else if(ecapbytes[6] > 29 && ecapbytes[6] < 38){
        sendProfile(ecapbytes[6] - 21,EXTRON1,1);
      }
      else if(ecapbytes[5] > 29 && ecapbytes[5] < 38){
        sendProfile(ecapbytes[5] - 21,EXTRON1,1);
      }

      if(ecap.substring(0,5) == "Auto_" || ecap.substring(15,20) == "Auto_") listenITE[0] = 0; // Sets listenITE[0] to 0 so the ITE mux data will be ignored while an autoswitch command is detected.
      ITEinputnum[0] = 0;                     // Resets ITEinputnum[0] to 0 so sendSVS will not repeat after this cycle through the void loop
      ITEtimer = millis();                 // resets ITEtimer to millis()
      MTVprevTime = millis();              // delays disconnection detection timer so it wont interrupt
     }

    
    // if a MT-VIKI active port disconnection is detected, and then later a reconnection, resend the profile.
    if(ecap.substring(24,41) == "IS_NON_INPUT_PORT"){
      if(!MTVdiscon[0]) sendProfile(0,EXTRON1,0);
      MTVdiscon[0] = true;
    }
    else if(ecap.substring(24,41) != "IS_NON_INPUT_PORT" && ecap.substring(0,11) == "Uart_RxData" && MTVdiscon[0]){
      MTVdiscon[0] = false;
      sendProfile(currentMTVinput[0],EXTRON1,1);
    }
#endif

    memset(ecapbytes,0,sizeof(ecapbytes)); // reset capture to all 0s
    ecap = "00000000000000000000000000000000000000000000";
    einput = "000000000000000000000000000000000000";

} // end of readExtron1()

void readExtron2(){
    
#if automatrixSW2 // if automatrixSW2 is set "true" in options, then "0LS" is sent every 500ms to see if an input has changed
      LS0time2(500);
#endif

#if !automatrixSW2
    if(MTVddSW2){            // if a MT-VIKI switch has been detected on SW2, then the currently active MT-VIKI hdmi port is checked for disconnection
      MTVtime2(1500);
    }
#endif


    // listens to the Extron sw2 Port for changes
    if(extronSerial2.available() > 0){ // if there is data available for reading, read
      extronSerial2.readBytes(ecapbytes,44); // read in and store only the first 44 bytes for every status message received from 2nd Extron port
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
    if(!debugE2CAP) ecap = String((char *)ecapbytes);


    if((ecap.substring(0,3) == "Out" || ecap.substring(0,3) == "OUT") && !automatrixSW2){ // store only the input and output states, some Extron devices report output first instead of input
      if(ecap.substring(4,5) == " "){
        einput = ecap.substring(5,9);
        if(ecap.substring(3,4).toInt() == ExtronVideoOutputPortSW2) eoutput[1] = 1;
        else eoutput[1] = 0;
      }
      else{
        einput = ecap.substring(6,10);
        if(ecap.substring(3,5).toInt() == ExtronVideoOutputPortSW2) eoutput[1] = 1;
        else eoutput[1] = 0;
      }
    }
    else if(ecap.substring(0,1) == "F"){ // detect if switch has changed auto/manual states
      einput = ecap.substring(4,8);
      eoutput[1] = 1;
    }
    else if(ecap.substring(0,3) == "Rpr"){ // detect if a Preset has been used
      einput = ecap.substring(0,5);
      eoutput[1] = 1;
    }
    else if(ecap.substring(0,8) == "RECONFIG"){     // This is received everytime a change is made on older Extron Crosspoints
      ExtronOutputQuery(ExtronVideoOutputPortSW2,2); // Read current input for "ExtronVideoOutputPortSW2" that is connected to port 2 of the DD
    }
#if automatrixSW2    
    else if(ecap.substring(amSizeSW2 + 6,amSizeSW2 + 9) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW2 + 6,amSizeSW2 + 11);
      eoutput[1] = 1;
    }
    else if(ecap.substring(amSizeSW2 + 7,amSizeSW2 + 10) == "Rpr"){ // detect if a Preset has been used 
      einput = ecap.substring(amSizeSW2 + 7,amSizeSW2 + 12);
      eoutput[1] = 1;
    }
    else if(ecap.substring(0,3) == "In0" && ecap.substring(4,7) != "All" && ecap.substring(5,8) != "All"){ // start of automatrix
      if(ecap.substring(0,4) == "In00"){
        amSizeSW2 = ecap.length() - 7;
        einput = ecap.substring(5,amSizeSW2 + 5);
      }
      else{
        amSizeSW2 = ecap.length() - 6;
        einput = ecap.substring(4,amSizeSW2 + 4);
      }
      uint8_t check2 = readAMstate(einput,amSizeSW2);
      if(check2 != currentInputSW2){
        currentInputSW2 = check2;
        if(currentInputSW2 == 0){
          setTie(currentInputSW2,2);
          sendProfile(0,EXTRON2,1);
        }
        else if(vinMatrix[0] == 1){
          recallPreset(vinMatrix[currentInputSW2 + 32],2);
        }
        else if(vinMatrix[0] == 0 || vinMatrix[0] == 2){
          setTie(currentInputSW2,2);          
          sendProfile(currentInputSW2 + 100,EXTRON2,1);
        }
      }
    } // end of automatrix
#endif
    else{                              // less complex switches only report input status, no output status
      einput = ecap.substring(0,4);
      eoutput[1] = 1;
    }

    // For older Extron Crosspoints, where "RECONFIG" is sent when changes are made, the profile is only changed when a different input is selected for the defined output. (ExtronVideoOutputPortSW2)
    // Without this, the profile would be resent when changes to other outputs are selected.
    if(einput.substring(0,2) == "IN" && einput.substring(2,4).toInt()+100 == currentProf) einput = "XX00";

    // For Extron devices, use remaining results to see which input is now active and change profile accordingly, cross-references eoutput[1]
    if(((einput.substring(0,2) == "In" || einput.substring(0,2) == "IN") && eoutput[1] && !automatrixSW2) || (einput.substring(0,3) == "Rpr")){
      if(einput.substring(0,3) == "Rpr"){
        sendProfile(einput.substring(3,5).toInt()+100,EXTRON2,1);
      }
      else if(einput != "IN0" && einput != "In0 " && einput != "In00"){
        sendProfile(einput.substring(2,4).toInt()+100,EXTRON2,1);
      }
      else if(einput == "IN0" || einput == "In0 " || einput == "In00"){
        sendProfile(0,EXTRON2,1);
      }

    }


#if !automatrixSW2
    // VIKI Manual Switch Detection (created by: https://github.com/Arthrimus)
    // ** hdmi output must be connected when powering on switch for ITE messages to appear, thus manual button detection working **

    if(millis() - ITEtimer2 > 1200){  // Timer that re-enables sending SVS serial commands using the ITE mux data after an autoswitch command (prevents duplicate commands)
      listenITE[1] = 1;  // Sets listenITE[1] to 1 so the ITE mux data can be used to send SVS serial commands again
      ITErecv[1] = 0; // Turns off ITErecv[1] so the SVS serial commands are not repeated if an autoswitch command preceeded the ITE commands
      ITEtimer2 = millis(); // Resets timer to current millis() count to disable this function once the variables hav been updated
    }


    if((ecap.substring(0,3) == "==>" || ecap.substring(15,18) == "==>") && listenITE[1]){   // checks if the serial command from the VIKI starts with "==>" This indicates that the command is an ITE mux status message
      if(ecap.substring(10,11) == "P"){       // checks the last value of the IT6635 mux. P3 points to inputs 1,2,3 / P2 points to inputs 4,5,6 / P1 input 7 / P0 input 8
        ITEstatus2[0] = ecap.substring(11,12).toInt();
      }
      if(ecap.substring(25,26) == "P"){       // checks the last value of the IT6635 mux. P3 points to inputs 1,2,3 / P2 points to inputs 4,5,6 / P1 input 7 / P0 input 8
        ITEstatus2[0] = ecap.substring(26,27).toInt();
      }
      if(ecap.substring(18,20) == ">0"){       // checks the value of the IT66535 IC that points to Dev->0. P2 is input 1 / P1 is input 2 / P0 is input 3
        ITEstatus2[1] = ecap.substring(12,13).toInt();
      }
      if(ecap.substring(33,35) == ">0"){       // checks the value of the IT66535 IC that points to Dev->0. P2 is input 1 / P1 is input 2 / P0 is input 3
        ITEstatus2[1] = ecap.substring(27,28).toInt();
      }
      if(ecap.substring(18,20) == ">1"){       // checks the value of the IT66535 IC that points to Dev->1. P2 is input 4 / P1 is input 5 / P0 is input 6
        ITEstatus2[2] = ecap.substring(12,13).toInt();
      }
      if(ecap.substring(33,35) == ">1"){       // checks the value of the IT66535 IC that points to Dev->1. P2 is input 4 / P1 is input 5 / P0 is input 6
        ITEstatus2[2] = ecap.substring(27,28).toInt();
      }
      ITErecv[1] = 1;                             // sets ITErecv[1] to 1 indicating that an ITE message has been received and an SVS command can be sent once the sendtimer elapses
      sendtimer2 = millis();                    // resets sendtimer2 to millis()
      ITEtimer2 = millis();                    // resets ITEtimer2 to millis()
      MTVprevTime2 = millis();                 // delays disconnection detection timer so it wont interrupt
    }


    if((millis() - sendtimer2 > 300) && ITErecv[1]){ // wait 300ms to make sure all ITE messages are received in order to complete ITEstatus
      if(ITEstatus2[0] == 3){                   // Checks if port 3 of the IT6635 chip is currently selected
        if(ITEstatus2[1] == 2) ITEinputnum[1] = 1;   // Checks if port 2 of the IT66353 DEV0 chip is selected, Sets ITEinputnum to input 1
        else if(ITEstatus2[1] == 1) ITEinputnum[1] = 2;   // Checks if port 1 of the IT66353 DEV0 chip is selected, Sets ITEinputnum to input 2
        else if(ITEstatus2[1] == 0) ITEinputnum[1] = 3;   // Checks if port 0 of the IT66353 DEV0 chip is selected, Sets ITEinputnum to input 3
      }
      else if(ITEstatus2[0] == 2){                 // Checks if port 2 of the IT6635 chip is currently selected
        if(ITEstatus2[2] == 2) ITEinputnum[1] = 4;   // Checks if port 2 of the IT66353 DEV1 chip is selected, Sets ITEinputnum to input 4
        else if(ITEstatus2[2] == 1) ITEinputnum[1] = 5;   // Checks if port 1 of the IT66353 DEV1 chip is selected, Sets ITEinputnum to input 5
        else if(ITEstatus2[2] == 0) ITEinputnum[1] = 6;   // Checks if port 0 of the IT66353 DEV1 chip is selected, Sets ITEinputnum to input 6
      }
      else if(ITEstatus2[0] == 1) ITEinputnum[1] = 7;   // Checks if port 1 of the IT6635 chip is currently selected, Sets ITEinputnum to input 7
      else if(ITEstatus2[0] == 0) ITEinputnum[1] = 8;   // Checks if port 0 of the IT6635 chip is currently selected, Sets ITEinputnum to input 8

      ITErecv[1] = 0;                              // sets ITErecv[1] to 0 to prevent the message from being resent
      sendtimer2 = millis();                     // resets sendtimer2 to millis()
    }

    if(ecap.substring(0,5) == "Auto_" || ecap.substring(15,20) == "Auto_" || ITEinputnum[1] > 0) MTVddSW2 = true; // enable MT-VIKI disconnection detection if MT-VIKI switch is present

    // for TESmart 4K60 / TESmart 4K30 / MT-VIKI HDMI switch on SW2
    if(ecapbytes[4] == 17 || ecapbytes[3] == 17 || ecap.substring(0,5) == "Auto_" || ecap.substring(15,20) == "Auto_" || ITEinputnum[1] > 0){
      if(ecapbytes[6] == 22 || ecapbytes[5] == 22 || ecapbytes[11] == 48 || ecapbytes[26] == 48 || ITEinputnum[1] == 1){
        sendProfile(101,EXTRON2,1);
        currentMTVinput[1] = 101;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] == 23 || ecapbytes[5] == 23 || ecapbytes[11] == 49 || ecapbytes[26] == 49 || ITEinputnum[1] == 2){
        sendProfile(102,EXTRON2,1);
        currentMTVinput[1] = 102;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] == 24 || ecapbytes[5] == 24 || ecapbytes[11] == 50 || ecapbytes[26] == 50 || ITEinputnum[1] == 3){
        sendProfile(103,EXTRON2,1);
        currentMTVinput[1] = 103;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] == 25 || ecapbytes[5] == 25 || ecapbytes[11] == 51 || ecapbytes[26] == 51 || ITEinputnum[1] == 4){
        sendProfile(104,EXTRON2,1);
        currentMTVinput[1] = 104;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] == 26 || ecapbytes[5] == 26 || ecapbytes[11] == 52 || ecapbytes[26] == 52 || ITEinputnum[1] == 5){
        sendProfile(105,EXTRON2,1);
        currentMTVinput[1] = 105;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] == 27 || ecapbytes[5] == 27 || ecapbytes[11] == 53 || ecapbytes[26] == 53 || ITEinputnum[1] == 6){
        sendProfile(106,EXTRON2,1);
        currentMTVinput[1] = 106;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] == 28 || ecapbytes[5] == 28 || ecapbytes[11] == 54 || ecapbytes[26] == 54 || ITEinputnum[1] == 7){
        sendProfile(107,EXTRON2,1);
        currentMTVinput[1] = 107;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] == 29 || ecapbytes[5] == 29 || ecapbytes[11] == 55 || ecapbytes[26] == 55 || ITEinputnum[1] == 8){
        sendProfile(108,EXTRON2,1);
        currentMTVinput[1] = 108;
        MTVdiscon[1] = false;
      }
      else if(ecapbytes[6] > 29 && ecapbytes[6] < 38){
        sendProfile(ecapbytes[6] + 79,EXTRON2,1);
      }
      else if(ecapbytes[5] > 29 && ecapbytes[5] < 38){
        sendProfile(ecapbytes[5] + 79,EXTRON2,1);
      }

      if(ecap.substring(0,5) == "Auto_" || ecap.substring(15,20) == "Auto_") listenITE[1] = 0; // Sets listenITE[1] to 0 so the ITE mux data will be ignored while an autoswitch command is detected.
      ITEinputnum[1] = 0;                     // Resets ITEinputnum to 0 so sendSVS will not repeat after this cycle through the void loop
      ITEtimer2 = millis();                 // resets ITEtimer to millis()
      MTVprevTime2 = millis();              // delays disconnection detection timer so it wont interrupt
    }

    // if a MT-VIKI active port disconnection is detected, and then later a reconnection, resend the profile.
    if(ecap.substring(24,41) == "IS_NON_INPUT_PORT"){
      if(!MTVdiscon[1]) sendProfile(0,EXTRON2,0);
      MTVdiscon[1] = true;
    }
    else if(ecap.substring(24,41) != "IS_NON_INPUT_PORT" && ecap.substring(0,11) == "Uart_RxData" && MTVdiscon[1]){
      MTVdiscon[1] = false;
      sendProfile(currentMTVinput[1],EXTRON2,1);
    }
#endif

    memset(ecapbytes,0,sizeof(ecapbytes)); // reset capture to 0s
    ecap = "00000000000000000000000000000000000000000000";
    einput = "000000000000000000000000000000000000";

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
  currentProf = num;
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

void setTie(uint8_t num, uint8_t sw){
  if(sw == 1){
    if(vinMatrix[0] == 0){
      extronSerial.print(num);
      extronSerial.print(F("*"));
      extronSerial.print(F("!"));
    }
    else if(vinMatrix[0] == 2){
      extronSerial.print(num);
      extronSerial.print(F("*"));
      extronSerial.print(ExtronVideoOutputPortSW1);
      extronSerial.print(F("!"));
    }
  }
  else if(sw == 2){
    if(vinMatrix[0] == 0){
      extronSerial2.print(num);
      extronSerial2.print(F("*"));
      extronSerial2.print(F("!"));
    }
    else if(vinMatrix[0] == 2){
      extronSerial2.print(num);
      extronSerial2.print(F("*"));
      extronSerial2.print(ExtronVideoOutputPortSW2);
      extronSerial2.print(F("!"));
    }
  }
} // end of setTie()

void recallPreset(uint8_t num, uint8_t sw){
  if(sw == 1){
    extronSerial.print(num);
    extronSerial.print(F("."));
  }
  else if(sw == 2){
    extronSerial2.print(num);
    extronSerial2.print(F("."));
  }

  delay(20);
} // end of recallPreset()

#if !automatrixSW1
void MTVtime1(unsigned long eTime){
  MTVcurrentTime = millis();  // Init timer
  if(MTVprevTime == 0)       // If previous timer not initialized, do so now.
    MTVprevTime = millis();
  if((MTVcurrentTime - MTVprevTime) >= eTime){ // If it's been longer than eTime, send MT-VIKI serial command for current input, see if it responds with disconnected, and reset the timer.
    MTVcurrentTime = 0;
    MTVprevTime = 0;
    extronSerialEwrite("viki",currentMTVinput[0],1);
    delay(50);
 }
}  // end of MTVtime1()
#endif

#if !automatrixSW2
void MTVtime2(unsigned long eTime){
  MTVcurrentTime2 = millis();  // Init timer
  if(MTVprevTime2 == 0)       // If previous timer not initialized, do so now.
    MTVprevTime2 = millis();
  if((MTVcurrentTime2 - MTVprevTime2) >= eTime){ // If it's been longer than eTime, send MT-VIKI serial command for current input, see if it responds with disconnected, and reset the timer.
    MTVcurrentTime2 = 0;
    MTVprevTime2 = 0;
    extronSerialEwrite("viki",currentMTVinput[1] - 100,2);
    delay(50);
 }
}  // end of MTVtime2()
#endif

void ExtronOutputQuery(uint8_t outputNum, uint8_t sw){
  char cmd[6]; 
  uint8_t len = 0;
  cmd[len++] = 'v';
  char buff[4];
  itoa(outputNum,buff,10);
  for(char* p = buff; *p; p++){
    cmd[len++] = *p;
  }
  cmd[len++] = '%';
  if(sw == 1)
    extronSerial.write((uint8_t *)cmd,len);
  else if(sw == 2)
    extronSerial2.write((uint8_t *)cmd,len);

  delay(50);
} // end of ExtronOutputQuery()

void extronSerialEwrite(String type, uint8_t value, uint8_t sw){
  if(type == "viki"){
    viki[2] = byte(value - 1);
    if(sw == 1)
      extronSerial.write(viki, 4);
    else if(sw == 2)
      extronSerial2.write(viki, 4);
  }
  delay(50);
}  // end of extronSerialEwrite()

void sendProfile(int sprof, uint8_t sname, uint8_t soverride){
  if(sprof != 0){
    mswitch[sname].On = 1;
    mswitch[sname].Prof = sprof;
    if(soverride == 1){
      mswitch[sname].King = 1;
    }
    else{
      if(mswitch[sname].Prof != currentProf){
        mswitch[sname].King = 1;
      }
      else return;
    }
    for(int i=0;i < mswitchSize;i++){ // set previous King to 0
      if(i != sname && mswitch[i].King == 1)
        mswitch[i].King = 0;
    }
    sendSVS(sprof);
  }
  else if(sprof == 0){ // all inputs are off, set attributes to 0, find a console that is On starting at the top of the list, set as King, send profile
    mswitch[sname].On = 0;
    mswitch[sname].Prof = 0;
    if(mswitch[sname].King == 1){
      for(uint8_t k=0;k < mswitchSize;k++){
        if(sname == k){
          mswitch[k].King = 0;
          for(uint8_t l=0;l < mswitchSize;l++){ // find next Switch that has an active console
            if(mswitch[l].On == 1){
              mswitch[l].King = 1;
              sendSVS(mswitch[l].Prof);
              break;
            }
          }
        }
      } // end of for()
    } // end of if King == 1
    uint8_t count = 0;
    for(uint8_t m=0;m < mswitchSize;m++){
      if(mswitch[m].On == 0) count++;
    }
    if(S0 && (count == mswitchSize) && currentProf != 0){ // of S0 is true, send S0 or "remote prof12" when all consoles are off
      sendSVS(0);
    }  
  } // end of else if prof == 0
} // end of sendProfile()

#if automatrixSW1 || automatrixSW2
uint8_t readAMstate(String& sinput, uint8_t size){

  uint32_t newAMstate = 0;
  for(uint8_t i=0;i < size;i++){
    char c = sinput.charAt(i);
    if(c >= '1' && c <= '9'){
      newAMstate |= (1UL << (size - 1 - i));
    }
  }

  uint32_t changed = newAMstate ^ prevAMstate;

  for(uint8_t bitPos = 0;bitPos < size;bitPos++){
    uint32_t bit = 1UL << (size - 1 - bitPos);
    uint8_t input = bitPos + 1;
    if(changed & bit){
      if(newAMstate & bit){ // input on
        bool exists = false;
        for(int j=0;j <= AMstateTop;j++){
          if(AMstate[j] == input){
            exists = true;
            break;
          }
        }
        if(!exists && AMstateTop < (size - 1)) AMstate[++AMstateTop] = input;
      } // end of input on
      else{ // input off
        for(int j=0;j <= AMstateTop;j++){
          if(AMstate[j] == input){
            for(int k = j;k < AMstateTop;k++){
              AMstate[k] = AMstate[k + 1];
            }
            AMstateTop--;
            break;
          }
        }
      } // end of input off
    } // end of changed ?
  } // end of for

  prevAMstate = newAMstate;
  return AMstate[AMstateTop];
} // end of readAMstate()
#endif
