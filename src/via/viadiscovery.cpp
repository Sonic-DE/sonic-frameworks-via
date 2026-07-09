// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include "viadiscovery.h"
#include "sonicvia_debug.h"

#ifdef HAVE_VIA_SUPPORT
#include <hidapi/hidapi.h>
#endif

QVector<ViaKeyboardInfo> ViaDiscovery::discoverViaKeyboards()
{
#ifdef HAVE_VIA_SUPPORT
    QVector<ViaKeyboardInfo> keyboards;

    // VIA keyboards expose usage page 0xff60. hid_enumerate filters by USB vendor/product id,
    // so enumerate everything (0/0) and filter on usage_page ourselves, matching the Rust
    // scan_keyboards() behavior.
    constexpr unsigned short kViaUsagePage = 0xff60;

    hid_device_info *enumeration = hid_enumerate(0x00, 0x00);
    for (hid_device_info *cur = enumeration; cur; cur = cur->next) {
        if (cur->usage_page != kViaUsagePage) {
            continue;
        }
        ViaKeyboardInfo info;
        info.path = QString::fromUtf8(cur->path);
        info.manufacturer = QString::fromWCharArray(cur->manufacturer_string);
        info.product = QString::fromWCharArray(cur->product_string);
        info.vid = cur->vendor_id;
        info.pid = cur->product_id;
        keyboards.append(std::move(info));
    }
    hid_free_enumeration(enumeration);

    qCDebug(SONICVIA) << "Discovered" << keyboards.size() << "VIA keyboard(s)";
    return keyboards;
#else
    qCDebug(SONICVIA) << "Stub: VIA support not available";
    return {};
#endif
}
