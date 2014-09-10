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


#include <stddef.h>

#include <windows.h>

#include <d3d9.h>

#include "tri_vs_2_0.h"
#include "tri_ps_2_0.h"


static IDirect3D9 * g_pD3D = NULL;
static IDirect3DDevice9 * g_pDevice = NULL;
static D3DPRESENT_PARAMETERS g_PresentationParameters;


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
        "SimpleDX9",
        NULL
    };
    RegisterClassEx(&wc);

    const int WindowWidth = 250;
    const int WindowHeight = 250;

    DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    RECT rect = {0, 0, WindowWidth, WindowHeight};
    AdjustWindowRect(&rect, dwStyle, FALSE);

    HWND hWnd = CreateWindow(wc.lpszClassName,
                             "Simple example using DirectX9",
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

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_pD3D) {
        return 1;
    }

    D3DCAPS9 caps;
    hr = g_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
    if (FAILED(hr)) {
       return 1;
    }

    DWORD dwBehaviorFlags;
    if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
        caps.VertexShaderVersion < D3DVS_VERSION(1, 1)) {
       dwBehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    } else {
       dwBehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }

    ZeroMemory(&g_PresentationParameters, sizeof g_PresentationParameters);
    g_PresentationParameters.Windowed = TRUE;
    g_PresentationParameters.BackBufferCount = 1;
    g_PresentationParameters.SwapEffect = D3DSWAPEFFECT_FLIP;
    g_PresentationParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
    g_PresentationParameters.hDeviceWindow = hWnd;

    g_PresentationParameters.EnableAutoDepthStencil = FALSE;
    g_PresentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                              D3DDEVTYPE_HAL,
                              hWnd,
                              dwBehaviorFlags,
                              &g_PresentationParameters,
                              &g_pDevice);
    if (FAILED(hr)) {
        return 1;
    }

    D3DCOLOR clearColor = D3DCOLOR_COLORVALUE(0.3f, 0.1f, 0.3f, 1.0f);
    g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, clearColor, 1.0f, 0);

    struct Vertex {
        float position[4];
        float color[4];
    };

    static const Vertex vertices[] = {
        { { -0.9f, -0.9f, 0.5f, 1.0f}, { 0.8f, 0.0f, 0.0f, 0.1f } },
        { {  0.9f, -0.9f, 0.5f, 1.0f}, { 0.0f, 0.9f, 0.0f, 0.1f } },
        { {  0.0f,  0.9f, 0.5f, 1.0f}, { 0.0f, 0.0f, 0.7f, 0.1f } },
    };

    static const D3DVERTEXELEMENT9 VertexElements[] = {
        { 0, offsetof(Vertex, position), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, offsetof(Vertex, color),    D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        D3DDECL_END()
    };

    LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration = NULL;
    hr = g_pDevice->CreateVertexDeclaration(VertexElements, &pVertexDeclaration);
    if (FAILED(hr)) {
        return 1;
    }
    g_pDevice->SetVertexDeclaration(pVertexDeclaration);

    LPDIRECT3DVERTEXSHADER9 pVertexShader = NULL;
    hr = g_pDevice->CreateVertexShader((CONST DWORD *)g_vs20_VS, &pVertexShader);
    if (FAILED(hr)) {
        return 1;
    }
    g_pDevice->SetVertexShader(pVertexShader);

    LPDIRECT3DPIXELSHADER9 pPixelShader = NULL;
    hr = g_pDevice->CreatePixelShader((CONST DWORD *)g_ps20_PS, &pPixelShader);
    if (FAILED(hr)) {
        return 1;
    }
    g_pDevice->SetPixelShader(pPixelShader);

    g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    g_pDevice->BeginScene();

    g_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertices, sizeof(Vertex));

    g_pDevice->EndScene();

    g_pDevice->Present(NULL, NULL, NULL, NULL);

    pPixelShader->Release();
    pPixelShader = NULL;

    pVertexShader->Release();
    pVertexShader = NULL;

    pVertexDeclaration->Release();
    pVertexDeclaration = NULL;

    g_pDevice->Release();
    g_pDevice = NULL;

    g_pD3D->Release();
    g_pD3D = NULL;

    DestroyWindow(hWnd);

    return 0;
}

