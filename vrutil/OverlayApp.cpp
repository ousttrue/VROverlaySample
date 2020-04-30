#include "OverlayApp.h"
#include <openvr.h>
#include <list>

namespace vrutil
{

class OverlayAppImpl
{
    vr::IVROverlay *m_overlay = nullptr;

    std::list<OverlayItemPtr> m_items;

public:
    bool Initialize()
    {
        vr::EVRInitError initError = vr::VRInitError_None;
        vr::VR_Init(&initError, vr::VRApplication_Overlay);
        if (initError != vr::VRInitError_None)
        {
            return false;
        }

        m_overlay = vr::VROverlay();
        return m_overlay != nullptr;
    }

    OverlayItemPtr CreateItem(const char *key, const char *friendlyName)
    {
        vr::VROverlayHandle_t handle = vr::k_ulOverlayHandleInvalid;
        vr::VROverlayError overlayError =
            vr::VROverlay()->CreateOverlay(key, friendlyName, &handle);
        if (overlayError != vr::VROverlayError_None)
        {
            return nullptr;
        }
        return OverlayItem::Create(handle);
    }
};

OverlayApp::OverlayApp() : m_impl(new OverlayAppImpl) {}


OverlayApp::~OverlayApp() { delete m_impl; }

bool OverlayApp::Initialize() { return m_impl->Initialize(); }

OverlayItemPtr OverlayApp::CreateOverlay(const char *key,
                                         const char *friendlyName)
{
    return m_impl->CreateItem(key, friendlyName);
}

} // namespace  vrutil
