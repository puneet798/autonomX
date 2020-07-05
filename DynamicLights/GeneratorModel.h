// Copyright 2020, Xmodal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QSharedPointer>
#include <QList>
#include <Facade.h>

class GeneratorModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum GeneratorRoles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        DescriptionRole,
        OutputMonitorRole
    };

    GeneratorModel(QList<QSharedPointer<Facade>> generators);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    void populate();
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE Facade * at(int index);

private:
    QList<QSharedPointer<Facade>> generatorFacades;
    bool flagDebug = true;
};
