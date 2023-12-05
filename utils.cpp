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


void convert(QString& filePath)
{
    QFileInfo original(filePath);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot open trace file";
        return;
    }

    QFile htmlFile(original.absolutePath() + "/" + original.baseName() + ".html");
    if (!htmlFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Cannot create new html file";
        return;
    }

    QTextStream in(&file);
    QTextStream out(&htmlFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        processLine(line);
        out << line;
    }

    qDebug() << "reading file done";
    htmlFile.close();
    file.close();

    // Open saved directory - @todo: remove this
    QFileInfo newFileDir(htmlFile);
    QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(newFileDir.absoluteFilePath())});
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
