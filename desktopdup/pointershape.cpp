#include "pointershape.h"
#include <dxgi1_2.h>

void PointerShape::SetSize(int width, int height)
{
    Width = width;
    Height = height;
    Stride = width * 4;
    RGBA.resize(width * height * 4);
}

bool PointerShape::Update(const Microsoft::WRL::ComPtr<IDXGIOutputDuplication> &dupl, int shapeSize)
{
    m_pointer_in_pixels.resize(shapeSize);
    DXGI_OUTDUPL_POINTER_SHAPE_INFO info;
    UINT required;
    if (FAILED(dupl->GetFramePointerShape((UINT)m_pointer_in_pixels.size(), m_pointer_in_pixels.data(), &required, &info)))
    {
        return false;
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/ne-dxgi1_2-dxgi_outdupl_pointer_shape_type
    // https://github.com/diederickh/screen_capture/blob/master/src/win/ScreenCaptureDuplicateOutputDirect3D11.cpp
    switch (info.Type)
    {
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
    {
        auto img_height = info.Height / 2;
        SetSize(info.Width, img_height);

        int xor_offset = info.Pitch * (info.Height / 2);
        uint8_t *and_map = &m_pointer_in_pixels.front();
        uint8_t *xor_map = &m_pointer_in_pixels.front() + xor_offset;
        uint8_t *out_pixels = &RGBA.front();
        int width_in_bytes = (info.Width + 7) / 8;

        for (uint32_t j = 0; j < img_height; ++j)
        {

            uint8_t bit = 0x80;

            for (uint32_t i = 0; i < info.Width; ++i)
            {

                uint8_t and_byte = and_map[j * width_in_bytes + i / 8];
                uint8_t xor_byte = xor_map[j * width_in_bytes + i / 8];
                uint8_t and_bit = (and_byte & bit) ? 1 : 0;
                uint8_t xor_bit = (xor_byte & bit) ? 1 : 0;
                int out_dx = j * info.Width * 4 + i * 4;

                if (0 == and_bit)
                {
                    if (0 == xor_bit)
                    {
                        /* 0 - 0 = black */
                        out_pixels[out_dx + 0] = 0x00;
                        out_pixels[out_dx + 1] = 0x00;
                        out_pixels[out_dx + 2] = 0x00;
                        out_pixels[out_dx + 3] = 0xFF;
                    }
                    else
                    {
                        /* 0 - 1 = white */
                        out_pixels[out_dx + 0] = 0xFF;
                        out_pixels[out_dx + 1] = 0xFF;
                        out_pixels[out_dx + 2] = 0xFF;
                        out_pixels[out_dx + 3] = 0xFF;
                    }
                }
                else
                {
                    if (0 == xor_bit)
                    {
                        /* 1 - 0 = transparent (screen). */
                        out_pixels[out_dx + 0] = 0x00;
                        out_pixels[out_dx + 1] = 0x00;
                        out_pixels[out_dx + 2] = 0x00;
                        out_pixels[out_dx + 3] = 0x00;
                    }
                    else
                    {
                        /* 1 - 1 = reverse, black. */
                        out_pixels[out_dx + 0] = 0x00;
                        out_pixels[out_dx + 1] = 0x00;
                        out_pixels[out_dx + 2] = 0x00;
                        out_pixels[out_dx + 3] = 0xFF;
                    }
                }

                if (0x01 == bit)
                {
                    bit = 0x80;
                }
                else
                {
                    bit = bit >> 1;
                }
            } /* cols */
        }     /* rows */
        return true;
    }

    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR: // fall through
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
    {
        SetSize(info.Width, info.Height);
        uint8_t *out = &RGBA.front();
        uint8_t *in = &m_pointer_in_pixels.front();
        for (uint32_t j = 0; j < info.Height; ++j, out += info.Pitch, in += info.Pitch)
        {
            memcpy((char *)out, in, info.Pitch);
        }
        return true;
    }
    }

    return false;
}
