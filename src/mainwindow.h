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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <QtXml>
#include "ccgmavcconfig.h"
#include "casparosclistener.h"

#include <osc/OscReceivedElements.h>
#include <ip/IpEndpointName.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void setReplayFrame();
    ~MainWindow();

private slots:
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();
    void on_btnBegin_clicked();
    void on_btnEnd_clicked();
    void on_tblReplays_itemSelectionChanged();
    void on_btnSpeedN05_clicked();
    void on_btnSpeed1_clicked();
    void on_btnSpeed07_clicked();
    void on_btnSpeed05_clicked();
    void on_btnSpeed03_clicked();
    void on_btnSpeed0_clicked();
    void on_btnStop_clicked();
    void on_spnPlayStart_valueChanged(int arg1);
    void on_spnPlayEnd_valueChanged(int arg1);
    void on_btnUpdateBegin_clicked();
    void on_btnDelete_clicked();
    void on_btnPlayAdd_clicked();
    void on_btnPlayUpdate_clicked();
    void on_btnPlay2_clicked();
    void on_btnStop2_clicked();
    void on_btnAdd2_clicked();
    void on_btnRemove2_clicked();
    void on_btnDiag_clicked();
    void on_btnLoad_clicked();
    void on_btnSave_clicked();
    void on_btnSaveAs_clicked();

    void on_btnSub50_clicked();

public slots:
    void onTcpStateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();
    void processOsc(QStringList address, QStringList values);
    void timerEvent();

private:
    void setSpeed(float speed);
    void playUpdate();
    void saveConfig(QString fileName);

    Ui::MainWindow *ui;
    QTcpSocket tcp;
    QUdpSocket udp;
    void log(QString message);
    CasparOscListener listener;

    QString configFileName;

    int recordFps, recordRealFps;

    bool playPlaying;
    int playCurFrame, playLastFrame, playCurVFrame, playLastVFrame, playFps;
    float playSpeed;
    QString playPath;
    QDateTime lastOsc;

    QTimer timer;
    int timerCounter, recPrevFrame;
};

#endif // MAINWINDOW_H
