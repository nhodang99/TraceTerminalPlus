#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QStringList>

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

namespace Highlight
{
const int CUSTOM_COLOR_NUMBER = 5;
// @TODO: Fix non-POD warning
static const QStringList customs = {"magenta", "blue", "deepskyblue", "peru", "teal"};
}

#endif // CONSTANTS_H
