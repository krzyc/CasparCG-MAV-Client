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

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
