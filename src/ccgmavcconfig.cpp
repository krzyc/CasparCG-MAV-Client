/*
* Copyright (c) 2015 Abbit Krzysztof Pyrkosz
*
* This file is part of CasparCG MAV Client.
*
* CasparCG MAV Client is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* CasparCG MAV Client is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CasparCG MAV Client. If not, see
* <http://www.gnu.org/licenses/>.
*
* Author: Krzysztof Pyrkosz <pyrkosz@o2.pl>
*/

#include "ccgmavcconfig.h"

CCGMAVCConfig::CCGMAVCConfig(QString id)
{
    root = document.createElement("CCGMAVCConfig");
    root.setAttribute("version", id);
    document.appendChild(root);
}

void CCGMAVCConfig::addParameter(QString id, QString value)
{
    QDomElement comp = document.createElement(id);
    QDomText text = document.createCDATASection(value);
    comp.appendChild(text);
    root.appendChild(comp);
}

QString CCGMAVCConfig::getParameter(QString id, QString defValue)
{
    QDomNodeList elements = document.elementsByTagName(id);
    if (elements.count() == 1)
        return elements.at(0).toElement().text();
    return defValue;
}

QString CCGMAVCConfig::toString()
{
    return document.toString();
}

int CCGMAVCConfig::readFromFile(QString fileName)
{
    QFile file;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    file.setFileName(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QString error;
        int line, column;
        if(document.setContent(stream.readAll(), &error, &line, &column)){
          qDebug() << document.toString(4);
        } else {
          qDebug() << "Error:" << error << "in line " << line << "column" << column;
        }
        file.close();
        return 1;
    }
    return 0;
}

int CCGMAVCConfig::writeToFile(QString fileName)
{
    QFile file;
    QTextStream stream(&file);
    stream.setGenerateByteOrderMark(true);
    stream.setCodec("UTF-8");
    file.setFileName(fileName);
    file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" << toString();
    file.close();
    return 1;
}
