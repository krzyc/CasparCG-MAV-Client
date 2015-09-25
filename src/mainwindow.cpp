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
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    recordRealFps = 0;
    timerCounter = 0;
    lastOsc = QDateTime::currentDateTime().addSecs(-1);
    playPlaying = false;

    ui->setupUi(this);
    setFixedSize(geometry().width(), geometry().height());

    QSettings settings("Abbit", "CasparCGMAVClient");
    settings.beginGroup("Configuration");
    ui->chkAutoConnect->setChecked(settings.value("auto_connect", true).toBool());
    ui->edtHost->setText(settings.value("host", "127.0.0.1:5250").toString());
    settings.endGroup();

    connect(&timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
    timer.start(100);

    connect(&tcp, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onTcpStateChanged(QAbstractSocket::SocketState)));

    connect(&listener, SIGNAL(messageAvailable(QStringList,QStringList)),
            this, SLOT(processOsc(QStringList,QStringList)));

    udp.bind(QHostAddress::AnyIPv4, 6250);
    connect(&udp, SIGNAL(readyRead()),
            this, SLOT(readyRead()));

    if (ui->chkAutoConnect->isChecked())
        on_btnConnect_clicked();
}

MainWindow::~MainWindow()
{
    on_btnDisconnect_clicked();

    QSettings settings("Abbit", "CasparCGMAVClient");
    settings.beginGroup("Configuration");
    settings.setValue("auto_connect", ui->chkAutoConnect->isChecked());
    settings.setValue("host", ui->edtHost->text());
    settings.endGroup();

    delete ui;
}

void MainWindow::log(QString message)
{
    ui->edtLog->moveCursor(QTextCursor::End);
    ui->edtLog->textCursor().insertText(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss") + ": " + message + "\n");
    ui->edtLog->moveCursor(QTextCursor::End);
    statusBar()->showMessage(message);
}

void MainWindow::on_btnConnect_clicked()
{
    QStringList host = ui->edtHost->text().split(':');
    tcp.connectToHost(host.first(), host.last().toInt());
}

void MainWindow::on_btnDisconnect_clicked()
{
    tcp.abort();
    tcp.disconnectFromHost();
}

void MainWindow::onTcpStateChanged(QAbstractSocket::SocketState socketState)
{
    QStringList states;
    states << "unconnected" << "host lookup" << "connecting" << "connected" << "bound" << "closing" << "listening";
    log("Connection status: " + states[socketState]);
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        ui->btnConnect->setEnabled(true);
        ui->btnDisconnect->setEnabled(false);
    }
    else
    {
        ui->btnConnect->setEnabled(false);
        ui->btnDisconnect->setEnabled(true);
    }
}

