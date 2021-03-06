#include "OverlayItem.h"
#include <openvr.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <iostream>
#include <DirectXMath.h>

namespace vrutil
{

template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

class OverlayItemImpl
{
    vr::VROverlayHandle_t m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    vr::IVROverlay *m_overlay;

public:
    OverlayItemImpl(uint64_t handle)
        : m_overlayHandle(handle), m_overlay(vr::VROverlay())
    {
        m_overlay->SetOverlayAlpha(m_overlayHandle, 1.0f);
        m_overlay->SetOverlayColor(m_overlayHandle, 1.0f, 1.0f, 1.0f);
        m_overlay->ShowOverlay(m_overlayHandle);
    }

    ~OverlayItemImpl()
    {
        m_overlay->ClearOverlayTexture(m_overlayHandle);
        m_overlay->DestroyOverlay(m_overlayHandle);
    }

    bool SetTexture(const ComPtr<ID3D11Texture2D> &texture2D)
    {
        vr::Texture_t texture = {texture2D.Get(), vr::TextureType_DirectX,
                                 vr::ColorSpace_Auto};
        auto error = m_overlay->SetOverlayTexture(m_overlayHandle, &texture);
        if (error != vr::EVROverlayError::VROverlayError_None)
        {
            std::cerr << "fail to SetOverlayTexture: " << error << std::endl;
            return false;
        }

        return true;
    }

    bool SetSharedHandle(HANDLE handle)
    {
        vr::Texture_t texture = {handle, vr::TextureType_DXGISharedHandle,
                                 vr::ColorSpace_Auto};
        auto error = m_overlay->SetOverlayTexture(m_overlayHandle, &texture);
        if (error != vr::EVROverlayError::VROverlayError_None)
        {
            std::cerr << "fail to SetOverlayTexture: " << error << std::endl;
            return false;
        }

        return true;
    }

    void SetWidthInMeters(float meter)
    {
        m_overlay->SetOverlayWidthInMeters(m_overlayHandle, meter);
    }

    bool ProcessEvents()
    {
        vr::VREvent_t Event;
        while (m_overlay->PollNextOverlayEvent(m_overlayHandle, &Event,
                                               sizeof(Event)))
        {
            std::cerr << Event.eventType << std::endl;

            switch (Event.eventType)
            {
            case vr::VREvent_MouseMove:
                break;
            case vr::VREvent_MouseButtonDown:
                break;
            case vr::VREvent_OverlayShown:
                return false;
            }
        }
        return true;
    }

    void Show() { m_overlay->ShowOverlay(m_overlayHandle); }

    void SetMatrix34(const float *matrix34)
    {
        m_overlay->SetOverlayTransformAbsolute(m_overlayHandle,
                                               vr::TrackingUniverseStanding,
                                               (vr::HmdMatrix34_t *)matrix34);
    }
}; // namespace vrutil

OverlayItem::OverlayItem(OverlayItemImpl *impl) : m_impl(impl) {}

OverlayItem::~OverlayItem() { delete m_impl; }

std::shared_ptr<OverlayItem> OverlayItem::Create(uint64_t handle)
{
    auto impl = new OverlayItemImpl(handle);
    auto item = new OverlayItem(impl);
    return std::shared_ptr<OverlayItem>(item);
}

void OverlayItem::ProcessEvents() { m_impl->ProcessEvents(); }

void OverlayItem::SetTexture(const ComPtr<ID3D11Texture2D> &texture)
{
    m_impl->SetTexture(texture);
}

void OverlayItem::SetSharedHandle(void *handle)
{
    m_impl->SetSharedHandle(handle);
}

void OverlayItem::SetOverlayWidthInMeters(float meter)
{
    m_impl->SetWidthInMeters(meter);
}

void OverlayItem::Show() { m_impl->Show(); }

void OverlayItem::SetMatrix34(const float *matrix34)
{
    m_impl->SetMatrix34(matrix34);
}

} // namespace vrutil
