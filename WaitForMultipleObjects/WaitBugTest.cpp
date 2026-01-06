#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <atomic>
#include <d3d11.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Test parameters
const int MAX_ITERATIONS = 100000;  // Run many times to catch rare bug
const int REPORT_INTERVAL = 10;   // Report progress every N iterations
const int NUM_EXTRA_HANDLES = 63;  // Create many handles to simulate complex scenario (MAXIMUM_WAIT_OBJECTS - 1)

// Statistics
std::atomic<int> totalTests(0);
std::atomic<int> bugOccurrences(0);
std::atomic<int> successfulWaits(0);

// Global D3D11 resources
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;

// Worker thread that signals the event after a delay
DWORD WINAPI SignalThread(LPVOID lpParam) {
    HANDLE hEvent = (HANDLE)lpParam;

    // Sleep for a reasonable time before signaling
    Sleep(100);  // 100ms delay

    SetEvent(hEvent);
    return 0;
}

// Initialize D3D11 device and context
bool InitD3D11() {
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,  // Driver Type
        nullptr,                    // Software
        0,                         // Flags
        nullptr,                    // Feature Levels
        0,                         // Num Feature Levels
        D3D11_SDK_VERSION,         // SDK Version
        &g_pd3dDevice,             // Device
        &featureLevel,             // Feature Level
        &g_pImmediateContext       // Device Context
    );

    if (FAILED(hr)) {
        printf("Failed to create D3D11 device: 0x%08X\n", hr);
        return false;
    }

    printf("D3D11 device created successfully.\n");
    return true;
}

// Clean up D3D11 resources
void CleanupD3D11() {
    if (g_pImmediateContext) {
        g_pImmediateContext->Release();
        g_pImmediateContext = nullptr;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

// Test function that tries to catch the bug
bool TestWaitForMultipleObjects() {
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {
        printf("Failed to create event: %lu\n", GetLastError());
        return false;
    }

    // Create many extra event handles to simulate complex scenario
    // Even though we only wait on 1, having many handles available may trigger the bug
    HANDLE extraHandles[NUM_EXTRA_HANDLES];
    for (int i = 0; i < NUM_EXTRA_HANDLES; i++) {
        extraHandles[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (extraHandles[i] == NULL) {
            printf("Failed to create extra event %d: %lu\n", i, GetLastError());
            // Clean up previously created handles
            for (int j = 0; j < i; j++) {
                CloseHandle(extraHandles[j]);
            }
            CloseHandle(hEvent);
            return false;
        }
    }

    // Create thread that will signal the event after delay
    HANDLE hThread = CreateThread(NULL, 0, SignalThread, hEvent, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create thread: %lu\n", GetLastError());
        for (int i = 0; i < NUM_EXTRA_HANDLES; i++) {
            CloseHandle(extraHandles[i]);
        }
        CloseHandle(hEvent);
        return false;
    }

    // Perform D3D11 graphics operations before wait
    if (g_pd3dDevice && g_pImmediateContext) {
        // Create a small texture
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = 256;
        texDesc.Height = 256;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        ID3D11Texture2D* pTexture = nullptr;
        HRESULT hr = g_pd3dDevice->CreateTexture2D(&texDesc, nullptr, &pTexture);

        if (SUCCEEDED(hr)) {
            // Force GPU flush
            g_pImmediateContext->Flush();
            pTexture->Release();
        }
    }

    // Call WaitForMultipleObjects with INFINITE timeout
    // This should NEVER return WAIT_TIMEOUT
    DWORD startTick = GetTickCount();
    DWORD result = WaitForMultipleObjects(1, &hEvent, FALSE, INFINITE);
    DWORD endTick = GetTickCount();
    DWORD elapsed = endTick - startTick;

    totalTests++;

    // Check for the bug: WAIT_TIMEOUT with INFINITE timeout
    if (result == WAIT_TIMEOUT) {
        bugOccurrences++;
        printf("\n!!! BUG DETECTED !!!\n");
        printf("WaitForMultipleObjects returned WAIT_TIMEOUT with INFINITE timeout!\n");
    for (int i = 0; i < NUM_EXTRA_HANDLES; i++) {
        CloseHandle(extraHandles[i]);
    }
        printf("Elapsed time: %lu ms\n", elapsed);
        printf("Iteration: %d\n", totalTests.load());
        printf("Bug occurrence count: %d\n\n", bugOccurrences.load());

        // Clean up
        SetEvent(hEvent);  // Make sure thread can exit
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
        CloseHandle(hEvent);
        return true;  // Bug found
    }
    else if (result == WAIT_OBJECT_0) {
        successfulWaits++;
        // Normal expected behavior
    }
    else {
        printf("Unexpected return value: 0x%08X (Error: %lu)\n", result, GetLastError());
    }

    // Clean up
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hEvent);

    return false;  // No bug this iteration
}

int main() {
    printf("WaitForMultipleObjects Bug Test - Qualcomm Windows\n");
    printf("==================================================\n");
    printf("Testing for spurious WAIT_TIMEOUT with INFINITE timeout\n");
    printf("Maximum iterations: %d\n", MAX_ITERATIONS);
    printf("Progress reports every: %d iterations\n\n", REPORT_INTERVAL);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    printf("Processor Architecture: ");
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_ARM64:
            printf("ARM64\n");
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            printf("x64\n");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            printf("x86\n");
            break;
        default:
            printf("Unknown (%d)\n", sysInfo.wProcessorArchitecture);
    }
    printf("\n");

    // Initialize D3D11
    if (!InitD3D11()) {
        printf("Failed to initialize D3D11. Exiting.\n");
        return 1;
    }

    DWORD overallStartTime = GetTickCount();

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        bool bugFound = TestWaitForMultipleObjects();

        // Stop immediately if bug is detected
        if (bugFound) {
            printf("Stopping test - bug detected!\n");
            break;
        }

        // Progress report
        if ((i + 1) % REPORT_INTERVAL == 0) {
            printf("Progress: %d/%d iterations completed. Bugs found: %d\n",
                   i + 1, MAX_ITERATIONS, bugOccurrences.load());
        }

        // Small delay between iterations to avoid overwhelming the system
        Sleep(1);
    }

    DWORD overallEndTime = GetTickCount();
    DWORD totalTime = overallEndTime - overallStartTime;

    printf("\n");
    printf("==================================================\n");
    printf("Test Complete\n");
    printf("==================================================\n");
    printf("Total iterations: %d\n", totalTests.load());
    printf("Successful waits: %d\n", successfulWaits.load());
    printf("Bug occurrences: %d\n", bugOccurrences.load());
    printf("Bug rate: %.4f%%\n",
           totalTests.load() > 0 ? (bugOccurrences.load() * 100.0 / totalTests.load()) : 0.0);
    printf("Total test time: %.2f seconds\n", totalTime / 1000.0);
    printf("\n");

    // Clean up D3D11
    CleanupD3D11();

    if (bugOccurrences.load() > 0) {
        printf("*** BUG REPRODUCED ***\n");
        printf("The bug was caught %d times during this test run.\n", bugOccurrences.load());
        return 1;  // Return error code to indicate bug found
    }
    else {
        printf("No bugs detected in this run.\n");
        printf("Try running the test multiple times or increasing MAX_ITERATIONS.\n");
        return 0;
    }
}
