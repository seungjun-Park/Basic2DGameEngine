#include "Texture.h"

#include <Windows.h>
#include <wincodec.h>

#include <vector>

#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;

bool Texture::Load(
    ID3D11Device* device,
    const std::wstring& path)
{
    HRESULT hr;

    ComPtr<IWICImagingFactory> factory;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(
            factory.GetAddressOf()
        )
    );

    if (FAILED(hr))
        return false;

    ComPtr<IWICBitmapDecoder> decoder;

    hr =
        factory->CreateDecoderFromFilename(
            path.c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            decoder.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    ComPtr<IWICBitmapFrameDecode> frame;

    hr =
        decoder->GetFrame(
            0,
            frame.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    UINT width = 0;
    UINT height = 0;

    frame->GetSize(
        &width,
        &height
    );

    ComPtr<IWICFormatConverter> converter;

    hr =
        factory->CreateFormatConverter(
            converter.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    hr =
        converter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom
        );

    if (FAILED(hr))
        return false;

    std::vector<unsigned char> pixels(
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        4
    );

    hr =
        converter->CopyPixels(
            nullptr,
            width * 4,
            static_cast<UINT>(
                pixels.size()
                ),
            pixels.data()
        );

    if (FAILED(hr))
        return false;

    D3D11_TEXTURE2D_DESC textureDesc{};

    textureDesc.Width = width;
    textureDesc.Height = height;

    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;

    textureDesc.Format =
        DXGI_FORMAT_R8G8B8A8_UNORM;

    textureDesc.SampleDesc.Count = 1;

    textureDesc.Usage =
        D3D11_USAGE_DEFAULT;

    textureDesc.BindFlags =
        D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData{};

    textureData.pSysMem =
        pixels.data();

    textureData.SysMemPitch =
        width * 4;

    ComPtr<ID3D11Texture2D> texture;

    hr =
        device->CreateTexture2D(
            &textureDesc,
            &textureData,
            texture.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};

    srvDesc.Format =
        textureDesc.Format;

    srvDesc.ViewDimension =
        D3D11_SRV_DIMENSION_TEXTURE2D;

    srvDesc.Texture2D.MipLevels = 1;

    hr =
        device->CreateShaderResourceView(
            texture.Get(),
            &srvDesc,
            m_shaderResourceView.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    m_width =
        static_cast<int>(width);

    m_height =
        static_cast<int>(height);

    return true;
}