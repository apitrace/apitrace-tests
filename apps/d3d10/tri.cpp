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


#include <initguid.h>

#include <windows.h>

#include "compat.h"

#include <d3d10.h>
#include <d3d10shader.h>


static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D10Device * g_pDevice = NULL;


int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    HRESULT hr;

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

    hr = D3D10CreateDeviceAndSwapChain(NULL,
                                       D3D10_DRIVER_TYPE_HARDWARE,
                                       NULL,
                                       0,
                                       D3D10_SDK_VERSION,
                                       &SwapChainDesc,
                                       &g_pSwapChain,
                                       &g_pDevice);
    if (FAILED(hr)) {
        return 1;
    }

    ID3D10RenderTargetView *pRenderTargetView = NULL;
    ID3D10Texture2D* pBackBuffer;
    hr = g_pSwapChain->GetBuffer(0, IID_ID3D10Texture2D, (void **)&pBackBuffer);
    if (FAILED(hr)) {
        return 1;
    }
    hr = g_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
    if (FAILED(hr)) {
        return 1;
    }
    pBackBuffer->Release();

    g_pDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);

    const float clearColor[4] = { 0.3f, 0.1f, 0.3f, 1.0f };
    g_pDevice->ClearRenderTargetView(pRenderTargetView, clearColor);

    const char *szShader = 
        "struct VS_OUTPUT {\n"
        "    float4 Pos : SV_POSITION;\n"
        "    float4 Color : COLOR0;\n"
        "};\n"
        "\n"
        "VS_OUTPUT VertexShader(float4 Pos : POSITION, float4 Color : COLOR) {\n"
        "    VS_OUTPUT Out;\n"
        "    Out.Pos = Pos;\n"
        "    Out.Color = Color;\n"
        "    return Out;\n"
        "}\n"
        "\n"
        "float4 PixelShader(VS_OUTPUT In) : SV_Target {\n"
        "    return In.Color;\n"
        "}\n"
    ;

    ID3D10Blob *pVertexShaderBlob;
    hr = D3D10CompileShader(szShader, strlen(szShader), NULL, NULL, NULL, "VertexShader", "vs_4_0", 0, &pVertexShaderBlob, NULL);

    ID3D10VertexShader * pVertexShader;
    hr = g_pDevice->CreateVertexShader((DWORD*)pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), &pVertexShader);

    struct Vertex {
        float x, y, z;
        float r, g, b, a;
    };

    D3D10_INPUT_ELEMENT_DESC InputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3D10InputLayout *pVertexLayout = NULL;
    hr = g_pDevice->CreateInputLayout(InputElementDescs,
                                      2,
                                      pVertexShaderBlob->GetBufferPointer(),
                                      pVertexShaderBlob->GetBufferSize(),
                                      &pVertexLayout);
    pVertexShaderBlob->Release();

    g_pDevice->IASetInputLayout(pVertexLayout);


    ID3D10Blob *pPixelShaderBlob;
    hr = D3D10CompileShader(szShader, strlen(szShader), NULL, NULL, NULL, "PixelShader", "ps_4_0", 0, &pPixelShaderBlob, NULL);

    ID3D10PixelShader * pPixelShader;
    hr = g_pDevice->CreatePixelShader((DWORD*)pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), &pPixelShader);
    pPixelShaderBlob->Release();


    g_pDevice->VSSetShader(pVertexShader);
    g_pDevice->GSSetShader(NULL);
    g_pDevice->PSSetShader(pPixelShader);

    static const Vertex vertices[] = {
        { -0.9f, -0.9f, 0.5f,  0.8f, 0.0f, 0.0f, 0.1f },
        {  0.9f, -0.9f, 0.5f,  0.0f, 0.9f, 0.0f, 0.1f },
        {  0.0f,  0.9f, 0.5f,  0.0f, 0.0f, 0.7f, 0.1f },
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
    ID3D10RasterizerState* pRasterizerState;
    g_pDevice->CreateRasterizerState(&RasterizerDesc, &pRasterizerState);
    g_pDevice->RSSetState(pRasterizerState);

    g_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    g_pDevice->Draw(3, 0);

    g_pSwapChain->Present(0, 0);

    g_pSwapChain->Release();
    g_pSwapChain = NULL;

    g_pDevice->Release();
    g_pDevice = NULL;

    DestroyWindow(hWnd);

    return 0;
}

