// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>

#include <QByteArray>
#include <QString>

// hidapi is linked PRIVATE: the HID handle is an opaque `void *`, so consumers don't need hidapi.h.
/*!
 * \class ViaDevice
 * \inmodule SonicVia
 * \inheaderfile via/viadevice.h
 *
 * \brief A thin, blocking wrapper over a single HID device path.
 *
 * sendReport() is virtual so that unit tests can inject a mock device.
 */
class SONICVIA_EXPORT ViaDevice
{
public:
    /*!
     * Creates a device for the HID path \a devicePath, as ViaDiscovery lists it.
     */
    explicit ViaDevice(QString devicePath);
    virtual ~ViaDevice();

    ViaDevice(const ViaDevice &) = delete;
    ViaDevice &operator=(const ViaDevice &) = delete;

    /*!
     * Opens the device; true when it is open. Always false when built without
     * VIA support (no hidapi).
     */
    bool open();
    /*!
     * Closes the device; always true.
     */
    bool close();

    /*!
     * Sends the 33-byte VIA \a report and reads the (<=32 byte) response.
     * Returns an empty QByteArray on any error or when built as a stub.
     */
    virtual QByteArray sendReport(const QByteArray &report);

    /*!
     * hidapi's description of the last error.
     */
    QString lastError() const;

    /*!
     * The device's HID path.
     */
    QString devicePath() const
    {
        return m_devicePath;
    }
    /*!
     * Whether open() succeeded and close() hasn't been called since.
     */
    bool isOpen() const
    {
        return m_open;
    }

private:
    QString m_devicePath;
    bool m_open = false;
    void *m_hidHandle = nullptr; // opaque; cast to hid_device* in the .cpp
};
