// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include "viacommand.h"
#include "sonicvia_debug.h"

ViaCommand::ViaCommand(ViaCommandId commandId, const QByteArray &payload)
    : m_commandId(commandId)
    , m_payload(payload)
{
    // The data region of a VIA report is 31 bytes (33 - report id - command id).
    if (m_payload.size() > kViaReportSize - 2) {
        qCWarning(SONICVIA) << "Payload exceeds" << (kViaReportSize - 2) << "bytes; truncating.";
        m_payload = m_payload.left(kViaReportSize - 2);
    }
}

ViaCommand::~ViaCommand() = default;

QByteArray ViaCommand::toReport() const
{
    QByteArray report;
    report.reserve(kViaReportSize);
    report.append(static_cast<char>(0x00)); // HID report id
    report.append(static_cast<char>(m_commandId)); // command id
    report.append(m_payload); // payload
    report.resize(kViaReportSize); // zero-pad to 33 bytes
    return report;
}

ViaCommand ViaCommand::getProtocolVersion()
{
    return ViaCommand(ViaCommandId::GetProtocolVersion);
}

ViaCommand ViaCommand::customSetValue(LightingChannel channel, RgbValueId valueId, const QByteArray &value)
{
    QByteArray payload;
    payload.append(static_cast<char>(channel));
    payload.append(static_cast<char>(valueId));
    payload.append(value);
    return ViaCommand(ViaCommandId::CustomSetValue, payload);
}

ViaCommand ViaCommand::customGetValue(LightingChannel channel, RgbValueId valueId)
{
    QByteArray payload;
    payload.append(static_cast<char>(channel));
    payload.append(static_cast<char>(valueId));
    return ViaCommand(ViaCommandId::CustomGetValue, payload);
}

ViaCommand ViaCommand::customSave()
{
    return ViaCommand(ViaCommandId::CustomSave);
}
