// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>

#include <QString>
#include <QVector>
#include <cstdint>

/*!
 * \class ViaKeyboardInfo
 * \inmodule SonicVia
 * \inheaderfile via/viadiscovery.h
 *
 * \brief A VIA keyboard found by ViaDiscovery.
 */
struct ViaKeyboardInfo {
    /*!
     * \variable ViaKeyboardInfo::path
     * The HID path, for ViaDevice.
     */
    QString path;
    /*!
     * \variable ViaKeyboardInfo::manufacturer
     * The manufacturer string.
     */
    QString manufacturer;
    /*!
     * \variable ViaKeyboardInfo::product
     * The product string.
     */
    QString product;
    /*!
     * \variable ViaKeyboardInfo::vid
     * The USB vendor ID.
     */
    quint16 vid = 0;
    /*!
     * \variable ViaKeyboardInfo::pid
     * The USB product ID.
     */
    quint16 pid = 0;
};

/*!
 * \class ViaDiscovery
 * \inmodule SonicVia
 * \inheaderfile via/viadiscovery.h
 *
 * \brief Finds VIA keyboards.
 */
class SONICVIA_EXPORT ViaDiscovery
{
public:
    /*!
     * Lists all VIA keyboards (usage page 0xff60). Pure enumeration: does NOT probe-open.
     * Empty when built as a stub or when no VIA device is present. Access is checked by the
     * caller via ViaDevice::open().
     */
    static QVector<ViaKeyboardInfo> discoverViaKeyboards();
};