void MainWindow::readyRead()
{
    QByteArray buffer;
    buffer.resize(udp.pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    udp.readDatagram(buffer.data(), buffer.size(),
                             &sender, &senderPort);

    listener.ProcessPacket(buffer.data(), buffer.size(), IpEndpointName(sender.toIPv4Address(), senderPort));
}

void MainWindow::processOsc(QStringList address, QStringList values)
{
    QString adr = address.join("/");
    if (adr == "channel/2/output/file/frame")
        ui->lcdFrame->display(values[0].toInt());
    if (adr == "channel/2/output/file/fps")
    {
        if (recordFps != values[0].toInt())
        {
            recordFps = values[0].toInt();
            ui->lblRecordFps->setText(QString::number(recordRealFps) + "fps/" + QString::number(recordFps) + "fps");
        }
    }
    else if (adr == "channel/" + ui->edtChannel->text() + "/stage/layer/" + ui->edtLayer->text() + "/file/path")
        playPath = values[0];
    else if (adr == "channel/" + ui->edtChannel->text() + "/stage/layer/" + ui->edtLayer->text() + "/file/speed")
        playSpeed = values[0].toFloat();
    else if (adr == "channel/" + ui->edtChannel->text() + "/stage/layer/" + ui->edtLayer->text() + "/file/frame")
    {
        playCurFrame = values[0].toInt();
        playLastFrame = values[1].toInt();
        lastOsc = QDateTime::currentDateTime();
    }
    else if (adr == "channel/" + ui->edtChannel->text() + "/stage/layer/" + ui->edtLayer->text() + "/file/vframe")
    {
        playCurVFrame = values[0].toInt();
        playLastVFrame = values[1].toInt();
    }
    else if (adr == "channel/" + ui->edtChannel->text() + "/stage/layer/" + ui->edtLayer->text() + "/file/fps")
        playFps = values[0].toInt();
}

void MainWindow::timerEvent()
{
    if ((timerCounter % 10) == 0 && timerCounter > 0)
    {
        if (recPrevFrame > 0)
        {
            recordRealFps = (ui->lcdFrame->intValue()-recPrevFrame);
            ui->lblRecordFps->setText(QString::number(recordRealFps) + "fps/" + QString::number(recordFps) + "fps");
        }
        recPrevFrame = ui->lcdFrame->intValue();
    }
    if (lastOsc.msecsTo(QDateTime::currentDateTime()) > 200)
    {
        playPlaying = false;
        ui->proPlayback->setValue(0);
        ui->proPlayback->setRange(0, 1);
        ui->lblReplayStatus->setText("Replay: stopped");
    }
    else
    {
        playPlaying = true;
        if (playLastVFrame > 0)
        {
            ui->proPlayback->setValue(playCurVFrame);
            ui->proPlayback->setRange(0, playLastVFrame);
        }
        else
        {
            ui->proPlayback->setValue(0);
            ui->proPlayback->setRange(0, 0);
        }
        ui->lblReplayStatus->setText("Replay: frame=" + QString::number(playCurFrame) +
                "/" + QString::number(playLastFrame) + "; " +
                "vframe=" + QString::number(playCurVFrame) +
                "/" + QString::number(playLastVFrame) + "; " +
                "fps=" + QString::number(playFps) + "; " +
                "speed=" + QString::number(playSpeed) + "; " +
                "file:" + playPath);
    }
    timerCounter++;
}

void MainWindow::on_btnBegin_clicked()
{
    if (ui->lcdFrame->intValue() > 0)
    {
        int replaysCurRow = ui->tblReplays->rowCount();
        ui->tblReplays->insertRow(replaysCurRow);
        int frame = ui->lcdFrame->intValue();
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(frame));
        ui->tblReplays->setItem(replaysCurRow, 1, item);
        item = new QTableWidgetItem(QString::number(frame + 1));
        ui->tblReplays->setItem(replaysCurRow, 2, item);
        item = new QTableWidgetItem("1");
        ui->tblReplays->setItem(replaysCurRow, 3, item);
        ui->tblReplays->selectRow(replaysCurRow);
    }
}

void MainWindow::on_btnUpdateBegin_clicked()
{
    if (ui->lcdFrame->intValue() > 0)
    {
        QItemSelectionModel *select = ui->tblReplays->selectionModel();
        if (select->hasSelection())
        {
            int frame = ui->lcdFrame->intValue();
            QTableWidgetItem *item = new QTableWidgetItem(QString::number(frame));
            ui->tblReplays->setItem(select->selectedRows()[0].row(), 1, item);
            if (ui->tblReplays->item(select->selectedRows()[0].row(), 2)->text().toInt() <= frame)
            {
                item = new QTableWidgetItem(QString::number(frame + 1));
                ui->tblReplays->setItem(select->selectedRows()[0].row(), 2, item);
                item = new QTableWidgetItem("1");
                ui->tblReplays->setItem(select->selectedRows()[0].row(), 3, item);

            }
            on_tblReplays_itemSelectionChanged();
        }
    }
}

void MainWindow::on_btnEnd_clicked()
{
    if (ui->lcdFrame->intValue() > 0)
    {
        QItemSelectionModel *select = ui->tblReplays->selectionModel();
        if (select->hasSelection() && ui->tblReplays->item(select->selectedRows()[0].row(), 1))
        {
            QTableWidgetItem *item = new QTableWidgetItem(QString::number((ui->lcdFrame->intValue())));
            ui->tblReplays->setItem(select->selectedRows()[0].row(), 2, item);
            item = new QTableWidgetItem(QString::number(
                    ui->tblReplays->item(select->selectedRows()[0].row(), 2)->text().toInt() -
                    ui->tblReplays->item(select->selectedRows()[0].row(), 1)->text().toInt()));
            ui->tblReplays->setItem(select->selectedRows()[0].row(), 3, item);
            on_tblReplays_itemSelectionChanged();
        }
    }
}

void MainWindow::on_btnDelete_clicked()
{
    QItemSelectionModel *select = ui->tblReplays->selectionModel();
    if (select->hasSelection())
    {
        ui->tblReplays->removeRow(select->selectedRows()[0].row());
    }
}

void MainWindow::on_btnPlayAdd_clicked()
{
    int replaysCurRow = ui->tblReplays->rowCount();
    ui->tblReplays->insertRow(replaysCurRow);
    QTableWidgetItem *item = new QTableWidgetItem(ui->spnPlayStart->text());
    ui->tblReplays->setItem(replaysCurRow, 1, item);
    item = new QTableWidgetItem(ui->spnPlayEnd->text());
    ui->tblReplays->setItem(replaysCurRow, 2, item);
    item = new QTableWidgetItem(QString::number(ui->spnPlayEnd->value() - ui->spnPlayStart->value()));
    ui->tblReplays->setItem(replaysCurRow, 3, item);
    ui->tblReplays->selectRow(replaysCurRow);
}

