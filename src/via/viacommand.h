// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#pragma once

#include <via/sonicvia_export.h>

#include <QByteArray>
#include <cstdint>

/*!
 * \enum ViaCommandId
 * \relates ViaCommand
 *
 * The VIA command IDs this library sends.
 *
 * \value GetProtocolVersion Asks for the device's VIA protocol version.
 * \value CustomSetValue Sets a lighting value.
 * \value CustomGetValue Reads a lighting value.
 * \value CustomSave Saves the lighting settings on the device.
 */
enum class ViaCommandId : uint8_t {
    GetProtocolVersion = 0x01,
    CustomSetValue = 0x07,
    CustomGetValue = 0x08,
    CustomSave = 0x09,
};

/*!
 * \enum LightingChannel
 * \relates ViaCommand
 *
 * The keyboard's lighting system.
 *
 * \value QmkRgblight QMK RGB Light.
 * \value QmkRgbMatrix QMK RGB Matrix.
 */
enum class LightingChannel : uint8_t {
    QmkRgblight = 0x02,
    QmkRgbMatrix = 0x03,
};

/*!
 * \variable kProtocolAlpha
 * \relates ViaCommand
 * VIA protocol version 7, "alpha".
 */
constexpr quint16 kProtocolAlpha = 7;
/*!
 * \variable kProtocolBeta
 * \relates ViaCommand
 * VIA protocol version 8, "beta".
 */
constexpr quint16 kProtocolBeta = 8;
/*!
 * \variable kProtocolGamma
 * \relates ViaCommand
 * VIA protocol version 9, "gamma", which many keyboards report.
 */
constexpr quint16 kProtocolGamma = 9;
/*!
 * \variable kProtocolV3
 * \relates ViaCommand
 *
 * VIA protocol version 11, where the lighting command encoding changes.
 * Devices reporting less use the legacy scheme: single-byte value IDs
 * 0x80-0x83, no channel byte, RGB Light only. From 11 on, commands carry the
 * channel and value IDs 1-4, and RGB Matrix is available.
 */
constexpr quint16 kProtocolV3 = 11;

/*!
 * \enum RgbValueId
 * \relates ViaCommand
 *
 * A lighting value.
 *
 * \value Brightness The brightness.
 * \value Effect The lighting effect.
 * \value EffectSpeed The effect's speed.
 * \value Color The color, as hue and saturation.
 */
enum class RgbValueId : uint8_t {
    Brightness = 0x01,
    Effect = 0x02,
    EffectSpeed = 0x03,
    Color = 0x04,
};

/*!
 * \variable kViaReportSize
 * \relates ViaCommand
 *
 * Size of a VIA HID report (leading 0x00 report id + command id + 31 data bytes).
 */
constexpr int kViaReportSize = 33;

/*!
 * \class ViaCommand
 * \inmodule SonicVia
 * \inheaderfile via/viacommand.h
 *
 * \brief One VIA command and its payload.
 */
class SONICVIA_EXPORT ViaCommand
{
public:
    /*!
     * Creates the command \a commandId with \a payload, truncated to 31 bytes.
     */
    explicit ViaCommand(ViaCommandId commandId, const QByteArray &payload = QByteArray());
    ~ViaCommand();

    /*!
     * Returns the 33-byte HID report: [0x00, commandId, payload, zero-padded to 33].
     */
    QByteArray toReport() const;

    /*!
     * The command's ID.
     */
    ViaCommandId commandId() const
    {
        return m_commandId;
    }
    /*!
     * The command's payload.
     */
    QByteArray payload() const
    {
        return m_payload;
    }

    /*!
     * The command that asks for the device's VIA protocol version.
     */
    static ViaCommand getProtocolVersion();
    /*!
     * Sets \a valueId on \a channel to the bytes \a value. Payload:
     * [channel, valueId, value...].
     *
     * \note This encodes the V3 scheme (protocol 11 and later) only. ViaProtocol
     * constructs the legacy payload ([valueId(0x80-0x83), value...]) directly;
     * don't use this for legacy devices.
     */
    static ViaCommand customSetValue(LightingChannel channel, RgbValueId valueId, const QByteArray &value);
    /*!
     * Reads \a valueId on \a channel. Payload: [channel, valueId].
     *
     * \note This encodes the V3 scheme (protocol 11 and later) only; see customSetValue().
     */
    static ViaCommand customGetValue(LightingChannel channel, RgbValueId valueId);
    /*!
     * Saves the lighting settings on the device.
     */
    static ViaCommand customSave();

private:
    ViaCommandId m_commandId;
    QByteArray m_payload; // up to 31 bytes (truncated if larger)
};
