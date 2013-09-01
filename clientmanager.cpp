/*
 *  QCMA: Cross-platform content manager assistant for the PS Vita
 *
 *  Copyright (C) 2013  Codestation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clientmanager.h"
#include "progressform.h"
#include "cmaclient.h"

ClientManager::ClientManager(QObject *parent) :
    QObject(parent)
{
}

void ClientManager::databaseUpdated(int count)
{
    progress.hide();
    if(count >= 0) {
        emit messageSent(tr("Added %1 items to the database").arg(count));
    } else {
        emit messageSent(tr("Database indexing aborted by user"));
    }
}

void ClientManager::showPinDialog(QString name, int pin)
{
    pinForm.setPin(name, pin);
    pinForm.startCountdown();
}

void ClientManager::start()
{
    // initializing database for the first use
    refreshDatabase();
    CmaEvent::db = &db;
    connect(&db, SIGNAL(fileAdded(QString)), &progress, SLOT(setFileName(QString)));
    connect(&db, SIGNAL(directoryAdded(QString)), &progress, SLOT(setDirectoryName(QString)));
    connect(&db, SIGNAL(updated(int)), this, SLOT(databaseUpdated(int)));
    connect(&progress, SIGNAL(canceled()), &db, SLOT(cancelOperation()), Qt::DirectConnection);

    thread_count = 2;
    qDebug("Starting cma threads");
    CmaClient *client;

    usb_thread = new QThread();
    client = new CmaClient();
    usb_thread->setObjectName("usb_thread");
    connect(usb_thread, SIGNAL(started()), client, SLOT(connectUsb()));
    connect(client, SIGNAL(finished()), usb_thread, SLOT(quit()), Qt::DirectConnection);
    connect(usb_thread, SIGNAL(finished()), usb_thread, SLOT(deleteLater()));
    connect(usb_thread, SIGNAL(finished()), this, SLOT(threadStopped()));
    connect(usb_thread, SIGNAL(finished()), client, SLOT(deleteLater()));

    connect(client, SIGNAL(deviceConnected(QString)), this, SIGNAL(deviceConnected(QString)));
    connect(client, SIGNAL(deviceDisconnected()), this, SIGNAL(deviceDisconnected()));
    connect(client, SIGNAL(refreshDatabase()), this, SLOT(refreshDatabase()));

    client->moveToThread(usb_thread);
    usb_thread->start();

    wireless_thread = new QThread();
    client = new CmaClient();
    wireless_thread->setObjectName("wireless_thread");
    connect(wireless_thread, SIGNAL(started()), client, SLOT(connectWireless()));
    connect(client, SIGNAL(receivedPin(QString,int)), this, SLOT(showPinDialog(QString,int)));
    connect(client, SIGNAL(finished()), wireless_thread, SLOT(quit()), Qt::DirectConnection);
    connect(wireless_thread, SIGNAL(finished()), wireless_thread, SLOT(deleteLater()));
    connect(wireless_thread, SIGNAL(finished()), this, SLOT(threadStopped()));
    connect(wireless_thread, SIGNAL(finished()), client, SLOT(deleteLater()));

    connect(client, SIGNAL(pinComplete()), &pinForm, SLOT(hide()));
    connect(client, SIGNAL(deviceConnected(QString)), this, SIGNAL(deviceConnected(QString)));
    connect(client, SIGNAL(deviceDisconnected()), this, SIGNAL(deviceDisconnected()));
    connect(client, SIGNAL(refreshDatabase()), this, SLOT(refreshDatabase()));

    client->moveToThread(wireless_thread);
    wireless_thread->start();
}

void ClientManager::refreshDatabase()
{
    if(!db.reload()) {
        emit messageSent(tr("Cannot refresh the database while is in use"));
    } else {
        progress.show();
    }
}

void ClientManager::stop()
{
    CmaClient::stop();
}

void ClientManager::threadStopped()
{
    mutex.lock();
    if(--thread_count == 0) {
        emit stopped();
    }
    mutex.unlock();
}
