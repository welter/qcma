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

#ifndef CMAOBJECT_H
#define CMAOBJECT_H

#include <QFileInfo>
#include <QString>

extern "C" {
#include <vitamtp.h>
}

#define OHFI_OFFSET 1000

class CMAObject
{
public:
    explicit CMAObject(CMAObject *parent = 0);
    ~CMAObject();

    void refreshPath();
    void rename(const QString &name);
    bool removeReferencedObject();
    void initObject(const QFileInfo &file);
    void updateObjectSize(unsigned long size);
    bool hasParent(const CMAObject *obj);

    bool operator==(const CMAObject &obj);
    bool operator!=(const CMAObject &obj);
    bool operator<(const CMAObject &obj);

    void setOhfi(int ohfi) {
        metadata.ohfi = ohfi;
    }

    static void resetOhfiCounter() {
        ohfi_count = OHFI_OFFSET;
    }

    QString path;
    CMAObject *parent;
    metadata_t metadata;

protected:
    static int ohfi_count;

private:
    void loadSfoMetadata(const QString &path);
    void loadMusicMetadata(const QString &path);
    void loadVideoMetadata(const QString &path);
    void loadPhotoMetadata(const QString &path);
};

#endif // CMAOBJECT_H