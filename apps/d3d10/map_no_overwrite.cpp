/**************************************************************************
 *
 * Copyright 2014 VMware, Inc.
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


/*
 * Test case for usage of D3D10_MAP_WRITE_DISCARD with D3D10_MAP_WRITE_NO_OVERWRITE
 * http://msdn.microsoft.com/en-us/library/windows/desktop/bb205318.aspx#NO_OVERWRITE_DETAILS
 *
 */
#include <stdio.h>
#include <stddef.h>

#include <initguid.h>
#include <windows.h>

#include "compat.h"

#include <d3d10.h>

#include "com_ptr.hpp"


int
main(int argc, char *argv[])
{
    HRESULT hr;

    com_ptr<IDXGIFactory> pFactory;
    hr = CreateDXGIFactory(IID_IDXGIFactory, (void**)&pFactory);
    if (FAILED(hr)) {
        return 1;
    }

    com_ptr<IDXGIAdapter> pAdapter;
    hr = pFactory->EnumAdapters(0, &pAdapter);
    if (FAILED(hr)) {
        return 1;
    }

    UINT Flags = 0;
    if (LoadLibraryA("d3d10sdklayers")) {
        Flags |= D3D10_CREATE_DEVICE_DEBUG;
    }

    com_ptr<ID3D10Device> pDevice;
    hr = D3D10CreateDevice(pAdapter,
                           D3D10_DRIVER_TYPE_HARDWARE,
                           NULL,
                           Flags,
                           D3D10_SDK_VERSION,
                           &pDevice);
    if (FAILED(hr)) {
        return 1;
    }

    UINT NumSegments = 8;
    UINT SegmentSize = 512;

    D3D10_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof BufferDesc);
    BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = NumSegments * SegmentSize;
    BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;

    com_ptr<ID3D10Buffer> pVertexBuffer;
    hr = pDevice->CreateBuffer(&BufferDesc, NULL, &pVertexBuffer);
    if (FAILED(hr)) {
        return 1;
    }

    for (UINT j = 0; j < NumSegments; ++j) {
        D3D10_MAP MapType = j == 0 ? D3D10_MAP_WRITE_DISCARD : D3D10_MAP_WRITE_NO_OVERWRITE;
        BYTE *pMap = NULL;
        hr = pVertexBuffer->Map(MapType, 0, (void **)&pMap);
        if (FAILED(hr)) {
            return 1;
        } 

        int c = (j % 255) + 1;
        memset(pMap + j*SegmentSize, c, SegmentSize);

        pVertexBuffer->Unmap();
    }

    return 0;
}

