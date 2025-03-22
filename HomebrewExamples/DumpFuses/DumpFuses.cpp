/*
This code grabs the CPU key from memory and puts it into keybuf (as you can see under declarations)
Use it however you want; its all supposed to be free to use for anyone!
- chackAJMCPE
*/
#include "stdafx.h" // Visual Studio needs this, idk why. You cant compile without it.
#include <stdio.h> // for printf and other functions
extern "C" NTSYSAPI PVOID NTAPI MmGetPhysicalAddress(IN PVOID Address);

// Hypervisor peek syscall
unsigned long long __declspec(naked) HvxPeekCall(DWORD key, unsigned long long type, unsigned long long SourceAddress, unsigned long long DestAddress, unsigned long long lenInBytes)
{ 
	__asm {
		li      r0, 0x0
		sc
		blr
	}
}


unsigned long long getFuseline(DWORD fuse) {
    if (fuse >= 12) return 0;  // Only 12 fuse lines (0-11)

    unsigned long long value = 0;
    BYTE* buffer = (BYTE*)XPhysicalAlloc(8, MAXULONG_PTR, 0, 
                       MEM_LARGE_PAGES|PAGE_READWRITE|PAGE_NOCACHE);
    
    if (buffer) {
        // CORRECTED: Each fuse line is 0x200 bytes apart!! Free60 Wiki is wrong!
        unsigned long long hvAddress = 0x8000020000020000ULL + (fuse * 0x200);
        unsigned long long physAddr = (unsigned long long)(ULONG_PTR)MmGetPhysicalAddress(buffer);
        unsigned long long destAddr = 0x8000000000000000ULL | (physAddr & 0xFFFFFFFF);

        unsigned long long result = HvxPeekCall(0x72627472, 5, hvAddress, destAddr, 8);
        
        if (result == 0x72627472 || result == destAddr) {
            value = *((unsigned long long*)buffer);
        } else {
            printf("Error reading fuse %d: 0x%llX\n", fuse, result);
        }
        
        XPhysicalFree(buffer);
    }
    return value;
}


/*
int main() { // optionally print fuses to UART
    printf("fuse dump:\n");
    for (int i = 0; i < 12; i++) {
        unsigned long long val = getFuseline(i);
        printf("Fuse %2d: %016llX\n", i, val);
    }
    return 0;
}
*/
