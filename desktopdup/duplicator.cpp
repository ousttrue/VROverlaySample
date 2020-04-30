#include "duplicator.h"
#include "quad.h"
#include "pointershape.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <iostream>
#include <memory>
#include <vector>

const auto SCALING_FACTOR = 1.0f;

/// get IDXGIOutput1 for IDXGIOutputDuplication
static Microsoft::WRL::ComPtr<IDXGIOutput1>
GetPrimaryOutput(const Microsoft::WRL::ComPtr<IDXGIDevice> &dxgi)
{
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    if (FAILED(dxgi->GetAdapter(&adapter)))
    {
        return nullptr;
    }

    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
#ifdef _DEBUG
        std::wcout << desc.Description << std::endl;
#endif
    }

    // get output
    Microsoft::WRL::ComPtr<IDXGIOutput> output;
    if (FAILED(adapter->EnumOutputs(0, &output)))
    {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IDXGIOutput1> output1;
    if (FAILED(output.As(&output1)))
    {
        return nullptr;
    }

    return output1;
}

class DesktopDuplicatorImpl
{
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> m_dupl;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_shared;
    float m_scalingFactor;

public:
    DesktopDuplicatorImpl(float scaling = 1.0f) : m_scalingFactor(scaling)
    {
    }

    // create dupl and setup desktop size texture for sharing
    HANDLE CreateDuplAndSharedHandle()
    {
        // create d3d11
        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_0,
        };
        D3D_FEATURE_LEVEL level;
        if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                     D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels,
                                     _countof(levels), D3D11_SDK_VERSION,
                                     &m_device, &level, &m_context)))
        {
            return nullptr;
        }

        // get dxgi from d3d11
        Microsoft::WRL::ComPtr<IDXGIDevice> dxgi;
        if (FAILED(m_device.As(&dxgi)))
        {
            return nullptr;
        }

        // get output1 from dxgi
        auto output = GetPrimaryOutput(dxgi);
        if (!output)
        {
            return nullptr;
        }

        {
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);
#ifdef _DEBUG
            std::wcout << desc.DeviceName << std::endl;