void MainWindow::on_btnPlayUpdate_clicked()
{
    QItemSelectionModel *select = ui->tblReplays->selectionModel();
    if (select->hasSelection())
    {
        QTableWidgetItem *item = new QTableWidgetItem(ui->spnPlayStart->text());
        ui->tblReplays->setItem(select->selectedRows()[0].row(), 1, item);
        item = new QTableWidgetItem(ui->spnPlayEnd->text());
        ui->tblReplays->setItem(select->selectedRows()[0].row(), 2, item);
        item = new QTableWidgetItem(QString::number(ui->spnPlayEnd->value() - ui->spnPlayStart->value()));
        ui->tblReplays->setItem(select->selectedRows()[0].row(), 3, item);
    }
}

void MainWindow::on_tblReplays_itemSelectionChanged()
{
    QItemSelectionModel *select = ui->tblReplays->selectionModel();
    if (!select->hasSelection())
    {
        ui->btnPlayUpdate->setEnabled(false);
        ui->btnDelete->setEnabled(false);
    }
    else
    {
        ui->btnPlayUpdate->setEnabled(true);
        ui->btnDelete->setEnabled(true);
        int row = select->selectedRows()[0].row();
        int replayStart = ui->tblReplays->item(row, 1)->text().toInt();
        int replayEnd = ui->tblReplays->item(row, 2) ? ui->tblReplays->item(row, 2)->text().toInt() : replayStart + 1;
        ui->spnPlayStart->setValue(replayStart);
        ui->spnPlayEnd->setValue(replayEnd);
        playUpdate();
    }
}

void MainWindow::setSpeed(float speed)
{
    QString command;
    if (playPlaying)
    {
        command = "CALL " + ui->edtChannel->text() + "-" + ui->edtLayer->text() +
                " SPEED " + QString::number(speed) +
                "\r\n";
        tcp.write(command.toUtf8());
        if (ui->chkUpdateStart->isChecked())
        {
           command = "CALL " + ui->edtChannel->text() + "-" + ui->edtLayer->text() +
                   " SEEK " + QString::number(ui->spnPlayStart->value()) +
                   "\r\n";
           tcp.write(command.toUtf8());
           command = "CALL " + ui->edtChannel->text() + "-" + ui->edtLayer->text() +
                   " LENGTH " + (ui->chkStopAtEnd->isChecked()
                           ? QString::number(ui->spnPlayEnd->value() - ui->spnPlayStart->value())
                           : "0") +
                   "\r\n";
           tcp.write(command.toUtf8());
        }
    }
    else
    {
        command = "PLAY " + ui->edtChannel->text() + "-" + ui->edtLayer->text() + " " +
                ui->edtFile->text() +
                " SPEED " + QString::number(speed) +
                " SEEK " + ui->spnPlayStart->text() +
                (ui->chkStopAtEnd->isChecked()
                        ? " LENGTH " + QString::number(ui->spnPlayEnd->value() - ui->spnPlayStart->value())
                        : "") +
                "\r\n";
        tcp.write(command.toUtf8());
    }

}

void MainWindow::playUpdate()
{
    ui->lblPlayFrames->setText("Frames: " + QString::number(ui->spnPlayEnd->value() - ui->spnPlayStart->value()));
    ui->lblPlayLength->setText("Length: " + QString::number((int)((ui->spnPlayEnd->value() - ui->spnPlayStart->value())/recordFps))+"s");
}

void MainWindow::on_btnSpeedN05_clicked()
{
    setSpeed(-0.5);
}

void MainWindow::on_btnSpeed1_clicked()
{
    setSpeed(1);
}

void MainWindow::on_btnSpeed07_clicked()
{
    setSpeed(0.7);
}

void MainWindow::on_btnSpeed05_clicked()
{
    setSpeed(0.5);
}

void MainWindow::on_btnSpeed03_clicked()
{
    setSpeed(0.3);
}

void MainWindow::on_btnSpeed0_clicked()
{
    setSpeed(0);
}

void MainWindow::on_btnStop_clicked()
{
    QString command;
    command = "STOP " + ui->edtChannel->text() + "-" + ui->edtLayer->text() + "\r\n";
    tcp.write(command.toUtf8());
}

void MainWindow::on_spnPlayStart_valueChanged(int arg1)
{
    playUpdate();
}

