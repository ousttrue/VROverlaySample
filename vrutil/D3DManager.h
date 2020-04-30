#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <gsl/span>

namespace vrutil
{
template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
using SharedTexture = std::pair<ComPtr<ID3D11Texture2D>, HANDLE>;

class D3DManager
{
    class D3D11ManagerImpl *m_impl;

public:
    D3DManager();
    ~D3DManager();
    bool Initialize();
    ComPtr<ID3D11Texture2D> CreateStaticTexture(int w, int h, const gsl::span<uint8_t> &bytes);
    SharedTexture CreateSharedStaticTexture(int w, int h, const gsl::span<uint8_t> &bytes);
    void Copy(const ComPtr<ID3D11Texture2D> &src, const ComPtr<ID3D11Texture2D> &dst);
};

} // namespace vrutil