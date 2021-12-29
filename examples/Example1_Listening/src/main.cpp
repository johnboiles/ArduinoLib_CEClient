/*
    Example1_Listening.ino

    Basic example to demonstrate the use of the CEClient library
    The client is configured in promiscuous and monitor mode 
    to receive all the messages on the CEC bus
    
    No specific callback function is defined, therefore the client
    calls the default one, which prints the packets on the Serial port

    Use http://www.cec-o-matic.com/ to decode captured messages
*/

#include <Arduino.h>
#include "CEClient.h"

#define CEC_PHYSICAL_ADDRESS    0x00
#define CEC_INPUT_PIN           D2
#define CEC_OUTPUT_PIN          D1

// create a CEC client
CEClient ceclient(CEC_PHYSICAL_ADDRESS, CEC_INPUT_PIN, CEC_OUTPUT_PIN);


void transmitComplete(bool complete) {
    Serial.printf("Transmit complete = %d\n", complete);
}

void messageReceived(int source, int dest, unsigned char* buffer, int count) {
    DbgPrint("RX (%ld): %02d -> %02d: %02X", millis(), source, dest, ((source&0x0f)<<4)|(dest&0x0f));
    for (int i = 0; i < count; i++)
        DbgPrint(":%02X", buffer[i]);
    DbgPrint("\n");
    // 0F:85 Request active source
    // 05:70:00:00 System audio mode request
    // 05:7D system audio mode status
    // 05:83 give physical address
    if (dest != 0x05) {
        return;
    }
    if (count == 1) {
        if (buffer[0] == 0xC3) { // CEC_OPCODE_REQUEST_ARC_START
            DbgPrint("Start arc\n");
            unsigned char buffer[] = {0xC1};
            ceclient.TransmitFrame(0, buffer, 1);        
        } else if (buffer[0] == 0x7D) {
            // Give system audio mode status
            DbgPrint("System audio mode status response\n");
            unsigned char buffer[] = {0x7E,0x01};
            ceclient.TransmitFrame(0, buffer, 2);
        } else if (buffer[0] == 0x46) {
            // whats your name?
            // Packet received at 5399: 05 -> 00: 50:47:70:79:74:68:6F:6E:2D:63:65:63 // My name python-cec
            DbgPrint("sending name\n");
            unsigned char buffer[] = {0x47,0x65, 0x73, 0x70, 0x68, 0x6F, 0x6D, 0x65};
            ceclient.TransmitFrame(0, buffer, 8);        

        } else if (buffer[0] == 0x8C) {
            DbgPrint("sending vendorid\n");
            unsigned char buffer[] = {0x87,0x00,0x13,0x37};
            ceclient.TransmitFrame(15, buffer, 4);        
        }
    } else if (count == 2) {
        if (buffer[1] == 0x41) {
            DbgPrint("vol up\n");
        } else if (buffer[1] == 0x42) {
            DbgPrint("vol down\n");
        } else if (buffer[1] == 0x43) {
            DbgPrint("vol mute\n");
        }
    } else if (count == 3) {
        // System audio mode request
        if (buffer[0] = 0x70 && buffer[1]==0x00 && buffer[2] == 0x00) {
            
            DbgPrint("System audio mode response\n");
            unsigned char buffer[] = {0x50,0x72,0x01};
            ceclient.TransmitFrame(0, buffer, 3);        

        }
    }
}

void setup() {

    Serial.begin(115200);
    Serial.println("Starting...");

    // initialize the client with the default device type (PLAYBACK)
    ceclient.begin(CEC_LogicalDevice::CDT_AUDIO_SYSTEM);

    // enable promiscuous mode (print all the incoming messages)
    ceclient.setPromiscuous(true);

    ceclient.onTransmitCompleteCallback(transmitComplete);
    ceclient.onReceiveCallback(messageReceived);
    // enable monitor mode (do not transmit)
    // ceclient.setMonitorMode(true);
    // ceclient.run();

    // unsigned char buffer[] = {0x50, 0x36};
    // unsigned char buffer[] = {0x50, 0x44, 0x40};
    // if (!ceclient.Transmit(buffer, 2)) {
    //     Serial.println("Failed Transmit");
    // } else {
    //     Serial.println("Transmit off");
    // }
    // unsigned char buffer2[] = {0x36};
    // ceclient.TransmitFrame(0, buffer2, 1);
}

void loop() {
    // run the client
    ceclient.run();
    // ceclient.Transmit
    char in = Serial.read();
    if (in == 'a') {
        Serial.println("On!");
        // unsigned char buffer2[] = {0x36};
        // Turns the tv on
        unsigned char buffer[] = {0x04};
        ceclient.TransmitFrame(0, buffer, 1);
    } else if (in == 'b') {
        Serial.println("Standby!");
        unsigned char buffer[] = {0x36};
        ceclient.TransmitFrame(15, buffer, 1);
    } else if (in == 'w') {
        Serial.println("Up!");
        unsigned char buffer[] = {0x44, 0x01};
        ceclient.TransmitFrame(0, buffer, 2);
    } else if (in == 'e') {
        Serial.println("Release!");
        unsigned char buffer[] = {0x45};
        ceclient.TransmitFrame(0, buffer, 1);        
    }
}
