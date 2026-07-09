// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>

#include <QString>
#include <QVector>
#include <cstdint>

struct ViaKeyboardInfo {
    QString path;
    QString manufacturer;
    QString product;
    quint16 vid = 0;
    quint16 pid = 0;
};

class SONICVIA_EXPORT ViaDiscovery
{
public:
    // Lists all VIA keyboards (usage page 0xff60). Pure enumeration — does NOT probe-open.
    // Empty when built as a stub or when no VIA device is present. Access is checked by the
    // caller via ViaDevice::open().
    static QVector<ViaKeyboardInfo> discoverViaKeyboards();
};
