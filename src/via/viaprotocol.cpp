// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include "viaprotocol.h"
#include "sonicvia_debug.h"

ViaProtocol::ViaProtocol(ViaDevice &device, quint16 protocolVersion)
    : m_device(device)
    , m_protocolVersion(protocolVersion)
{
}

ViaProtocol::~ViaProtocol() = default;

quint16 ViaProtocol::queryProtocolVersion(ViaDevice &device)
{
    const ViaCommand cmd(ViaCommandId::GetProtocolVersion);
    const QByteArray response = device.sendReport(cmd.toReport());
    if (response.size() < 3) {
        return 0;
    }
    const auto first = static_cast<unsigned char>(response[0]);
    if (first != static_cast<unsigned char>(ViaCommandId::GetProtocolVersion)) {
        return 0;
    }
    // Big-endian u16 at response[1],[2] (verified: utils::shift_to_16_bit(buffer[1], buffer[2])).
    return (static_cast<quint16>(static_cast<unsigned char>(response[1])) << 8) | static_cast<quint16>(static_cast<unsigned char>(response[2]));
}

quint8 ViaProtocol::valueIdByte(RgbValueId valueId) const
{
    const quint8 v3 = static_cast<quint8>(valueId); // enum values are already 1..4 (V3 ids)
    // Legacy ids 0x80..0x83 == 0x7F + v3_id (verified from qmk-via-api ViaLightingValue).
    return (m_protocolVersion >= kProtocolV3) ? v3 : static_cast<quint8>(0x7F + v3);
}

bool ViaProtocol::setLightingValue(LightingChannel channel, RgbValueId valueId, const QByteArray &value)
{
    // Legacy protocol exposes only RGB Light (no matrix). Reject other channels without HID,
    // matching qmk-via-api's early Err(UnsupportedFeature) gate.
    if (m_protocolVersion < kProtocolV3 && channel != LightingChannel::QmkRgblight) {
        qCWarning(SONICVIA) << "Channel" << static_cast<int>(channel) << "not available on legacy protocol" << m_protocolVersion;
        return false;
    }
    // Payload: V3 = [channel, valueId(1-4), value]; legacy = [valueId(0x80-0x83), value].
    QByteArray payload;
    if (m_protocolVersion >= kProtocolV3) {
        payload.append(static_cast<char>(channel));
    }
    payload.append(static_cast<char>(valueIdByte(valueId)));
    payload.append(value);
    const ViaCommand cmd(ViaCommandId::CustomSetValue, payload);
    const QByteArray response = m_device.sendReport(cmd.toReport());
    if (response.isEmpty()) {
        qCWarning(SONICVIA) << "No response from device for setLightingValue";
        return false;
    }
    const auto first = static_cast<unsigned char>(response[0]);
    if (first == 0xFF) {
        qCWarning(SONICVIA) << "Unsupported lighting value on channel" << static_cast<int>(channel) << "value" << static_cast<int>(valueId);
        return false;
    }
    // The firmware echoes the command id on success.
    if (first != static_cast<unsigned char>(cmd.commandId())) {
        qCWarning(SONICVIA) << "Unexpected response id:" << static_cast<int>(first);
        return false;
    }
    return true;
}

bool ViaProtocol::getLightingValue(LightingChannel channel, RgbValueId valueId, QByteArray &outValue)
{
    if (m_protocolVersion < kProtocolV3 && channel != LightingChannel::QmkRgblight) {
        qCWarning(SONICVIA) << "Channel" << static_cast<int>(channel) << "not available on legacy protocol" << m_protocolVersion;
        return false;
    }
    // GET payload mirrors SET without the value bytes.
    QByteArray payload;
    if (m_protocolVersion >= kProtocolV3) {
        payload.append(static_cast<char>(channel));
    }
    payload.append(static_cast<char>(valueIdByte(valueId)));
    const ViaCommand cmd(ViaCommandId::CustomGetValue, payload);

    const QByteArray response = m_device.sendReport(cmd.toReport());
    if (response.isEmpty()) {
        qCWarning(SONICVIA) << "No response from device for getLightingValue";
        return false;
    }
    const auto first = static_cast<unsigned char>(response[0]);
    if (first == 0xFF) {
        qCWarning(SONICVIA) << "Unsupported lighting value on channel" << static_cast<int>(channel) << "value" << static_cast<int>(valueId);
        return false;
    }
    if (first != static_cast<unsigned char>(cmd.commandId())) {
        qCWarning(SONICVIA) << "Unexpected response id:" << static_cast<int>(first);
        return false;
    }
    // Value offset depends on protocol: V3 response = [cmdId, channel, valueId, value...] (index 3);
    // legacy response = [cmdId, valueId, value...] (index 2). Verified from qmk-via-api
    // (get_rgblight_color V3 uses val[3]/val[4], legacy uses val[2]/val[3]).
    const int valueOffset = (m_protocolVersion >= kProtocolV3) ? 3 : 2;
    if (response.size() < valueOffset + 1) {
        qCWarning(SONICVIA) << "Response too short:" << response.size();
        return false;
    }
    outValue = response.mid(valueOffset);
    return true;
}

