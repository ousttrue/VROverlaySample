#pragma once
#include "OverlayItem.h"

namespace vrutil
{

class OverlayApp
{
    class OverlayAppImpl *m_impl;

public:
    OverlayApp();
    ~OverlayApp();
    bool Initialize();
    OverlayItemPtr CreateOverlay(const char *key, const char *friendlyName);
};

} // namespace vrutil
