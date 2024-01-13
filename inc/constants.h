#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QStringList>
#include <QHostAddress>

// Common constants
namespace Config
{
const QString CONFIG_DIR            = QStringLiteral("./TraceTerminalPlus.ini");
const QString CONNECTED_HOST        = QStringLiteral("udpSocket/host");
const QString CONNECTED_PORT        = QStringLiteral("udpSocket/port");
const QString SPECIFIC_HOST         = QStringLiteral("udpSocket/specificHost");
const QString TRACEVIEW_AUTOSCROLL  = QStringLiteral("traceview/autoscroll");
const QString MAINWINDOW_GEOMETRY   = QStringLiteral("mainwindow/geometry");
const QString SEARCH_CASESENSITIVE  = QStringLiteral("search/caseSensitive");
const QString SEARCH_LOOPSEARCH     = QStringLiteral("search/loopSearch");
const QString CUSTOMS               = QStringLiteral("custom/customs");
}

// @TODO: Fix non-POD warning
namespace Highlight
{
const int CUSTOM_COLOR_NUMBER = 5;
static const QStringList customs = {"magenta", "blue", "deepskyblue", "peru", "teal"};
}

// @TODO: Fix non-POD warning
namespace SpecialInterface
{
static const QString ANY_INTERFACE = QHostAddress(QHostAddress::Any).toString();
static const QString ANY_IPV6_INTERFACE = QHostAddress(QHostAddress::AnyIPv6).toString();
static const QString LOCALHOST_INTERFACE = QHostAddress(QHostAddress::LocalHost).toString();
}

#endif // CONSTANTS_H
