// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include "viadevice.h"
#include "sonicvia_debug.h"
#include "viacommand.h" // kViaReportSize

#ifdef HAVE_VIA_SUPPORT
#include <hidapi/hidapi.h>
#endif

ViaDevice::ViaDevice(QString devicePath)
    : m_devicePath(std::move(devicePath))
{
}

ViaDevice::~ViaDevice()
{
    close();
}

bool ViaDevice::open()
{
#ifdef HAVE_VIA_SUPPORT
    if (m_open) {
        return true;
    }
    hid_device *handle = hid_open_path(m_devicePath.toUtf8().constData());
    if (!handle) {
        qCWarning(SONICVIA) << "Failed to open HID device:" << m_devicePath;
        return false;
    }
    hid_set_nonblocking(handle, 0); // blocking mode
    m_hidHandle = handle;
    m_open = true;
    qCDebug(SONICVIA) << "Opened HID device:" << m_devicePath;
    return true;
#else
    qCDebug(SONICVIA) << "Stub: VIA support not available";
    return false;
#endif
}

bool ViaDevice::close()
{
#ifdef HAVE_VIA_SUPPORT
    if (m_open) {
        hid_close(static_cast<hid_device *>(m_hidHandle));
        m_hidHandle = nullptr;
        m_open = false;
        qCDebug(SONICVIA) << "Closed HID device:" << m_devicePath;
    }
#endif
    return true;
}

QByteArray ViaDevice::sendReport(const QByteArray &report)
{
#ifdef HAVE_VIA_SUPPORT
    if (!m_open || !m_hidHandle) {
        qCWarning(SONICVIA) << "Device not open:" << m_devicePath;
        return {};
    }
    if (report.size() != kViaReportSize) {
        qCWarning(SONICVIA) << "Invalid report size:" << report.size() << "(expected" << kViaReportSize << ")";
        return {};
    }

    auto *handle = static_cast<hid_device *>(m_hidHandle);
    const int written = hid_write(handle, reinterpret_cast<const unsigned char *>(report.constData()), static_cast<size_t>(report.size()));
    if (written < 0) {
        qCWarning(SONICVIA) << "hid_write failed:" << lastError();
        return {};
    }

    // VIA responses are RAW_EPSIZE (32) bytes, without the leading report id.
    unsigned char buffer[32] = {0};
    const int read = hid_read_timeout(handle, buffer, sizeof(buffer), 1000);
    if (read < 0) {
        qCWarning(SONICVIA) << "hid_read_timeout failed:" << lastError();
        return {};
    }
    if (read == 0) {
        qCWarning(SONICVIA) << "HID read timeout on" << m_devicePath;
        return {};
    }
    return QByteArray(reinterpret_cast<const char *>(buffer), read);
#else
    Q_UNUSED(report)
    qCDebug(SONICVIA) << "Stub: VIA support not available";
    return {};
#endif
}

QString ViaDevice::lastError() const
{
#ifdef HAVE_VIA_SUPPORT
    if (m_hidHandle) {
        if (const wchar_t *err = hid_error(static_cast<hid_device *>(m_hidHandle))) {
            return QString::fromWCharArray(err);
        }
    }
    return {};
#else
    return QStringLiteral("VIA support not available");
#endif
}
