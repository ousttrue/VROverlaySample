#include <vrutil.h>
#include <stdint.h>
#include <iostream>

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

    auto zAxis = app.CreateOverlay("z_axis", "z_axis");
    uint8_t zPixels[] = {
        255, 255, 255, 255, // 00
        0,   0,   255, 255, // 10
    };
    auto zTexture = d3d.CreateStaticTexture(2, 1, zPixels);
    zAxis->Show();
    zAxis->SetOverlayWidthInMeters(0.4f);

    float axis[] = {0, 1, 0};
    float position[] = {0, 0.1f, 0}; // 0.4f/2/2
    xAxis->AxisAnglePosition(axis, 0, position);
    zAxis->AxisAnglePosition(axis, 90, position);

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
