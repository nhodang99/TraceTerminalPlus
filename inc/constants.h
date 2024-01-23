#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QStringList>
#include <QHostAddress>

// Common constants
namespace Config
{
const QString CONFIG_DIR            = QStringLiteral("./TraceTerminalPlus.ini");
const QString INTERFACE             = QStringLiteral("Server/interface");
const QString PORT                  = QStringLiteral("Server/port");
const QString REMOTE_ADDRESS        = QStringLiteral("Server/remoteAddress");
const QString TRACEVIEW_AUTOSCROLL  = QStringLiteral("Traceview/autoscroll");
const QString MAINWINDOW_GEOMETRY   = QStringLiteral("Mainwindow/geometry");
const QString SEARCH_CASESENSITIVE  = QStringLiteral("Search/caseSensitive");
const QString SEARCH_LOOPSEARCH     = QStringLiteral("Search/loopSearch");
const QString CUSTOMS               = QStringLiteral("Custom/customs");
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
static const QString REMOTE_INTERFACE = QStringLiteral("Remote");
static const QString SERIAL_INTERFACE = QStringLiteral("Serial");
}

#endif // CONSTANTS_H
