/*
This code grabs the CPU key from memory and puts it into keybuf (as you can see under declarations)
Use it however you want; its all supposed to be free to use for anyone!
- chackAJMCPE
*/
#include "stdafx.h" // Visual Studio needs this, idk why. You cant compile without it.
#include <stdio.h> // for printf and other functions
unsigned char keybuf[0x10];
extern "C" NTSYSAPI PVOID NTAPI MmGetPhysicalAddress(IN PVOID Address);

unsigned long long __declspec(naked) HvxPeekCall(DWORD key, unsigned long long type, unsigned long long SourceAddress, unsigned long long DestAddress, unsigned long long lenInBytes)
{ 
	__asm {
		li      r0, 0x0
		sc
		blr
	}
}

unsigned long long HvxPeek(unsigned long long SourceAddress, unsigned long long DestAddress, unsigned long long lenInBytes)
{
	return HvxPeekCall(0x72627472, 5, SourceAddress, DestAddress, lenInBytes);
}


bool GetCPUKey()
{
	PBYTE buf = (PBYTE)XPhysicalAlloc(0x10, MAXULONG_PTR, 0, MEM_LARGE_PAGES|PAGE_READWRITE|PAGE_NOCACHE);
	if (buf != NULL)
	{
		unsigned long long dest = 0x8000000000000000ULL | ((DWORD)MmGetPhysicalAddress(buf)&0xFFFFFFFF);
		ZeroMemory(buf, 0x10);
		unsigned long long ret = HvxPeek(0x20ULL, dest, 0x10ULL);
		if(ret == 0x72627472 || ret == dest)
		{
			memcpy(keybuf, buf, 0x10);
			XPhysicalFree(buf);
			return true;
		}
		else
			printf("SysCall Return value: 0x%llX\n", ret);
		XPhysicalFree(buf);
	}
	return false;
}



/* 
int main() // optionally print the cpu key to UART
{
    if(GetCPUKey())
    {
        printf("CPU Key: ");
        for (int i = 0; i < 0x10; i++)
        {
            printf("%02X", keybuf[i]);
        }
        printf("\n");
    }
    else
    {
        printf("Failed to retrieve CPU Key.\n");
    }
    return 0;
}
*/
