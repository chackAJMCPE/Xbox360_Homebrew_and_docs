/*
This code downloads a file from web using a specified url.
Use it however you want; its all made to be free to use for anyone!
- chackAJMCPE
*/
#include "stdafx.h" // Visual Studio needs this, idk why. You cant compile without it.
#include <xtl.h>
#include <stdio.h>

void DownloadFileHttp(const char* url, const char* destination)
{
    // Parse URL components
    char host[256] = {0};
    char path[512] = "/";
    const char* url_start = strstr(url, "://");
    const char* host_start = url_start ? url_start + 3 : url;
    const char* path_start = strchr(host_start, '/');

    // Extract host and path
    if(path_start) {
        size_t host_len = path_start - host_start;
        strncpy(host, host_start, (host_len < 255) ? host_len : 255);
        host[255] = '\0';
        strncpy(path, path_start, 511);
        path[511] = '\0';
    } else {
        strncpy(host, host_start, 255);
        host[255] = '\0';
    }

    // Initialize network
    XNetStartupParams xnsp = {0};
    xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
    xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
    XNetStartup(&xnsp);

    // Wait for network initialization
    DWORD dwStart = GetTickCount();
    while((GetTickCount() - dwStart) < 6000) {}

    // Start Winsock
    WSADATA WsaData;
    if(WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
        XNetCleanup();
        return;
    }

    // DNS lookup
    XNDNS* dnsResult = NULL;
    INT err = XNetDnsLookup(host, NULL, &dnsResult);
    
    // Wait for DNS resolution
    while(dnsResult && dnsResult->iStatus == WSAEINPROGRESS) {
        Sleep(50);
    }

    if(!dnsResult || dnsResult->iStatus != 0 || dnsResult->cina == 0) {
        if(dnsResult) XNetDnsRelease(dnsResult);
        WSACleanup();
        XNetCleanup();
        return;
    }

    // Get first IPv4 address
    ULONG ipAddress = dnsResult->aina[0].S_un.S_addr;
    XNetDnsRelease(dnsResult);

    // Create socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == INVALID_SOCKET) {
        WSACleanup();
        XNetCleanup();
        return;
    }

    // Set security bypass option
    BOOL optTrue = TRUE;
    setsockopt(sock, SOL_SOCKET, 0x5801, (const char*)&optTrue, sizeof(BOOL));

    // Configure connection
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.S_un.S_addr = ipAddress;
    service.sin_port = htons(80);

    // Connect to server
    if(connect(sock, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        XNetCleanup();
        return;
    }

    // Send HTTP request
    char request[1024];
    sprintf_s(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n\r\n",
        path, host);

    if(send(sock, request, (int)strlen(request), 0) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        XNetCleanup();
        return;
    }

    // Open output file
    FILE* fp = NULL;
    if(fopen_s(&fp, destination, "wb") != 0 || !fp) {
        closesocket(sock);
        WSACleanup();
        XNetCleanup();
        return;
    }

    // Receive data with header stripping
    char buffer[4096];
    int bytesReceived;
    int headerState = 0;
    bool headersProcessed = false;
    int contentStart = 0;

    while((bytesReceived = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        if(headersProcessed) {
            fwrite(buffer, 1, bytesReceived, fp);
            continue;
        }

        for(int i = 0; i < bytesReceived; i++) {
            if(headerState == 0 && buffer[i] == '\r') headerState = 1;
            else if(headerState == 1 && buffer[i] == '\n') headerState = 2;
            else if(headerState == 2 && buffer[i] == '\r') headerState = 3;
            else if(headerState == 3 && buffer[i] == '\n') {
                headersProcessed = true;
                contentStart = i + 1;
                break;
            }
            else {
                headerState = 0;
            }
        }

        if(headersProcessed) {
            // Write remaining data from this chunk
            if(contentStart < bytesReceived) {
                fwrite(buffer + contentStart, 1, bytesReceived - contentStart, fp);
            }
        }
    }

    // Cleanup
    fclose(fp);
    closesocket(sock);
    WSACleanup();
    XNetCleanup();
}

void __cdecl main()
{
    DownloadFileHttp("http://example.com/example.file", "game:\\data.xml"); // The url MUST be HTTP. Xbox doesnt support HTTPS. 
	// The usage of this function is: DownloadFileHttp("url", "location (with filename)");
}