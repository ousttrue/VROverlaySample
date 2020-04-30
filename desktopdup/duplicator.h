#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class DesktopDuplicator
{
    class DesktopDuplicatorImpl *m_impl = nullptr;

public:
    DesktopDuplicator();
    ~DesktopDuplicator();
    // create dupl and setup desktop size texture for sharing
    void *CreateDuplAndSharedHandle();
    // return false if error
    bool Duplicate();
    void Sleep(int ms);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> Acquire();
};
