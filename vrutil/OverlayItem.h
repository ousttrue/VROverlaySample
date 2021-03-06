#pragma once
#include <memory>
#include <gsl/span>
#include "D3DManager.h"

namespace vrutil
{

class OverlayItem
{
    class OverlayItemImpl *m_impl;
    OverlayItem(class OverlayItemImpl *impl);

public:
    ~OverlayItem();
    static std::shared_ptr<OverlayItem> Create(uint64_t handle);
    void ProcessEvents();
    void SetTexture(const ComPtr<ID3D11Texture2D> &texture);
    void SetSharedHandle(void *handle);
    void SetOverlayWidthInMeters(float meter);
    void Show();
    void SetMatrix34(const float *matrix34);
};
using OverlayItemPtr = std::shared_ptr<OverlayItem>;

} // namespace vrutil
