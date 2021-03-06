/**************************************************************************
 *
 * Copyright 2012 Jose Fonseca
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <initguid.h>
#include <windows.h>

#include "winsdk_compat.h"

#include <d3d11.h>

#include <d3d9.h> // D3DPERF_*

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#include "tri_vs_4_0.h"
#include "tri_ps_4_0.h"


template< typename T >
inline void
setObjectName(ComPtr<T> &pObject, const char *szName)
{
    pObject->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(szName), szName);
}


int
main(int argc, char *argv[])
{
    HRESULT hr;

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        DefWindowProc,
        0,
        0,
        hInstance,
        NULL,
        NULL,
        NULL,
        NULL,
        "SimpleDX10",
        NULL
    };
    RegisterClassEx(&wc);

    const int WindowWidth = 250;
    const int WindowHeight = 250;

    DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    RECT rect = {0, 0, WindowWidth, WindowHeight};
    AdjustWindowRect(&rect, dwStyle, FALSE);

    HWND hWnd = CreateWindow(wc.lpszClassName,
                             "Simple example using DirectX10",
                             dwStyle,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left,
                             rect.bottom - rect.top,
                             NULL,
                             NULL,
                             hInstance,
                             NULL);
    if (!hWnd) {
        return 1;
    }

    UINT Flags = 0;
    hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_NULL, 0, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION, NULL, NULL, NULL);
    if (SUCCEEDED(hr)) {
        Flags |= D3D11_CREATE_DEVICE_DEBUG;
    }

    ComPtr<IDXGIFactory> pFactory;
    hr = CreateDXGIFactory(IID_IDXGIFactory, (void**)(&pFactory));
    if (FAILED(hr)) {
        return 1;
    }

    ComPtr<IDXGIAdapter> pAdapter;
    hr = pFactory->EnumAdapters(0, &pAdapter);
    if (FAILED(hr)) {
        return 1;
    }

    static const D3D_FEATURE_LEVEL FeatureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pDeviceContext;
    hr = D3D11CreateDevice(pAdapter.Get(),
                           D3D_DRIVER_TYPE_UNKNOWN,
                           NULL, /* Software */
                           Flags,
                           FeatureLevels,
                           sizeof FeatureLevels / sizeof FeatureLevels[0],
                           D3D11_SDK_VERSION,
                           &pDevice,
                           NULL, /* pFeatureLevel */
                           &pDeviceContext); /* ppImmediateContext */
    if (FAILED(hr)) {
        return 1;
    }

    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof SwapChainDesc);
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.OutputWindow = hWnd;
    SwapChainDesc.Windowed = true;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ComPtr<IDXGISwapChain> pSwapChain;
    hr = pFactory->CreateSwapChain(pDevice.Get(), &SwapChainDesc, &pSwapChain);
    if (FAILED(hr)) {
        return 1;
    }

    D3DPERF_SetMarker(D3DCOLOR_XRGB(128, 128, 128), L"Marker");

    setObjectName(pDevice, "Device");

    ComPtr<ID3D11Texture2D> pBackBuffer;
    hr = pSwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void **)&pBackBuffer);
    if (FAILED(hr)) {
        return 1;
    }

    setObjectName(pBackBuffer, "BackBuffer");

    D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
    ZeroMemory(&RenderTargetViewDesc, sizeof RenderTargetViewDesc);
    RenderTargetViewDesc.Format = SwapChainDesc.BufferDesc.Format;
    RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    RenderTargetViewDesc.Texture2D.MipSlice = 0;

    ComPtr<ID3D11RenderTargetView> pRenderTargetView;
    hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(), &RenderTargetViewDesc, &pRenderTargetView);
    if (FAILED(hr)) {
        return 1;
    }

    pDeviceContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), NULL);

    D3DPERF_BeginEvent(D3DCOLOR_XRGB(128, 128, 128), L"Frame");

    D3DPERF_BeginEvent(D3DCOLOR_XRGB(128, 128, 128), L"Clear");
    const float clearColor[4] = { 0.3f, 0.1f, 0.3f, 1.0f };
    pDeviceContext->ClearRenderTargetView(pRenderTargetView.Get(), clearColor);
    D3DPERF_EndEvent(); // Clear

    ComPtr<ID3D11VertexShader> pVertexShader;
    hr = pDevice->CreateVertexShader(g_VS, sizeof g_VS, NULL, &pVertexShader);
    if (FAILED(hr)) {
        return 1;
    }

    struct Vertex {
        float position[4];
        float color[4];
    };

    static const D3D11_INPUT_ELEMENT_DESC InputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ComPtr<ID3D11InputLayout> pVertexLayout;
    hr = pDevice->CreateInputLayout(InputElementDescs,
                                    2,
                                    g_VS, sizeof g_VS,
                                    &pVertexLayout);
    if (FAILED(hr)) {
        return 1;
    }

    pDeviceContext->IASetInputLayout(pVertexLayout.Get());

    ComPtr<ID3D11PixelShader> pPixelShader;
    hr = pDevice->CreatePixelShader(g_PS, sizeof g_PS, NULL, &pPixelShader);
    if (FAILED(hr)) {
        return 1;
    }

    pDeviceContext->VSSetShader(pVertexShader.Get(), NULL, 0);
    pDeviceContext->PSSetShader(pPixelShader.Get(), NULL, 0);

    static const Vertex vertices[] = {
        { { -0.9f, -0.9f, 0.5f, 1.0f}, { 0.8f, 0.0f, 0.0f, 0.1f } },
        { {  0.9f, -0.9f, 0.5f, 1.0f}, { 0.0f, 0.9f, 0.0f, 0.1f } },
        { {  0.0f,  0.9f, 0.5f, 1.0f}, { 0.0f, 0.0f, 0.7f, 0.1f } },
    };

    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof BufferDesc);
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof vertices;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA BufferData;
    BufferData.pSysMem = vertices;
    BufferData.SysMemPitch = 0;
    BufferData.SysMemSlicePitch = 0;

    ComPtr<ID3D11Buffer> pVertexBuffer;
    hr = pDevice->CreateBuffer(&BufferDesc, &BufferData, &pVertexBuffer);
    if (FAILED(hr)) {
        return 1;
    }

    UINT Stride = sizeof(Vertex);
    UINT Offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &Stride, &Offset);

    D3D11_VIEWPORT ViewPort;
    ViewPort.TopLeftX = 0;
    ViewPort.TopLeftY = 0;
    ViewPort.Width = WindowWidth;
    ViewPort.Height = WindowHeight;
    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;
    pDeviceContext->RSSetViewports(1, &ViewPort);

    D3D11_RASTERIZER_DESC RasterizerDesc;
    ZeroMemory(&RasterizerDesc, sizeof RasterizerDesc);
    RasterizerDesc.CullMode = D3D11_CULL_NONE;
    RasterizerDesc.FillMode = D3D11_FILL_SOLID;
    RasterizerDesc.FrontCounterClockwise = true;
    RasterizerDesc.DepthClipEnable = true;
    ComPtr<ID3D11RasterizerState> pRasterizerState;
    hr = pDevice->CreateRasterizerState(&RasterizerDesc, &pRasterizerState);
    if (FAILED(hr)) {
        return 1;
    }
    pDeviceContext->RSSetState(pRasterizerState.Get());

    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    D3DPERF_BeginEvent(D3DCOLOR_XRGB(128, 128, 128), L"Draw");
    pDeviceContext->Draw(3, 0);
    D3DPERF_EndEvent(); // Draw

    D3DPERF_EndEvent(); // Frame

    pSwapChain->Present(0, 0);


    ID3D11Buffer *pNullBuffer = NULL;
    UINT NullStride = 0;
    UINT NullOffset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pNullBuffer, &NullStride, &NullOffset);

    pDeviceContext->OMSetRenderTargets(0, NULL, NULL);

    pDeviceContext->IASetInputLayout(NULL);

    pDeviceContext->VSSetShader(NULL, NULL, 0);

    pDeviceContext->PSSetShader(NULL, NULL, 0);

    pDeviceContext->RSSetState(NULL);

    DestroyWindow(hWnd);

    return 0;
}

