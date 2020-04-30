#include <vrutil.h>
#include <DirectXMath.h>
#include "duplicator.h"

int main(int argc, char **argv)
{
    vrutil::OverlayApp app;
    if (!app.Initialize())
    {
        return 1;
    }

    // create shared handle for desktop texture
    DesktopDuplicator dup;
    auto handle = dup.CreateDuplAndSharedHandle();
    if (!handle)
    {
        return 2;
    }

    auto item = app.CreateOverlay("desktopdup", "desktopdup");
    item->SetSharedHandle(handle);
    item->SetOverlayWidthInMeters(4.0f);

    {
        // left side
        using namespace DirectX;
        XMFLOAT3 axis{0, 1, 0};
        auto rot =
            XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(90));
        XMFLOAT3 pos{-3, 1.5f, 0};
        auto translation = XMMatrixTranslationFromVector(XMLoadFloat3(&pos));
        XMFLOAT4X4 matrix44;
        XMStoreFloat4x4(&matrix44, XMMatrixMultiplyTranspose(rot, translation));
        item->SetMatrix34(&matrix44._11);
    }

    while (true)
    {
        item->ProcessEvents();
#if 1
        dup.Duplicate();
#else
        auto texture = dup.Acquire();
        item->SetTexture(texture);
#endif
    }

    return 0;
}
