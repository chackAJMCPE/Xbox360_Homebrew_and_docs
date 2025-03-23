/*
This code gets the console type (MotherBoard revision, not including opus).
Use it however you want; its made to be free to use for anyone!
- chackAJMCPE
*/
#include "stdafx.h" // Visual Studio needs this, idk why. You cant compile without it.
#include <stdio.h> // For printf

struct _XBOX_HARDWARE_INFO; extern "C" _XBOX_HARDWARE_INFO* XboxHardwareInfo; // Kernel function

// Enum for console types
typedef enum {
    REV_XENON = 0,
    REV_ZEPHYR,
    REV_FALCON,
    REV_JASPER,
    REV_TRINITY,
    REV_CORONA,
    REV_CORONA_PHISON,
    REV_WINCHESTER,
    REV_UNKNOWN
} ConsoleType;

int GetConsoleType() {
    // Retrieve console type value, defaulting to 0x70000000 if XboxHardwareInfo is NULL
    DWORD value = XboxHardwareInfo ? *(DWORD*)XboxHardwareInfo & 0xF0000000 : 0x70000000;
    BYTE PCIBridgeRevisionID = XboxHardwareInfo ? *((BYTE*)XboxHardwareInfo + 5) : 0;
    
    // Determine console type
    switch (value) {
        case 0x00000000:
            return REV_XENON;
        case 0x10000000:
            return REV_ZEPHYR;
        case 0x20000000:
            return REV_FALCON;
        case 0x30000000:
            return REV_JASPER;
        case 0x40000000:
            return REV_TRINITY;
        case 0x50000000:
            if (PCIBridgeRevisionID == 0x70) { // TODO: Add sfcx check!
                return REV_CORONA_PHISON;
            } else {
                return REV_CORONA;
            }
        case 0x60000000:
            return REV_WINCHESTER;
        case 0x70000000:
            return REV_UNKNOWN;
		default:
            return REV_UNKNOWN;
    }
}

/*
int main() { // optionally print the console type (number) to UART
    int consoleType = GetConsoleType();
    printf("Console Type: %d\n", consoleType);  // Print the return value
    return 0;
}
*/
