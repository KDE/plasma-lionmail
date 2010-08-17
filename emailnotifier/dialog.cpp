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


Dialog::Dialog(bool showImportant, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      //m_navIcon(0),
      m_titleBar(0),
      m_statusBar(0),
      m_amConnected(false)
{
    buildDialog(showImportant);
}

Dialog::~Dialog()
{
}

void Dialog::buildDialog(bool showImportant)
{
    QGraphicsGridLayout *gridLayout = new QGraphicsGridLayout(this);
    setLayout(gridLayout);

    m_titleBar = new Plasma::Label(this);
    m_titleBar->setText(i18nc("list title", "<b><font size=\"+1\">&nbsp;&nbsp;&nbsp;New Emails</font></b>"));
    gridLayout->addItem(m_titleBar, 0, 0, 1, 2);


    m_tabBar = new Plasma::TabBar(this);


    m_unreadList = new EmailList(showImportant, this);
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
    Q_UNUSED(tabIndex);
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
    Akonadi::AgentManager* am = Akonadi::AgentManager::self();
    if (!m_amConnected) {
        connect(am, SIGNAL(instanceStatusChanged(const Akonadi::AgentInstance&)),
                this, SLOT(instanceStatusChanged(const Akonadi::AgentInstance&)));
        connect(am, SIGNAL(instanceProgressChanged(const Akonadi::AgentInstance&)),
                this, SLOT(instanceStatusChanged(const Akonadi::AgentInstance&)));
    }

    //am->synchronizeCollectionTree();

    Akonadi::Collection collection;
    foreach(const quint64 id, m_unreadList->collectionIds()) {
        //quint64 id = 111;
        kDebug() << "Connected, now syncing:" << id;
        Akonadi::Collection collection = Akonadi::Collection(id);
        if (collection.isValid()) {
            if (collection.resource().isEmpty()) {
                collection.setResource("akonadi_imap_resource_0");
                kWarning() << "invalid resource string, set to" << collection.resource() << " to not make AgentManager crash (!!!)";
            }
            kDebug() << "collection is good" << collection.resource();
            am->synchronizeCollection(collection);
        }
    }
    //return;
    // demo...
    Akonadi::AgentInstance::List instances = am->instances();
    foreach (const Akonadi::AgentInstance &instance, instances ) {
        //kDebug() << "Name:" << instance.name() << "(" << instance.identifier() << ")"
                 //<< instance.isOnline() << instance.statusMessage();
        if (instance.identifier() == collection.resource()) {
            Akonadi::AgentInstance inst = Akonadi::AgentInstance(instance);
            //inst.synchronizeCollectionTree();
            inst.synchronize();
            kDebug() << "this might be ours" << instance.identifier();
        }
    }
}

void Dialog::instanceStatusChanged(const Akonadi::AgentInstance &instance)
{
    QString _s;
    switch (instance.status()) {
        case Akonadi::AgentInstance::Running:
            _s = i18nc("sync status running", "Checking email in %1", instance.name());
            break;
        case Akonadi::AgentInstance::Idle:
            _s = i18nc("sync status idle", "%1 is idle", instance.name());
            break;
        case Akonadi::AgentInstance::Broken:
            _s = i18nc("sync status error", "Error checking email in %1", instance.name());
            break;
    }
    m_statusBar->setText(_s);
    kDebug() << "Instance changed:" << _s << instance.statusMessage() << instance.progress();

    // TODO: $time since last email check when idle :)
}

#include "dialog.moc"
