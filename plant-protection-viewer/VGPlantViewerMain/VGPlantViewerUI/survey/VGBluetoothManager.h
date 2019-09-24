/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009, 2015 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/// @file
///     @author Lorenz Meier <mavteam@student.ethz.ch>

#ifndef __VGBLUETOOTHMANAGER_H__
#define __VGBLUETOOTHMANAGER_H__

#include <QObject>

class BluetoothCommand;
class LinkInterface;
class VGBluetoothManager : public QObject
{
    Q_OBJECT
public:
    explicit VGBluetoothManager(QObject *parent = NULL);
    ~VGBluetoothManager();

    Q_INVOKABLE void linkBlt(BluetoothCommand *cmd);
    Q_INVOKABLE bool isConnected(const QString &devName);
    Q_INVOKABLE void shutdown(void);
private slots:
    void _linkConnected(bool b);
    void onBltlinkDestroied(QObject *obj);
private:
    void _prcsLink(LinkInterface *link);
private:
    BluetoothCommand  *m_bltCmd;
};

#endif
