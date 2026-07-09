// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>

#include <QByteArray>
#include <QString>

// A thin, blocking wrapper over a single HID device path. sendReport() is virtual so that
// unit tests can inject a mock device.
//
// IMPORTANT: hidapi is linked PRIVATE and must NOT appear in this PUBLIC header. The HID
// handle is held as an opaque `void *` so that consumers (Phase 2 helper, unit tests) can
// include this header without needing hidapi.h on their include path.
class SONICVIA_EXPORT ViaDevice
{
public:
    explicit ViaDevice(QString devicePath);
    virtual ~ViaDevice();

    ViaDevice(const ViaDevice &) = delete;
    ViaDevice &operator=(const ViaDevice &) = delete;

    bool open();
    bool close();

    // Sends the 33-byte VIA report and reads the (<=32 byte) response.
    // Returns an empty QByteArray on any error or when built as a stub.
    virtual QByteArray sendReport(const QByteArray &report);

    QString lastError() const;

    QString devicePath() const
    {
        return m_devicePath;
    }
    bool isOpen() const
    {
        return m_open;
    }

private:
    QString m_devicePath;
    bool m_open = false;
    void *m_hidHandle = nullptr; // opaque; cast to hid_device* in the .cpp
};
