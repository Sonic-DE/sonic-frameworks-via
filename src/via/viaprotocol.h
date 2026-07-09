// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>
#include <via/viacommand.h>
#include <via/viadevice.h>

#include <QColor>
#include <cstdint>

// Result of converting a QColor to the VIA single-byte HSV components used by the Rust helper.
struct ViaHsv {
    quint8 hue = 0;
    quint8 saturation = 0;
    quint8 brightness = 0;
};

class SONICVIA_EXPORT ViaProtocol
{
public:
    // `protocolVersion` selects the command encoding (V3 >= kProtocolV3 vs legacy). Defaults to V3.
    // Use queryProtocolVersion(device) to obtain the device's actual version before constructing.
    explicit ViaProtocol(ViaDevice &device, quint16 protocolVersion = kProtocolV3);
    ~ViaProtocol();

    // Queries GetProtocolVersion and returns the big-endian u16 at response[1],[2]
    // (verified: utils::shift_to_16_bit(buffer[1], buffer[2])). Returns 0 on failure (the caller
    // should then skip the device). Independant of the instance protocol version.
    static quint16 queryProtocolVersion(ViaDevice &device);

    quint16 protocolVersion() const
    {
        return m_protocolVersion;
    }

    // Generic lighting value accessors. `value` is the raw value bytes (e.g. [hue, sat] for
    // Color, [brightness] for Brightness). Encoding is protocol-version-aware: V3 uses
    // [channel, valueId(1-4), value]; legacy (< 11) uses rgblight-only [valueId(0x80-0x83), value]
    // and rejects non-rgblight channels without HID. On success the device echoed the command; on
    // unsupported (0xFF) or mismatch these return false.
    bool setLightingValue(LightingChannel channel, RgbValueId valueId, const QByteArray &value);
    bool getLightingValue(LightingChannel channel, RgbValueId valueId, QByteArray &outValue);

    // Convenience wrappers mirroring qmk-via-api.
    bool setColor(LightingChannel channel, quint8 hue, quint8 saturation);
    bool setBrightness(LightingChannel channel, quint8 brightness);
    bool getEffect(LightingChannel channel, quint8 &outEffect);

    bool saveSettings();

    // Converts a QColor to the VIA single-byte HSV (verified to match the Rust helper scaling).
    // Returns a zeroed ViaHsv for an invalid color.
    static ViaHsv hsvFromColor(const QColor &color);

    // Replicates the Rust helper apply_color() flow for this single device: try RGB Matrix
    // (effect > 0), fall back to RGB Light, otherwise return false. Returns false in stub mode.
    bool applyColor(const QColor &color);

    // Replicates the Rust helper has_devices() predicate for this single device: true when the
    // RGB Matrix channel responds (device returns a value, not 0xFF) OR the RGB Light channel
    // responds — REGARDLESS of the effect value. This is intentionally more lenient than
    // applyColor() (which requires effect > 0); the asymmetry is verified from qmk-via-api where
    // get_*_effect().is_ok() (used by has_devices) checks channel reachability while apply_color
    // checks effect > 0 (in use). Returns false in stub mode.
    bool hasLightingSupport();

private:
    ViaDevice &m_device;
    quint16 m_protocolVersion = kProtocolV3;
    // Maps a RgbValueId to its on-wire byte for the active protocol: V3 = 1..4, legacy = 0x80..0x83
    // (verified: legacy_byte == 0x7F + v3_byte).
    quint8 valueIdByte(RgbValueId valueId) const;
};
