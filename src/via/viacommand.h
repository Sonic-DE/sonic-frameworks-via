// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>

#include <QByteArray>
#include <cstdint>

// Byte values are authoritative (verified from qmk-via-api 0.7.0 ViaCommandId).
enum class ViaCommandId : uint8_t {
    GetProtocolVersion = 0x01,
    CustomSetValue = 0x07, // qmk-via-api: CustomMenuSetValue
    CustomGetValue = 0x08, // qmk-via-api: CustomMenuGetValue
    CustomSave = 0x09, // qmk-via-api: CustomMenuSave
};

// Verified from qmk-via-api ViaChannelId.
enum class LightingChannel : uint8_t {
    QmkRgblight = 0x02,
    QmkRgbMatrix = 0x03,
};

// VIA protocol versions (verified from qmk-via-api PROTOCOL_* constants). The lighting command
// ENCODING branches at PROTOCOL_V3 (11): devices reporting < 11 (e.g. the common protocol 9 /
// GAMMA) use the LEGACY single-byte value-id scheme (0x80-0x83, no channel byte, rgblight only);
// >= 11 use the V3 channel+value-id scheme (0x02/0x03 channel, value ids 1-4, RGB matrix enabled).
constexpr quint16 kProtocolAlpha = 7;
constexpr quint16 kProtocolBeta = 8;
constexpr quint16 kProtocolGamma = 9;
constexpr quint16 kProtocolV3 = 11;

// Verified from qmk-via-api ViaQmkRgbMatrixValue / ViaQmkRgblightValue.
enum class RgbValueId : uint8_t {
    Brightness = 0x01,
    Effect = 0x02,
    EffectSpeed = 0x03,
    Color = 0x04,
};

// Size of a VIA HID report (leading 0x00 report id + command id + 31 data bytes).
// Verified from qmk-via-api RAW_EPSIZE (32) + 1.
constexpr int kViaReportSize = 33;

class SONICVIA_EXPORT ViaCommand
{
public:
    explicit ViaCommand(ViaCommandId commandId, const QByteArray &payload = QByteArray());
    ~ViaCommand();

    // Returns the 33-byte HID report: [0x00, commandId, payload, zero-padded to 33].
    QByteArray toReport() const;

    ViaCommandId commandId() const
    {
        return m_commandId;
    }
    QByteArray payload() const
    {
        return m_payload;
    }

    static ViaCommand getProtocolVersion();
    // V3 (protocol >= 11) lighting value builders: payload = [channel, valueId, value...].
    // NOTE: these encode the V3 scheme only. ViaProtocol constructs the legacy (< 11) payload
    // ([valueId(0x80-0x83), value...]) directly; do not use these for legacy devices.
    static ViaCommand customSetValue(LightingChannel channel, RgbValueId valueId, const QByteArray &value);
    static ViaCommand customGetValue(LightingChannel channel, RgbValueId valueId);
    static ViaCommand customSave();

private:
    ViaCommandId m_commandId;
    QByteArray m_payload; // up to 31 bytes (truncated if larger)
};
