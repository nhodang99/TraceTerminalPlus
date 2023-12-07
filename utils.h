#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QTextStream>

// Common constants
namespace Config {
const QString CONFIG_DIR            = QStringLiteral("./TraceTerminalPlus.ini");
const QString INTERFACE             = QStringLiteral("udpSocket/interface");
const QString PORT                  = QStringLiteral("udpSocket/port");
const QString TRACEVIEW_AUTOSCROLL  = QStringLiteral("traceview/autoscroll");
const QString MAINWINDOW_FULLSCREEN = QStringLiteral("mainwindow/windowState");
const QString MAINWINDOW_GEOMETRY   = QStringLiteral("mainwindow/geometry");
}

// helper functions
void processLine(QString&);

#endif // UTILS_H
