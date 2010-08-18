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

#ifndef EMAILNOTIFIERDIALOG_H
#define EMAILNOTIFIERDIALOG_H

//Qt
#include <QLabel>
#include <QStringList>

//Akonadi
#include <Akonadi/AgentInstance>
#include <Akonadi/AgentManager>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/TabBar>

//own
#include "emaillist.h"

namespace Plasma
{
    class Icon;
    class Dialog;
}

  /**
  * @short The panel used to display emails in a popup
  *
  */
  class Dialog : public QGraphicsWidget
  {
  Q_OBJECT

    public:
        /**
        * Constructor of the dialog
        * @param parent the parent of this object
        **/
        Dialog(bool showImportant, QGraphicsWidget *parent);
        virtual ~Dialog();

        EmailList* unreadEmailList();

        void setTitle(const QString &title);

    Q_SIGNALS:
        void statusChanged(int count, const QString &statusText);

    private Q_SLOTS:
        void toggleTab();
        void openUrl(const QUrl url);
        void updateNavIcon(int tabIndex);
        void refreshClicked();

        void instanceStatusChanged(const Akonadi::AgentInstance &instance);

    Q_SIGNALS:
        void updateToolTip(const QString&, int);

    private :
        /**
        * @internal build the dialog
        **/
        void buildDialog(bool showImportant);
        void updateStatus(const QString status);

        //Plasma::IconWidget *m_navIcon;
        Plasma::TabBar *m_tabBar;
        Plasma::Label *m_titleBar;
        Plasma::Label *m_statusBar;
        Plasma::IconWidget *m_refreshIcon;

        EmailList* m_unreadList;

        bool m_amConnected;
  };

#endif

