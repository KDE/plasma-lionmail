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
        EmailList(quint64 collectionId, QGraphicsWidget *parent);

        virtual ~EmailList();

        virtual int emailsCount();
        virtual QString statusText();

        virtual void addCollection(const quint64 collectionId);
        virtual void removeCollection(const quint64 collectionId);
        QList<quint64> collectionIds();

    Q_SIGNALS:
        void updateToolTip(const QString&, int);
        void activated(const QUrl);
        void statusChanged(int count, const QString &statusText);

    protected:
        virtual bool accept(const Akonadi::Item email);
        void fetchItem(const quint64 id);

        Akonadi::Session* m_session;
        QHash<QUrl, EmailWidget*> m_emailWidgets;
        QHash<quint64, Akonadi::EntityTreeModel*> m_etms;

        int m_emailsCount;
        QString m_statusText;

    private Q_SLOTS:
        void rowAdded(const QModelIndex &index, int start, int end);
        void rowsRemoved(const QModelIndex &index, int start, int end);
        void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
        void itemChanged(Akonadi::Item item);
        void fetchDone(KJob* job);
        void updateStatus();

        void deleteItem();

    private :
        /**
        * @internal build the dialog
        **/
        void buildEmailList();
        //void initETM();
        void addItem(Akonadi::Item item);


        QGraphicsWidget* m_innerWidget;
        QGraphicsLinearLayout* m_listLayout;


  };

#endif

