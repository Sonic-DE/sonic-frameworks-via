// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>
#include <via/viacommand.h>
#include <via/viadevice.h>

#include <QColor>
#include <cstdint>

/*!
 * \class ViaHsv
 * \inmodule SonicVia
 * \inheaderfile via/viaprotocol.h
 *
 * \brief Result of converting a QColor to VIA's single-byte HSV components.
 */
struct ViaHsv {
    /*!
     * \variable ViaHsv::hue
     * The hue, 0-255.
     */
    quint8 hue = 0;
    /*!
     * \variable ViaHsv::saturation
     * The saturation, 0-255.
     */
    quint8 saturation = 0;
    /*!
     * \variable ViaHsv::brightness
     * The brightness, 0-255.
     */
    quint8 brightness = 0;
};

/*!
 * \class ViaProtocol
 * \inmodule SonicVia
 * \inheaderfile via/viaprotocol.h
 *
 * \brief The VIA lighting commands for one device.
 */
class SONICVIA_EXPORT ViaProtocol
{
public:
    /*!
     * Talks to \a device. \a protocolVersion selects the command encoding (V3 >= kProtocolV3 vs legacy). Defaults to V3.
     * Use queryProtocolVersion(device) to obtain the device's actual version before constructing.
     */
    explicit ViaProtocol(ViaDevice &device, quint16 protocolVersion = kProtocolV3);
    ~ViaProtocol();

    /*!
     * Queries GetProtocolVersion on \a device and returns the big-endian u16 at response[1],[2]
     * (verified: utils::shift_to_16_bit(buffer[1], buffer[2])). Returns 0 on failure (the caller
     * should then skip the device). Independent of the instance protocol version.
     */
    static quint16 queryProtocolVersion(ViaDevice &device);

    /*!
     * The protocol version that selects the command encoding.
     */
    quint16 protocolVersion() const
    {
        return m_protocolVersion;
    }

    /*!
     * Sets \a valueId on \a channel. \a value is the raw value bytes (e.g. [hue, sat] for
     * Color, [brightness] for Brightness). Encoding is protocol-version-aware: V3 uses
     * [channel, valueId(1-4), value]; legacy (< 11) uses rgblight-only [valueId(0x80-0x83), value]
     * and rejects non-rgblight channels without HID. On success the device echoed the command; on
     * unsupported (0xFF) or mismatch this returns false.
     */
    bool setLightingValue(LightingChannel channel, RgbValueId valueId, const QByteArray &value);
    /*!
     * Reads \a valueId on \a channel into \a outValue, encoded as for
     * setLightingValue(); false when the device doesn't support it or the
     * response doesn't match.
     */
    bool getLightingValue(LightingChannel channel, RgbValueId valueId, QByteArray &outValue);

    /*!
     * Sets the color on \a channel to \a hue and \a saturation.
     */
    bool setColor(LightingChannel channel, quint8 hue, quint8 saturation);
    /*!
     * Sets the brightness on \a channel to \a brightness.
     */
    bool setBrightness(LightingChannel channel, quint8 brightness);
    /*!
     * Reads the lighting effect on \a channel into \a outEffect.
     */
    bool getEffect(LightingChannel channel, quint8 &outEffect);

    /*!
     * Saves the lighting settings on the device.
     */
    bool saveSettings();

    /*!
     * Converts \a color to the VIA single-byte HSV.
     * Returns a zeroed ViaHsv for an invalid color.
     */
    static ViaHsv hsvFromColor(const QColor &color);

    /*!
     * Sets the device's lighting to \a color: try RGB Matrix (effect > 0), fall back to RGB
     * Light, otherwise return false. Returns false in stub mode.
     */
    bool applyColor(const QColor &color);

    /*!
     * Whether this device has lighting: true when the RGB Matrix channel responds (device
     * returns a value, not 0xFF) OR the RGB Light channel responds, REGARDLESS of the effect
     * value. This is intentionally more lenient than applyColor(), which requires effect > 0.
     * Returns false in stub mode.
     */
    bool hasLightingSupport();

private:
    ViaDevice &m_device;
    quint16 m_protocolVersion = kProtocolV3;
    // Maps a RgbValueId to its on-wire byte for the active protocol: V3 = 1..4, legacy = 0x80..0x83
    // (verified: legacy_byte == 0x7F + v3_byte).
    quint8 valueIdByte(RgbValueId valueId) const;
};
