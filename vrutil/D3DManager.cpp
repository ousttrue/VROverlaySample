#include "D3DManager.h"
#include <list>
#include <assert.h>

namespace vrutil
{

class D3D11ManagerImpl
{
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;

public:
    bool Initialize()
    {
        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT
#if DEBUG
                     | D3D11_CREATE_DEVICE_DEBUG
#endif
            ;

        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_0,
        };
        D3D_FEATURE_LEVEL level;
        if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                     flags, levels, _countof(levels),
                                     D3D11_SDK_VERSION, &m_device, &level,
                                     &m_context)))
        {
            return false;
        }

        return true;
    }

    ComPtr<ID3D11Texture2D> CreateStaticTexture(UINT width, UINT height,
                                                const gsl::span<uint8_t> &bytes)
    {
        assert(width * height * 4 == bytes.size());
        D3D11_TEXTURE2D_DESC desc{
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = {1, 0},
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };
        D3D11_SUBRESOURCE_DATA data{
            .pSysMem = bytes.data(),
            .SysMemPitch = width * 4,
            .SysMemSlicePitch = static_cast<UINT>(bytes.size()),
        };
        ComPtr<ID3D11Texture2D> texture;
        if (FAILED(m_device->CreateTexture2D(&desc, &data, &texture)))
        {
            return nullptr;
        }

        return texture;
    }

    SharedTexture CreateSharedStaticTexture(UINT width, UINT height,
                                            const gsl::span<uint8_t> &bytes)
    {
        assert(width * height * 4 == bytes.size());
        D3D11_TEXTURE2D_DESC desc{
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = {1, 0},
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
            .CPUAccessFlags = 0,
            .MiscFlags = D3D11_RESOURCE_MISC_SHARED, // shared
        };
        D3D11_SUBRESOURCE_DATA data{
            .pSysMem = bytes.data(),
            .SysMemPitch = width * 4,
            .SysMemSlicePitch = static_cast<UINT>(bytes.size()),
        };
        ComPtr<ID3D11Texture2D> texture;
        if (FAILED(m_device->CreateTexture2D(&desc, nullptr, &texture)))
        {
            return SharedTexture();
        }

        // create shared handle
        Microsoft::WRL::ComPtr<IDXGIResource> sharedResource;
        if (FAILED(texture.As(&sharedResource)))
        {
            return SharedTexture();
        }
        HANDLE handle;
        if (FAILED(sharedResource->GetSharedHandle(&handle)))
        {
            return SharedTexture();
        }

        return std::make_pair(texture, handle);
    }

    void Copy(const ComPtr<ID3D11Texture2D> &src,
              const ComPtr<ID3D11Texture2D> &dst)
    {
        m_context->CopyResource(dst.Get(), src.Get());
    }
};

D3DManager::D3DManager() : m_impl(new D3D11ManagerImpl) {}

D3DManager::~D3DManager() { delete m_impl; }

bool D3DManager::Initialize() { return m_impl->Initialize(); }

ComPtr<ID3D11Texture2D>
D3DManager::CreateStaticTexture(int w, int h, const gsl::span<uint8_t> &bytes)
{
    return m_impl->CreateStaticTexture(w, h, bytes);
}

SharedTexture
D3DManager::CreateSharedStaticTexture(int w, int h,
                                      const gsl::span<uint8_t> &bytes)
{
    return m_impl->CreateSharedStaticTexture(w, h, bytes);
}

void D3DManager::Copy(const ComPtr<ID3D11Texture2D> &src,
                      const ComPtr<ID3D11Texture2D> &dst)
{
    m_impl->Copy(src, dst);
}

} // namespace vrutil