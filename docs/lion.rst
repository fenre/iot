The Lion board
=============================================

The Lion is an Arduino-compatible LoRa development board for Internet of Things, made by Arrow.

General specifications
----------------------

* MC27561-LION
* Microchip CPU Cortex M0+ USB Host (ATSAMD21J18A-MU)
* Microchip Crypto Authentication (ATSHA204A-MAHDA-T
* Dynaflex 868Mhz Antenna
* Linear Power management (LTC3526LEDC-LTC4413EDD-LTC1844ES5)
* Microchip LoRA Module (RN2483)
* Telit GPS with Embedded Antenna (SE868-A)
* Microchip Bluetooth Low Energy (RN48730)


Datasheet
---------

Find the Lion User Guide here :download:`pdf <lion_guide.pdf>`

Find the schematics for Lion here :download:`pdf <lion_schematics.pdf>`


How to set up the Arduino IDE
-----------------------------

Open the Arduino IDE.

Go to Tools -> Boards -> Boards Manager. 
You'll need to add two packages.

* First, find and install "Arduino SAMD" Boards. 
* Then, in the "Type" drop-down menu, select "Partner", and choose "ARROW Boards". 

You now need to install the necessary libraries. 

* Go to Sketch -> Include Library -> Manage Libraries. 
* Search for RN2483 and install the one that says something like "SmartEverything LoRa RN2483". 

Install this, and you should now be ready to enter some code. 


The code to make it run
-----------------------

:: 
        
            SmarteEveryting Lion RN2483 Library - sendDataOTA_console

            This example shows how to configure and send messages after a OTA Join.
            Please consider the storeConfiguration example can be uset to store
            the required keys and skip the configuration part in curent example.
            
            created 25 Feb 2017
            by Seve (seve@ioteam.it)

            This example is in the public domain
            https://github.com/axelelettronica/sme-rn2483-library

            More information on RN2483 available here:
            http://www.microchip.com/wwwproducts/en/RN2483
                
           

This board is configured using the Arduino IDE.
Copy the following code into the IDE and upload it to the board::

        #include <Arduino.h>
        #include <rn2483.h>

        /* * INPUT DATA (for OTA)
        * 
        *  1) device EUI, 
        *  2) application EUI 
        *  3) and application key 
        *  
        *  then OTAA procedure can start.
        */
        char c; 
        char buff[100]  = {};
        unsigned char i = 0;
        unsigned char buff_size = 100;

        //30 seconds, default should be more than : 30000 is about 5 minutes;
        static long LOOP_PERIOD = 3000; 
        static long loop_cnt = LOOP_PERIOD - 300;  

        void setup() {
            errE err;
            const char *s;
            loraDbg = false;         // Set to 'true' to enable RN2483 TX/RX traces
            bool storeConfig = true; // Set to 'false' if persistend config already in place    
            
            delay(500);   
            Serial.begin(115200);

            lora.begin();
            delay(1000);
        
            Serial.print("FW Version :");
            Serial.println(lora.sysGetVersion());

        // Configuration of the LoRa module (keys, IDs, etc...)
            if (storeConfig) {
                // Write HwEUI
                Serial.println("Writing Hw EUI in DEV EUI ...");
                lora.macSetDevEUICmd(lora.sysGetHwEUI());
                s = lora.sendRawCmdAndAnswer("mac get deveui");
                Serial.print("NEW MAC DEV-EUI : ");
                Serial.println(s);

                if (err != RN_OK) {
                    Serial.println("\nFailed writing Dev EUI");
                }
                
                Serial.println("Writing APP EUI ...");
                err = lora.macSetAppEUICmd("F8CACBFE4713D195");
                if (err != RN_OK) {
                    Serial.println("\nFailed writing APP EUI");
                }
                
                Serial.println("Writing Application Key ...");
                lora.macSetAppKeyCmd("00112233445566778899AABBCCDDEEFF");
                if (err != RN_OK) {
                    Serial.println("\nFailed writing raw APP Key"); 
                }
                
                Serial.println("Writing Device Address ...");
                err = lora.macSetDevAddrCmd(lora.sendRawCmdAndAnswer("mac get devaddr"));
                if (err != RN_OK) {
                    Serial.println("\nFailed writing Dev Address");
                }
                
                Serial.println("Setting ADR ON ...");
                err = lora.macSetAdrOn();
                if (err != RN_OK) {
                    Serial.println("\nFailed setting ADR");
                }
            }
            
            Serial.println("Setting Automatic Reply ON ...");
            err = lora.macSetArOn();
            if (err != RN_OK) {
                Serial.println("\nFailed setting automatic reply");
            }
            
            Serial.println("Setting Trasmission Power to Max ...");
            lora.macPause();
            err = lora.radioSetPwr(14);
            if (err != RN_OK) {
                Serial.println("\nFailed Setting the power to max power");
            }
            lora.macResume();

            delay(5000);
        pinMode(PIN_LED_13, OUTPUT);
        }


        void loop() {
        
            static int loop_cnt = 0;
            static bool joined = false;
            static uint32_t state;
            String data;

            // Sending commands to the LoRa module via the console
            if (Serial.available()) {
                c = Serial.read();    
                Serial.write(c); 
                buff[i++] = c;
                if (c == '\n') {
                    Serial.print(lora.sendRawCmdAndAnswer(buff));
                    i = 0;
                    memset(buff, 0, sizeof(buff));
                }
            }

            // Print incomming messages to console
            if (lora.available()) {
                // Printing asychronous rx msgs from RN2483
                const char * info;
                char incoming[200];
                int counter = 0;  // variable for looping through the incoming message
                int msgStart = 0; // used for identifying the payload in the received packet
                String msgHex,msgType,finalMsg = ""; // msgHex - incoming message stored as a string of hex values; msgType - determine if there is message in the receive window; finalMsg - incoming message in characters
                info = lora.read(); // pointer to the beginning of the received message
                for(int k = 0;k<200;k++)
                {
                incoming[k] = *(info + k); // Store the content of the Serial buffer into a char array
                }
                Serial.println(incoming);

                // Verifying if a message arrived in one of the receive windows
                // If yes, the LoRa module will send the following structure to the MCU: mac_rx <port#> <data>
                for(counter ; counter<6 ; counter++) //collecting the first six characters of the LoRa module's message to the MCU
                {
                    msgType.concat(info[counter]);          
                }
                if(msgType.equals("mac_rx"))
                {
                memset(&msgType,0,sizeof(msgType));
                counter++; // Increase the counter by 1 so that now it points to the port#
                for(counter ; counter<200; counter++ ) // Continue looping through the LoRa module's message
                {
                    if(incoming[counter]=='\0') // When reaching the end of the message exit from the loop
                    break;

                    if(incoming[counter]==' ' && msgStart==0) // "Space" after port# indicates the beginning of the data field, where the received information is stored
                    {
                    msgStart = 1; // Set a "flag" to true so that we can start collecting the data in a String character by character 
                    counter++; //Jump to the first character of the data field which is a "Space" afer the <port #>
                    }
                    
                    if(msgStart)
                    {
                    msgHex.concat(incoming[counter]); // Building the data into a String variable character by character
                    }
                }

                    // data field (now msgHex value) contains hex coded ASCII characters. Following loop decodes these hex values into characters
                    for(int j = 0; j<msgHex.length()-2;j++)
                    {
                    counter= hexToInteger(msgHex[j])*16 + hexToInteger(msgHex[++j]);
                        finalMsg.concat((char)counter);
                    }
                    
                    Serial.println(finalMsg);
            }
                info = 0;  
            }

            if (!(loop_cnt % LOOP_PERIOD)) { // 30 seconds
            state = lora.macGetStatus();
            
            // Check If network is still joined
            if (MAC_JOINED(state)) {
                if (!joined) {
                        Serial.println("\nOTA Network JOINED! ");
                        joined = true;
                }

        // Collecting sensor info to be sent to the gateway
            data = "Hello World!\r\n";
            Serial.println("Sending Unconfirmed Buffer ...");
            lora.macTxCmd(data,data.length());
                
                } else {
                joined = false;
                lora.macJoinCmd(OTAA);
                delay(100);
            }
            loop_cnt = 0; 
            }
            loop_cnt++;
            delay(10);
        }


        int hexToInteger(char c){
        int tmpval;
        if (c >='0' && (c <= '9')) 
        {
            tmpval = c - '0';
        } 
        else if (c >='A' && (c <= 'F'))
        {
            tmpval = c - 'A' + 10;
        } 
        else if (c >='a' && (c <= 'f'))
        {
            tmpval = c - 'a' + 10;
        }
        return tmpval;
        }


Updating firmware
-----------------

In the odd case that you find yourself with a board with old firmware, it can be updated fairly easily.

You will need the Arduino IDE, and a package file that can be downloaded from here. :download:`zip <RN2483FirmwareUpdater.zip>`

Power up the board, and load the RN2483FirmwareUpdater.ino sketch. The Arduino IDE will have to be set up as per the instructions above.

Upload the sketch, and open the Serial Monitor. When prompted, type "c" in the input field and press Enter. 
When the process is done, load the getChipInformation sketch from "File" -> "Examples" -> "SmartEverything Lion". Upload it. Most probably, you'll not get any info now.
Unplug the board, and then start it again. Upload the getChipInformation sketch again. You should now see the new firmware version listed in the Serial Monitor.


In the rare case that you get an error message while updating the firmware, you can activate both debug and bootloader mode from the serial monitor after uploading the RN2483FirmwareUpdater sketch.

I updated 60 Lion boards, and found 2 that was DOA.