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
//Qt
#include <QGraphicsGridLayout>
#include <QLabel>
#include <QTimer>

//Akonadi
#include <Akonadi/AgentInstance>

//KDE
#include <KDebug>
#include <KGlobalSettings>
#include <KLineEdit>
#include <KRun>

//plasma
#include <Plasma/Dialog>
#include <Plasma/Theme>


//own
#include "dialog.h"


Dialog::Dialog(quint64 collectionId, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      //m_navIcon(0),
      m_titleBar(0),
      m_statusBar(0)
{
    buildDialog();
}

Dialog::~Dialog()
{
}

void Dialog::buildDialog()
{
    QGraphicsGridLayout *gridLayout = new QGraphicsGridLayout(this);
    setLayout(gridLayout);

    m_titleBar = new Plasma::Label(this);
    m_titleBar->setText(i18nc("list title", "<b><font size=\"+1\">&nbsp;&nbsp;&nbsp;New Emails</font></b>"));
    gridLayout->addItem(m_titleBar, 0, 0, 1, 2);


    m_tabBar = new Plasma::TabBar(this);


    m_unreadList = new EmailList(0, this);
    connect(m_unreadList, SIGNAL(activated(const QUrl)), SLOT(openUrl(const QUrl)));
    connect(m_unreadList, SIGNAL(statusChanged(int, const QString&)), this, SIGNAL(statusChanged(int, const QString&)));
    m_tabBar->addTab(KIcon("mail-unread-new"), i18n("Unread"), m_unreadList);
    m_tabBar->setTabBarShown(false);

    gridLayout->addItem(m_tabBar, 1, 0, 1, 3);

    m_statusBar = new Plasma::Label(this);
    //m_statusBar->setText("status");
    m_statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_statusBar->setMaximumHeight(22);
    m_statusBar->setFont(KGlobalSettings::smallestReadableFont());
    gridLayout->addItem(m_statusBar, 2, 0, 1, 2);

    m_refreshIcon = new Plasma::IconWidget(this);
    m_refreshIcon->setIcon("view-refresh");
    m_refreshIcon->setToolTip(i18nc("tooltip on the refresh button", "Check for new email"));
    m_refreshIcon->setMaximumHeight(16);
    gridLayout->addItem(m_refreshIcon, 2, 2, 1, 1);
    connect(m_refreshIcon, SIGNAL(clicked()), this, SLOT(refreshClicked()));

    connect(m_tabBar, SIGNAL(currentChanged(int)), SLOT(updateNavIcon(int)));

    updateStatus(i18nc("no active search, no results shown", "Idle."));
    updateNavIcon(m_tabBar->currentIndex());
    setPreferredSize(540, 320);
}

EmailList* Dialog::unreadEmailList()
{
    return m_unreadList;
}

void Dialog::updateStatus(const QString status)
{
    m_statusBar->setText(status);
}

void Dialog::updateNavIcon(int tabIndex)
{
    return;
}

void Dialog::toggleTab()
{
    if (m_tabBar->currentIndex() == 0) {
        m_tabBar->setCurrentIndex(1);
    } else {
        m_tabBar->setCurrentIndex(0);
    }
}

void Dialog::openUrl(const QUrl url)
{
    kDebug() << "Opening ..." << url;
    KRun::runUrl(url, "message/rfc822", 0);
}

void Dialog::refreshClicked()
{
    kDebug() << "refresh!";
    /*
     *  void AgentManager::synchronizeCollection (   const Collection &  collection   )
        SIGNAL void    instanceStatusChanged (const Akonadi::AgentInstance &instance)
        SIGNAL void    instanceProgressChanged (const Akonadi::AgentInstance &instance)
     *
     */
    Akonadi::AgentInstance::List instances = Akonadi::AgentManager::self()->instances();
    foreach ( const Akonadi::AgentInstance &instance, instances ) {
        kDebug() << "Name:" << instance.name() << "(" << instance.identifier() << ")";
    }
}

#include "dialog.moc"