#endif
        }
        if (FAILED(output->DuplicateOutput(m_device.Get(), &m_dupl)))
        {
            return nullptr;
        }

        DXGI_OUTDUPL_DESC duplDesc;
        m_dupl->GetDesc(&duplDesc);

        // create shared texture
        D3D11_TEXTURE2D_DESC desc = {0};
        desc.Format = duplDesc.ModeDesc.Format;
        desc.Width =
            static_cast<UINT>(duplDesc.ModeDesc.Width * m_scalingFactor);
        desc.Height =
            static_cast<UINT>(duplDesc.ModeDesc.Height * m_scalingFactor);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        if (FAILED(m_device->CreateTexture2D(&desc, nullptr, &m_shared)))
        {
            return nullptr;
        }

        // create shared handle
        Microsoft::WRL::ComPtr<IDXGIResource> sharedResource;
        if (FAILED(m_shared.As(&sharedResource)))
        {
            return nullptr;
        }

        HANDLE handle;
        if (FAILED(sharedResource->GetSharedHandle(&handle)))
        {
            return nullptr;
        }

        // set overlay
        return handle;
    }

    // return false if error
    bool Duplicate()
    {
        m_dupl->ReleaseFrame();

        DXGI_OUTDUPL_FRAME_INFO info;
        Microsoft::WRL::ComPtr<IDXGIResource> resource;
        auto hr = m_dupl->AcquireNextFrame(INFINITE, &info, &resource);
        switch (hr)
        {
        case S_OK:
        {
            Microsoft::WRL::ComPtr<ID3D11Texture2D> duplTexture;
            if (FAILED(resource.As(&duplTexture)))
            {
                return false;
            }

            // copy duplTexture to shared
#if 0
            m_context->CopyResource(m_shared.Get(), duplTexture.Get());
#else
            if (!CreateRendererIfEmpty(m_shared))
            {
                return false;
            }

            m_renderer->Render(m_context, duplTexture);
            if (info.PointerPosition.Visible)
            {
                m_x = info.PointerPosition.Position.x;
                m_y = info.PointerPosition.Position.y;
                m_cursor = GetCursorTexture(info);
            }
            if (m_cursor)
            {
                m_renderer->Render(m_context, m_cursor, m_x, m_y);
            }
#endif
            break;
        }

        case DXGI_ERROR_WAIT_TIMEOUT:
            break;

        case DXGI_ERROR_ACCESS_LOST:
            return false;

        case DXGI_ERROR_INVALID_CALL:
            // not released previous frame
            return false;

        default:
            return false;
        }

        return true;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> Acquire()
    {
        m_dupl->ReleaseFrame();

        DXGI_OUTDUPL_FRAME_INFO info;
        Microsoft::WRL::ComPtr<IDXGIResource> resource;
        auto hr = m_dupl->AcquireNextFrame(INFINITE, &info, &resource);
        switch (hr)
        {
        case S_OK:
        {
            Microsoft::WRL::ComPtr<ID3D11Texture2D> duplTexture;
            if (SUCCEEDED(resource.As(&duplTexture)))
            {
                return duplTexture;
            }
            break;
        }

        case DXGI_ERROR_WAIT_TIMEOUT:
            break;

        case DXGI_ERROR_ACCESS_LOST:
            break;

        case DXGI_ERROR_INVALID_CALL:
            // not released previous frame
            break;

        default:
            break;
        }

        return nullptr;
    }

private:
    std::unique_ptr<QuadRenderer> m_renderer;
    bool
    CreateRendererIfEmpty(const Microsoft::WRL::ComPtr<ID3D11Texture2D> &dst)
    {
        if (!m_renderer)
        {
            auto renderer = std::make_unique<QuadRenderer>();
            if (!renderer->Initialize(m_device, dst, m_scalingFactor))
            {
                return false;
            }
            m_renderer = std::move(renderer);
        }
        return true;
    }

    PointerShape m_shape;
    int m_x;
    int m_y;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_cursor;

    Microsoft::WRL::ComPtr<ID3D11Texture2D>
    GetCursorTexture(const DXGI_OUTDUPL_FRAME_INFO &frameInfo)
    {
        if (frameInfo.PointerShapeBufferSize)
        {
            // Get shape
            if (m_shape.Update(m_dupl, frameInfo.PointerShapeBufferSize))
            {
                D3D11_TEXTURE2D_DESC desc;
                desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                desc.Width = m_shape.Width, desc.Height = m_shape.Height;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags = 0;
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                desc.MipLevels = 1;
                desc.ArraySize = 1;

                D3D11_SUBRESOURCE_DATA data;
                data.SysMemPitch = m_shape.Stride;
                data.pSysMem = m_shape.RGBA.data();
                data.SysMemSlicePitch = 0;

                if (FAILED(m_device->CreateTexture2D(&desc, &data, &m_cursor)))
                {
                    return nullptr;
                }
                // std::cout << "create cursor texture" << std::endl;
            }
        }

        return m_cursor;
    }
};

DesktopDuplicator::DesktopDuplicator()
    : m_impl(new DesktopDuplicatorImpl(SCALING_FACTOR))
{
}

DesktopDuplicator::~DesktopDuplicator()
{
    delete m_impl;
}

// create dupl and setup desktop size texture for sharing
void *DesktopDuplicator::CreateDuplAndSharedHandle()
{
    return m_impl->CreateDuplAndSharedHandle();
}

// return false if error
bool DesktopDuplicator::Duplicate()
{
    return m_impl->Duplicate();
}

void DesktopDuplicator::Sleep(int ms)
{
    ::Sleep(ms);
}

Microsoft::WRL::ComPtr<ID3D11Texture2D> DesktopDuplicator::Acquire()
{
    return m_impl->Acquire();
}
