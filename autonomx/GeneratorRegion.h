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
#include <QRect>

class GeneratorRegion : public QObject {
    Q_OBJECT
    Q_PROPERTY(QRect rect READ getRect WRITE writeRect NOTIFY rectChanged)
    Q_PROPERTY(double intensity READ getIntensity WRITE writeIntensity NOTIFY intensityChanged)
public:
    GeneratorRegion(QRect rect, double intensity);
    GeneratorRegion(const GeneratorRegion& region);

    QRect getRect() const;
    double getIntensity() const;

    void writeRect(QRect rect);
    void writeIntensity(double intensity);
private:
    QRect rect;
    double intensity;
signals:
    void rectChanged(QRect rect);
    void intensityChanged(double intensity);
};
