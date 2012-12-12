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

#include <initguid.h>
#include <windows.h>

#include "compat.h"

#include <d3d10_1.h>

#include "tri_vs_4_0.h"
#include "tri_ps_4_0.h"


static IDXGIFactory1 *g_pFactory = NULL;
static IDXGIAdapter *g_pAdapter = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D10Device1 * g_pDevice = NULL;


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
    BOOL Windowed = TRUE;

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

    ShowWindow(hWnd, SW_SHOW);

    UINT Flags = 0;
    if (LoadLibraryA("d3d10sdklayers")) {
        Flags |= D3D10_CREATE_DEVICE_DEBUG;
    }

    hr = CreateDXGIFactory1(IID_IDXGIFactory1, (void**)(&g_pFactory) );
    if (FAILED(hr)) {
        return 1;
    }

    hr = g_pFactory->EnumAdapters(0, &g_pAdapter);
    if (FAILED(hr)) {
        return 1;
    }

    hr = D3D10CreateDevice1(g_pAdapter,
                            D3D10_DRIVER_TYPE_HARDWARE,
                            NULL,
                            Flags,
                            D3D10_FEATURE_LEVEL_10_0,
                            D3D10_1_SDK_VERSION,
                            &g_pDevice);
    if (FAILED(hr)) {
        return 1;
    }

    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof SwapChainDesc);
    SwapChainDesc.BufferDesc.Width = WindowWidth;
    SwapChainDesc.BufferDesc.Height = WindowHeight;
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

    hr = g_pFactory->CreateSwapChain(g_pDevice, &SwapChainDesc, &g_pSwapChain);
    if (FAILED(hr)) {
        return 1;
    }

    ID3D10RenderTargetView *pRenderTargetView = NULL;
    ID3D10Texture2D* pBackBuffer;
    hr = g_pSwapChain->GetBuffer(0, IID_ID3D10Texture2D, (void **)&pBackBuffer);
    if (FAILED(hr)) {
        return 1;
    }
    D3D10_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
    ZeroMemory(&RenderTargetViewDesc, sizeof RenderTargetViewDesc);
    RenderTargetViewDesc.Format = SwapChainDesc.BufferDesc.Format;
    RenderTargetViewDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    RenderTargetViewDesc.Texture2D.MipSlice = 0;
    hr = g_pDevice->CreateRenderTargetView(pBackBuffer, &RenderTargetViewDesc, &pRenderTargetView);
    if (FAILED(hr)) {
        return 1;
    }
    pBackBuffer->Release();

    g_pDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);

    const float clearColor[4] = { 0.3f, 0.1f, 0.3f, 1.0f };
    g_pDevice->ClearRenderTargetView(pRenderTargetView, clearColor);

    ID3D10VertexShader * pVertexShader;
    hr = g_pDevice->CreateVertexShader(g_VS, sizeof g_VS, &pVertexShader);
    if (FAILED(hr)) {
        return 1;
    }

    struct Vertex {
        float position[4];
        float color[4];
    };

    static const D3D10_INPUT_ELEMENT_DESC InputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, position), D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),    D3D10_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3D10InputLayout *pVertexLayout = NULL;
    hr = g_pDevice->CreateInputLayout(InputElementDescs,
                                      2,
                                      g_VS,
                                      sizeof g_VS,
                                      &pVertexLayout);
    if (FAILED(hr)) {
        return 1;
    }

    g_pDevice->IASetInputLayout(pVertexLayout);

    ID3D10PixelShader * pPixelShader;
    hr = g_pDevice->CreatePixelShader(g_PS, sizeof g_PS, &pPixelShader);
    if (FAILED(hr)) {
        return 1;
    }

    g_pDevice->VSSetShader(pVertexShader);
    g_pDevice->PSSetShader(pPixelShader);

    static const Vertex vertices[] = {
        { { -0.9f, -0.9f, 0.5f, 1.0f}, { 0.8f, 0.0f, 0.0f, 0.1f } },
        { {  0.9f, -0.9f, 0.5f, 1.0f}, { 0.0f, 0.9f, 0.0f, 0.1f } },
        { {  0.0f,  0.9f, 0.5f, 1.0f}, { 0.0f, 0.0f, 0.7f, 0.1f } },
    };

    D3D10_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof BufferDesc);
    BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof vertices;
    BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;

    ID3D10Buffer *pVertexBuffer;
    hr = g_pDevice->CreateBuffer(&BufferDesc, NULL, &pVertexBuffer);
    if (FAILED(hr)) {
        return 1;
    }

    void *pMap = NULL;
    pVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &pMap);
    memcpy(pMap, vertices, sizeof vertices);
    pVertexBuffer->Unmap();

    UINT Stride = sizeof(Vertex);
    UINT Offset = 0;
    g_pDevice->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);
    
    D3D10_VIEWPORT ViewPort;
    ViewPort.TopLeftX = 0;
    ViewPort.TopLeftY = 0;
    ViewPort.Width = WindowWidth;
    ViewPort.Height = WindowHeight;
    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;
    g_pDevice->RSSetViewports(1, &ViewPort);
    
    D3D10_RASTERIZER_DESC RasterizerDesc;
    ZeroMemory(&RasterizerDesc, sizeof RasterizerDesc);
    RasterizerDesc.CullMode = D3D10_CULL_NONE;
    RasterizerDesc.FillMode = D3D10_FILL_SOLID;
    RasterizerDesc.FrontCounterClockwise = true;
    RasterizerDesc.DepthClipEnable = true;
    ID3D10RasterizerState* pRasterizerState = NULL;
    hr = g_pDevice->CreateRasterizerState(&RasterizerDesc, &pRasterizerState);
    if (FAILED(hr)) {
        return 1;
    }
    g_pDevice->RSSetState(pRasterizerState);

    g_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    g_pDevice->Draw(3, 0);

    g_pSwapChain->Present(0, 0);


    ID3D10Buffer *pNullBuffer = NULL;
    UINT NullStride = 0;
    UINT NullOffset = 0;
    g_pDevice->IASetVertexBuffers(0, 1, &pNullBuffer, &NullStride, &NullOffset);
    pVertexBuffer->Release();

    g_pDevice->OMSetRenderTargets(0, NULL, NULL);
    pRenderTargetView->Release();

    g_pDevice->IASetInputLayout(NULL);
    pVertexLayout->Release();

    g_pDevice->VSSetShader(NULL);
    pVertexShader->Release();

    g_pDevice->PSSetShader(NULL);
    pPixelShader->Release();

    g_pDevice->RSSetState(NULL);
    pRasterizerState->Release();

    g_pSwapChain->Release();
    g_pSwapChain = NULL;

    g_pDevice->Release();
    g_pDevice = NULL;

    DestroyWindow(hWnd);

    return 0;
}

