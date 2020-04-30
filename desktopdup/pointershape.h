#pragma once
#include <vector>
#include <stdint.h>
#include <wrl/client.h>

struct IDXGIOutputDuplication;
class PointerShape
{
    std::vector<uint8_t> m_pointer_in_pixels;

public:
    bool Update(const Microsoft::WRL::ComPtr<IDXGIOutputDuplication> &dupl, int shapeSize);
    int Width = 0;
    int Height = 0;
    int Stride = 0;
    std::vector<uint8_t> RGBA;
    void SetSize(int width, int height);
};
