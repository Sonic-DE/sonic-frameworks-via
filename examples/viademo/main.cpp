// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: SonicDE

#include <via/via.h>

#include <QColor>
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const auto keyboards = ViaDiscovery::discoverViaKeyboards();
    if (keyboards.isEmpty()) {
        qDebug() << "No VIA keyboards found.";
        return 0;
    }

    for (const auto &kb : keyboards) {
        qDebug() << kb.product << kb.path << QStringLiteral("0x%1:0x%2").arg(kb.vid, 4, 16, QLatin1Char('0')).arg(kb.pid, 4, 16, QLatin1Char('0'));
        ViaDevice device(kb.path);
        if (!device.open()) {
            qDebug() << "Failed to open:" << device.lastError();
            continue;
        }
        const quint16 protoVer = ViaProtocol::queryProtocolVersion(device);
        if (protoVer == 0) {
            qDebug() << "Could not read protocol version.";
            continue;
        }
        ViaProtocol protocol(device, protoVer);
        qDebug() << "Protocol version:" << protoVer;
        if (protocol.applyColor(QColor(0xFF, 0x00, 0x00))) {
            qDebug() << "Applied red.";
        } else {
            qDebug() << "applyColor failed.";
        }
    }
    return 0;
}
