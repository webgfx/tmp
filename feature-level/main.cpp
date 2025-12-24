#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdio.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

const char* GetFeatureLevelString(D3D_FEATURE_LEVEL level) {
    switch (level) {
        case D3D_FEATURE_LEVEL_9_1:  return "9.1";
        case D3D_FEATURE_LEVEL_9_2:  return "9.2";
        case D3D_FEATURE_LEVEL_9_3:  return "9.3";
        case D3D_FEATURE_LEVEL_10_0: return "10.0";
        case D3D_FEATURE_LEVEL_10_1: return "10.1";
        case D3D_FEATURE_LEVEL_11_0: return "11.0";
        case D3D_FEATURE_LEVEL_11_1: return "11.1";
        case D3D_FEATURE_LEVEL_12_0: return "12.0";
        case D3D_FEATURE_LEVEL_12_1: return "12.1";
        case D3D_FEATURE_LEVEL_12_2: return "12.2";
        default: return "Unknown";
    }
}

void CheckD3D11FeatureLevel() {
    printf("=== D3D11 ===\n");

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    D3D_FEATURE_LEVEL obtainedLevel;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Hardware device
        nullptr,                    // No software rasterizer
        0,                          // Flags
        featureLevels,              // Feature levels to try
        _countof(featureLevels),    // Number of feature levels
        D3D11_SDK_VERSION,          // SDK version
        &device,                    // Output device
        &obtainedLevel,             // Output feature level
        &context                    // Output device context
    );

    if (SUCCEEDED(hr)) {
        printf("D3D11 Device created successfully!\n");
        printf("Feature Level: %s (0x%x)\n", GetFeatureLevelString(obtainedLevel), obtainedLevel);
        context->Release();
        device->Release();
    } else {
        printf("Failed to create D3D11 device. HRESULT: 0x%08X\n", hr);
    }
}

void CheckD3D12FeatureLevel() {
    printf("\n=== D3D12 ===\n");

    // Create DXGI factory to enumerate adapters
    IDXGIFactory4* factory = nullptr;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        printf("Failed to create DXGI factory. HRESULT: 0x%08X\n", hr);
        return;
    }

    // Get default adapter
    IDXGIAdapter1* adapter = nullptr;
    hr = factory->EnumAdapters1(0, &adapter);
    if (FAILED(hr)) {
        printf("Failed to enumerate adapters. HRESULT: 0x%08X\n", hr);
        factory->Release();
        return;
    }

    // Try feature levels from highest to lowest
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    ID3D12Device* device = nullptr;
    D3D_FEATURE_LEVEL maxLevel = (D3D_FEATURE_LEVEL)0;

    for (int i = 0; i < _countof(featureLevels); i++) {
        hr = D3D12CreateDevice(adapter, featureLevels[i], IID_PPV_ARGS(&device));
        if (SUCCEEDED(hr)) {
            maxLevel = featureLevels[i];
            device->Release();
            device = nullptr;
            break;
        }
    }

    if (maxLevel != 0) {
        printf("D3D12 Device created successfully!\n");
        printf("Feature Level: %s (0x%x)\n", GetFeatureLevelString(maxLevel), maxLevel);
    } else {
        printf("Failed to create D3D12 device. HRESULT: 0x%08X\n", hr);
    }

    adapter->Release();
    factory->Release();
}

int main() {
    CheckD3D11FeatureLevel();
    CheckD3D12FeatureLevel();
}
