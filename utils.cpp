#include "utils.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QDebug>

namespace
{
    const QString warningTag    = QStringLiteral("<font color=Orange>");
    const QString errorTag      = QStringLiteral("<font color=Red>");
    const QString panicTag      = QStringLiteral("<font color=Purple>");
    const QString printTag      = QStringLiteral("<font color=#757575>"); // Sonic silver
    const QString traceTag      = QStringLiteral("<font color=Black>");
    const QString endTag        = QStringLiteral("</font>");
}

void processLine(QString& line)
{
    QString tag;
    if( line.contains(" WARNG - ") )
    {
        tag = warningTag;
    }
    else if( line.contains(" ERROR - ") )
    {
        tag = errorTag;
    }
    else if( line.contains(" PANIC - ") )
    {
        tag = panicTag;
    }
    else if( line.contains(" PRINT - ") )
    {
        tag = printTag;
    }
    else
    {
        tag = traceTag;
    }
    line = tag + line + endTag;
}
