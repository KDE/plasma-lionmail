/*
    Copyright 2010 by Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef EMAILLIST_H
#define EMAILLIST_H

//Qt
#include <QGraphicsLinearLayout>
#include <QStringList>
#include <QModelIndex>

// KDE
#include <KDebug>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/ScrollWidget>

//own
#include "emailwidget.h"

namespace Plasma
{
    class Icon;
}

namespace Akonadi {
    class EntityTreeModel;
}

  /**
  * @short The list of emails
  *
  */
  class EmailList : public Plasma::ScrollWidget
  {
  Q_OBJECT

    public:
        EmailList(QGraphicsWidget *parent);

        virtual ~EmailList();

    Q_SIGNALS:
        void updateToolTip(const QString&, int);
        
    private Q_SLOTS:
        void rowAdded(const QModelIndex &index, int start, int end);


    private :
        /**
        * @internal build the dialog
        **/
        void buildEmailList();
        void initETM();
        
        Akonadi::EntityTreeModel* m_model;
        QGraphicsWidget* m_innerWidget;
        QGraphicsLinearLayout* m_listLayout;
  };

#endif

