/*!
 * @file main.c
 * @brief Update URI Content of an NDEF formatted Mifare Classic card.
 * ----------
 * Adapted code from Seeed Studio PN532 driver for Arduino.
 * You can find the Seeed Studio PN532 driver here: https://github.com/Seeed-Studio/PN532
 * ----------
 * Inspired by examples in ValvanoWareTM4C123 by Dr. Jonathan Valvano
 * as well as his book Embedded Systems: Real-Time Interfacing to Arm Cortex-M Microcontrollers
 * You can find ValvanoWareTM4C123 at http://edx-org-utaustinx.s3.amazonaws.com/UT601x/ValvanoWareTM4C123.zip?dl=1
 * You can find his book at https://www.amazon.com/gp/product/1463590156/ref=oh_aui_detailpage_o05_s00?ie=UTF8&psc=1
 * You can find more of his work at http://users.ece.utexas.edu/~valvano/
 * ----------
 * NXP PN532 Data Sheet: https://www.nxp.com/docs/en/nxp/data-sheets/PN532_C1.pdf
 * NXP PN532 User Manual: https://www.nxp.com/docs/en/user-guide/141520.pdf
 * ----------
 * For future development and updates, please follow this repository: https://github.com/ZeeLivermorium/PN532_TM4C123
 * ----------
 * If you find any bug or problem, please create new issue or a pull request with a fix in the repository.
 * Or you can simply email me about the problem or bug at zeelivermorium@gmail.com
 * Much Appreciated!
 * ----------
 * @author Zee Livermorium
 * @date Apr 19, 2018
 */

#include <stdint.h>
#include <string.h>
#include "PLL.h"
#include "Serial.h"
#include "PN532.h"

uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };         // buffer to store the returned UID
uint8_t uidLength;                               // length of the UID (4 or 7 bytes depending on ISO14443A card type)
char*   serial_buffer;                             // a buffer pointer for serial reading

// Use the default NDEF keys (these should have been set by mifareClassic_formatNDEF function)
uint8_t keyA[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
uint8_t keyB[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };

// Note: check out PN532.h for all NDEF prefixes.

const char * URIContent = "github.com/ZeeLivermorium/PN532_4C123";   // for a url
uint8_t URIPrefix = NDEF_URIPREFIX_HTTP_WWWDOT;

//const char * URIContent = "mail@example.com";    // for an email address
//uint8_t URIPrefix = NDEF_URIPREFIX_MAILTO;

//const char * URIContent = "+1 420 420 6969";     // for a phone number
//uint8_t URIPrefix = NDEF_URIPREFIX_TEL;

int main(void) {
    /*-- TM4C123 Init --*/
    PLL_Init(Bus80MHz);                   // bus clock at 80 MHz
    PN532_Init();                         // init and wake up PN532
    Serial_Init();                        // for serial I/O
    
    /*-- PN532 Init --*/
    uint32_t firmwareVersion = PN532_getFirmwareVersion();
    
    if (!firmwareVersion) {               // if not able to read version number, quit
        Serial_println("Did not find PN532 board :(");
        return 0;                         // exit
    }
    
    /* output firmware info */
    Serial_println("");
    Serial_println("Found PN5%x", (firmwareVersion >> 24) & 0xFF);
    Serial_println("Firmware Version %u.%u", (firmwareVersion >> 16) & 0xFF, (firmwareVersion >> 8) & 0xFF);
    Serial_println("-------------------------------");
    
    PN532_SAMConfiguration();                          // configure board to read RFID tags
    
    /*-- loop --*/
    while(1) {
        Serial_println("Place NDEF formatted Mifare Classic card on the reader to update its content, ");
        Serial_println("and press [Enter] to continue ...");
        Serial_getString(serial_buffer, 0);  // wait for any key to be pressed
        
        if ( readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength) ) {
            Serial_println("Found a card :) ");
            Serial_println("UID Length: %u bytes.", uidLength); // output uid length
            /* output uid */
            Serial_print("UID: ");
            for (uint8_t i=0; i < uidLength; i++) Serial_print(" 0x%x", uid[i]);
            Serial_println("");
            Serial_println("-------------------------------");
            
            /* make sure it's a Mifare Classic card */
            if(uidLength != 4) {          // not Mifare Classic
                Serial_println("This doesn't seem to be a Mifare Classic card :(");
                delay(1500);              // PN532(no netflix) and chill before continuing :)
                continue;
            }
            Serial_println("Found a Mifare Classic card :)");
            
            /* authenticate NDEF format */
            if (!mifareClassic_authenticateBlock (uid, uidLength, 4, 0, keyB)) {
                Serial_println("Unable to authenticate block 4 ... this is not NDEF formatted :(");
                Serial_println("*******************************");
                Serial_println("");
                delay(1500);              // PN532(no netflix) and chill before continuing :)
                continue;
            }
            
            Serial_println("Authentication succeeded (seems to be an NDEF/NFC Forum tag) ...");
            
            /* check URI content length */
            if (strlen(URIContent) > 38)
            {
                /*
                 * the length is also checked in the writeNDEFURI function, but lets
                 * warn users here just in case they change the value and it's bigger
                 * than it should be
                 */
                Serial_println("URI content length is too long ... must be less than 38 characters");
                Serial_println("*******************************");
                Serial_println("");
                delay(1500);              // PN532(no netflix) and chill before continuing :)
                continue;
            }
            
            Serial_println("Updating sector 1 with URI as NDEF Message");
            
            /* try to write an NDEF record to sector 1 */
            if (mifareClassic_writeNDEFURI(1, URIPrefix, URIContent)) {
                Serial_println("NDEF URI Record written to sector 1");
                Serial_println("Job Done!");
            }
            else Serial_println("NDEF Record creation failed! :(");
            
            Serial_println("*******************************");
            Serial_println("");
            
            delay(1500);                  // PN532(no netflix) and chill before continuing :)
        }
        else Serial_println("No card is found :( ");
    }
}