bool ViaProtocol::setColor(LightingChannel channel, quint8 hue, quint8 saturation)
{
    QByteArray value;
    value.append(static_cast<char>(hue));
    value.append(static_cast<char>(saturation));
    return setLightingValue(channel, RgbValueId::Color, value);
}

bool ViaProtocol::setBrightness(LightingChannel channel, quint8 brightness)
{
    QByteArray value;
    value.append(static_cast<char>(brightness));
    return setLightingValue(channel, RgbValueId::Brightness, value);
}

bool ViaProtocol::getEffect(LightingChannel channel, quint8 &outEffect)
{
    QByteArray value;
    if (!getLightingValue(channel, RgbValueId::Effect, value) || value.isEmpty()) {
        return false;
    }
    outEffect = static_cast<unsigned char>(value[0]);
    return true;
}

bool ViaProtocol::saveSettings()
{
    const ViaCommand cmd = ViaCommand::customSave();
    const QByteArray response = m_device.sendReport(cmd.toReport());
    if (response.isEmpty()) {
        qCWarning(SONICVIA) << "No response from device for saveSettings";
        return false;
    }
    const auto first = static_cast<unsigned char>(response[0]);
    return first != 0xFF && first == static_cast<unsigned char>(cmd.commandId());
}

ViaHsv ViaProtocol::hsvFromColor(const QColor &color)
{
    if (!color.isValid()) {
        qCWarning(SONICVIA) << "Invalid color passed to hsvFromColor";
        return {};
    }
    // Qt 6 getHsvF takes float* and returns components in [0.0, 1.0]; hue is -1.0 for achromatic.
    float hueF = 0.0f, satF = 0.0f, valF = 0.0f;
    color.getHsvF(&hueF, &satF, &valF);

    ViaHsv out;
    // Matches the Rust helper: hue_deg/360*255 == hueF*255 (hueF == hue_deg/360).
    out.hue = (hueF < 0.0f) ? quint8(0) : static_cast<quint8>(hueF * 255.0f);
    out.saturation = static_cast<quint8>(satF * 255.0f);
    out.brightness = static_cast<quint8>(valF * 255.0f);
    return out;
}

bool ViaProtocol::applyColor(const QColor &color)
{
    const ViaHsv hsv = hsvFromColor(color);
    if (!color.isValid()) {
        return false;
    }

    // RGB Matrix first: supported when effect > 0 (matches qmk-via-api/get_rgb_matrix_effect).
    quint8 matrixEffect = 0;
    if (getEffect(LightingChannel::QmkRgbMatrix, matrixEffect) && matrixEffect > 0) {
        if (!setColor(LightingChannel::QmkRgbMatrix, hsv.hue, hsv.saturation)) {
            qCWarning(SONICVIA) << "Failed to set RGB Matrix color";
            return false;
        }
        if (!setBrightness(LightingChannel::QmkRgbMatrix, hsv.brightness)) {
            qCWarning(SONICVIA) << "Failed to set RGB Matrix brightness";
            return false;
        }
        return true;
    }

    // Fall back to RGB Light.
    quint8 rgblightEffect = 0;
    if (getEffect(LightingChannel::QmkRgblight, rgblightEffect) && rgblightEffect > 0) {
        if (!setColor(LightingChannel::QmkRgblight, hsv.hue, hsv.saturation)) {
            qCWarning(SONICVIA) << "Failed to set RGB Light color";
            return false;
        }
        if (!setBrightness(LightingChannel::QmkRgblight, hsv.brightness)) {
            qCWarning(SONICVIA) << "Failed to set RGB Light brightness";
            return false;
        }
        return true;
    }

    qCWarning(SONICVIA) << "No supported RGB lighting channel on this device";
    return false;
}

bool ViaProtocol::hasLightingSupport()
{
    // Matches Rust has_devices(): rgblight.is_ok() || matrix.is_ok(). "ok" means the channel
    // responded with a value (not 0xFF / not empty) — the effect value itself is irrelevant here,
    // which is why a device with lighting disabled (effect 0) still reports support.
    quint8 effect = 0;
    if (getEffect(LightingChannel::QmkRgbMatrix, effect)) {
        return true;
    }
    if (getEffect(LightingChannel::QmkRgblight, effect)) {
        return true;
    }
    return false;
}
