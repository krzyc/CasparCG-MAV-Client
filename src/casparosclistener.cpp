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

#include "casparosclistener.h"

void CasparOscListener::ProcessMessage( const osc::ReceivedMessage& m,
        const IpEndpointName& remoteEndpoint )
{
    (void)remoteEndpoint; // suppress unused parameter warning

    try {
        QStringList address = QString(m.AddressPattern()+1).split("/");
        if (address[2] != "mixer") // exclude mixer messages
        {
            QStringList list;
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            for (int i = 0; i < m.ArgumentCount(); i++)
            {
                QString types = m.TypeTags();
                if (types[i] == 'f')
                    list.append(QString::number((arg++)->AsFloat()));
                else if (types[i] == 'h')
                    list.append(QString::number((arg++)->AsInt64()));
                else if (types[i] == 's')
                    list.append((arg++)->AsString());
                else
                    arg++;
            }
            emit messageAvailable(address, list);
        }
    } catch( osc::Exception& e ){
        qDebug() << "error while parsing message: " << m.AddressPattern() << ": " << e.what();
    }
}
