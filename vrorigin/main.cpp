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

    auto item = app.CreateOverlay("vr_origin", "vr_origin");
    uint8_t pixels[] = {
        255, 255, 255, 255, // 00
        255, 255, 255, 255, // 10
        255, 255, 255, 255, // 01
        255, 255, 255, 255, // 11
    };

    auto texture = d3d.CreateStaticTexture(2, 2, pixels);
    item->Show();
    item->SetOverlayWidthInMeters(1.0f);
    float axis[] = {0, 1, 0};
    float position[] = {0, 0, 0};
    item->AxisAnglePosition(axis, 0, position);

    // main loop
    FpsTimer timer(80);
    while (true)
    {
        timer.NewFrame();
        item->ProcessEvents();
        item->SetTexture(texture); // require each frame ?
        timer.WaitNextFrame();
    }

    return 0;
}