void MainWindow::on_spnPlayEnd_valueChanged(int arg1)
{
    playUpdate();
}

void MainWindow::on_btnPlay2_clicked()
{
    QString command;
    command = "PLAY " + ui->edtRecordChannel->text() + " " + ui->edtDevice->text() + "\r\n";
    tcp.write(command.toUtf8());
}

void MainWindow::on_btnStop2_clicked()
{
    QString command;
    command = "STOP " + ui->edtRecordChannel->text() + "\r\n";
    tcp.write(command.toUtf8());
}

void MainWindow::on_btnAdd2_clicked()
{
    QString command;
    command = "ADD " + ui->edtRecordChannel->text() + " REPLAY " + ui->edtFile->text() + "\r\n";
    tcp.write(command.toUtf8());
}

void MainWindow::on_btnRemove2_clicked()
{
    QString command;
    command = "REMOVE " + ui->edtRecordChannel->text() + " REPLAY\r\n";
    tcp.write(command.toUtf8());
}

void MainWindow::on_btnDiag_clicked()
{
    QString command;
    command = "DIAG\r\n";
    tcp.write(command.toUtf8());
}

void MainWindow::on_btnLoad_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load configuration"), QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory), tr("CCGMAVC file (*.ccgmavc)"));
    CCGMAVCConfig config("1");
    if (config.readFromFile(fileName) > 0)
    {
        configFileName = fileName;
        ui->btnSave->setEnabled(true);

        config.addParameter("ReplayFile", ui->edtFile->text());
        config.addParameter("PlayChannel", ui->edtChannel->text());
        config.addParameter("PlayLayer", ui->edtLayer->text());
        config.addParameter("ReplayDevice", ui->edtDevice->text());
        config.addParameter("RecordChannel", ui->edtRecordChannel->text());
        ui->edtFile->setText(config.getParameter("ReplayFile", "replay1"));
        ui->edtChannel->setText(config.getParameter("PlayChannel", "1"));
        ui->edtLayer->setText(config.getParameter("PlayLayer", "25"));
        ui->edtDevice->setText(config.getParameter("ReplayDevice", "DECKLINK 1"));
        ui->edtRecordChannel->setText(config.getParameter("RecordChannel", "2"));

        QStringList reps = config.getParameter("Replays").split("\n");
        ui->tblReplays->clearContents();
        ui->tblReplays->setRowCount(0);
        foreach (QString rep, reps)
        {
            if (rep.length() > 0)
            {
                QStringList r = rep.split(";");
                if (r.length() == 4)
                {
                    int row = ui->tblReplays->rowCount();
                    ui->tblReplays->setRowCount(row+1);
                    ui->tblReplays->setItem(row, 0, new QTableWidgetItem(r[0]));
                    ui->tblReplays->setItem(row, 1, new QTableWidgetItem(r[1]));
                    ui->tblReplays->setItem(row, 2, new QTableWidgetItem(r[2]));
                    ui->tblReplays->setItem(row, 3, new QTableWidgetItem(r[3]));
                }
            }
        }
    }
}

void MainWindow::on_btnSave_clicked()
{
    saveConfig(configFileName);
}

void MainWindow::on_btnSaveAs_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save configuration"), QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory), tr("CCGMAVC file (*.ccgmavc)"));
    saveConfig(fileName);
}

void MainWindow::saveConfig(QString fileName)
{
    CCGMAVCConfig config("1");
    config.addParameter("ReplayFile", ui->edtFile->text());
    config.addParameter("PlayChannel", ui->edtChannel->text());
    config.addParameter("PlayLayer", ui->edtLayer->text());
    config.addParameter("ReplayDevice", ui->edtDevice->text());
    config.addParameter("RecordChannel", ui->edtRecordChannel->text());

    QStringList reps;
    for (int i = 0; i < ui->tblReplays->rowCount(); i++)
    {
        QString rep =
            (ui->tblReplays->item(i, 0) ? ui->tblReplays->item(i, 0)->text() : "") + ";" +
            (ui->tblReplays->item(i, 1) ? ui->tblReplays->item(i, 1)->text() : "") + ";" +
            (ui->tblReplays->item(i, 2) ? ui->tblReplays->item(i, 2)->text() : "") + ";" +
            (ui->tblReplays->item(i, 3) ? ui->tblReplays->item(i, 3)->text() : "");
        reps.append(rep);
    }
    config.addParameter("Replays", reps.join("\n"));

    if (config.writeToFile(fileName) > 0)
    {
        configFileName = fileName;
        ui->btnSave->setEnabled(true);
    }
    log("configSavedAs("+fileName+")");
}
