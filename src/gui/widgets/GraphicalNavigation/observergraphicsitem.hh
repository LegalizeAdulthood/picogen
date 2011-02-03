//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2011  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef OBSERVERGRAPHICSITEM_HH
#define OBSERVERGRAPHICSITEM_HH

#include <QGraphicsItem>
#include <QVector2D>
#include "observer.hh"

class ObserverGraphicsItem : public QGraphicsItem
{
public:
        ObserverGraphicsItem();

        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                   QWidget *widget);

private:
        QVector2D relateMouseToOwnPos (QPointF mouse);

        void paintViewingDirection(QPainter *painter) const;
        void paintHorizon(QPainter *painter) const;
        void paintSun(QPainter *painter) const;
        void paintCenterMarker (QPainter *painter) const;

private:
        Observer observer_;

protected:
        enum MouseMoveEffect {
                mm_do_nothing = 0,
                mm_change_position,
                mm_change_yaw
        };
        MouseMoveEffect mouseMoveEffect;

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // OBSERVERGRAPHICSITEM_HH
