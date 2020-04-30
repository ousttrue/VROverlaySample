#include <vrutil.h>
#include <stdint.h>
#include <iostream>
#include <DirectXMath.h>

class FpsTimer
{
    uint32_t m_frameTime;
    uint32_t m_begin;

public:
    FpsTimer(uint32_t frameRate) { m_frameTime = 1000 / frameRate - 5; }
    void NewFrame() { m_begin = timeGetTime(); }
    void WaitNextFrame()
    {
        auto now = timeGetTime();
        auto delta = now - m_begin;
        if (delta < m_frameTime)
        {
            Sleep(m_frameTime - delta);
        }
    }
};

int main(int argc, char **argv)
{
    vrutil::OverlayApp app;
    if (!app.Initialize())
    {
        return 1;
    }

    vrutil::D3DManager d3d;
    if (!d3d.Initialize())
    {
        return 2;
    }

    auto xAxis = app.CreateOverlay("x_axis", "x_axis");
    uint8_t xPixels[] = {
        255, 255, 255, 255, // 00
        255, 0,   0,   255, // 10
    };
    auto xTexture = d3d.CreateStaticTexture(2, 1, xPixels);
    xAxis->Show();
    xAxis->SetOverlayWidthInMeters(0.4f);
    {
        using namespace DirectX;
        XMFLOAT3 axis{1, 0, 0};
        auto rot =
            XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(-90));
        XMFLOAT3 pos{0, 0, 0};
        auto translation = XMMatrixTranslationFromVector(XMLoadFloat3(&pos));
        XMFLOAT4X4 matrix44;
        XMStoreFloat4x4(&matrix44, XMMatrixMultiplyTranspose(rot, translation));
        xAxis->SetMatrix34(&matrix44._11);
    }

    auto zAxis = app.CreateOverlay("z_axis", "z_axis");
    uint8_t zPixels[] = {
        255, 255, 255, 255, // 00
        0,   0,   255, 255, // 10
    };
    auto zTexture = d3d.CreateStaticTexture(2, 1, zPixels);
    zAxis->Show();
    zAxis->SetOverlayWidthInMeters(0.4f);
    {
        using namespace DirectX;
        XMFLOAT3 axis{1, 0, 0};
        auto rot0 =
            XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(-90));
        
        XMFLOAT3 yaxis{0, 1, 0};
        auto rot1 = 
            XMMatrixRotationAxis(XMLoadFloat3(&yaxis), XMConvertToRadians(90));

        auto rot = XMMatrixMultiply(rot0, rot1);

        XMFLOAT3 pos{0, 0, 0};       
        auto translation = XMMatrixTranslationFromVector(XMLoadFloat3(&pos));
        XMFLOAT4X4 matrix44;
        XMStoreFloat4x4(&matrix44, XMMatrixMultiplyTranspose(rot, translation));
        zAxis->SetMatrix34(&matrix44._11);
    }

    // main loop
    FpsTimer timer(80);
    while (true)
    {
        timer.NewFrame();
        xAxis->ProcessEvents();
        xAxis->SetTexture(xTexture); // require each frame ?
        zAxis->ProcessEvents();
        zAxis->SetTexture(zTexture); // require each frame ?
        timer.WaitNextFrame();
    }

    return 0;
}
