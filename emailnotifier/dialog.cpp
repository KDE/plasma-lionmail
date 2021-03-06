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
//#include <QGraphicsGridLayout>
#include <QGraphicsAnchorLayout>
#include <QLabel>
#include <QTimer>

//KDE
#include <KDebug>
#include <KGlobalSettings>
#include <KIcon>
#include <KLineEdit>
#include <KRun>

// Akonadi
#include <Akonadi/Collection>

//plasma
#include <Plasma/Dialog>
#include <Plasma/PushButton>
#include <Plasma/Theme>


//own

#include "dialog.h"


Dialog::Dialog(bool showImportant, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      //m_navIcon(0),
      m_titleBar(0),
      m_statusBar(0),
      m_unreadList(0),
      m_importantList(0),
      m_amConnected(false)
{
    buildDialog(showImportant);
}

Dialog::~Dialog()
{
}

void Dialog::buildDialog(bool showImportant)
{
    //m_gridLayout = new QGraphicsGridLayout(this);
    //setLayout(m_gridLayout);


    m_tabBar = new Plasma::TabBar(this);

    m_unreadList = new EmailList(showImportant, this);
    connect(m_unreadList, SIGNAL(activated(const QUrl)), SLOT(openUrl(const QUrl)));
    connect(m_unreadList, SIGNAL(statusChanged(int, const QString&)), this, SIGNAL(statusChanged(int, const QString&)));
    connect(m_unreadList, SIGNAL(statusChanged(int, const QString&)), this, SLOT(updateTabs()));
    m_titleBar = new Plasma::Label(this);
    setTitle(i18nc("list title", "New Messages"));


    
    m_tabBar->addTab(KIcon("mail-unread-new"), i18n("New Messages"), m_unreadList);
    m_tabBar->setTabBarShown(false);

    //m_gridLayout->addItem(m_tabBar, 1, 0, 1, 3);

    m_statusBar = new Plasma::Label(this);
    //m_statusBar->setText("status");
    m_statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_statusBar->setMaximumHeight(22);
    m_statusBar->setFont(KGlobalSettings::smallestReadableFont());
    //m_gridLayout->addItem(m_statusBar, 2, 0, 1, 2);

    m_refreshIcon = new Plasma::IconWidget(this);
    m_refreshIcon->setIcon("view-refresh");
    m_refreshIcon->setToolTip(i18nc("tooltip on the refresh button", "Check Mail"));
    m_refreshIcon->setMaximumHeight(16);
    //m_gridLayout->addItem(m_refreshIcon, 2, 2, 1, 1);
    connect(m_refreshIcon, SIGNAL(clicked()), this, SLOT(refreshClicked()));

    connect(m_tabBar, SIGNAL(currentChanged(int)), SLOT(updateNavIcon(int)));


    m_anchorLayout = new QGraphicsAnchorLayout(this);
    /*
    // Fix the actual email widget on top-left and bottom-right corners
    m_anchorLayout->addCornerAnchors(m_emailWidget, Qt::TopLeftCorner,
                                     m_anchorLayout, Qt::TopLeftCorner);
    m_anchorLayout->addCornerAnchors(m_emailWidget, Qt::BottomRightCorner,
                                     m_anchorLayout, Qt::BottomRightCorner);

    m_anchorLayout->addCornerAnchors(m_expandIcon, Qt::TopRightCorner,
                                     m_anchorLayout, Qt::TopRightCorner);
    m_anchorLayout->addAnchor(m_actionsWidget, Qt::AnchorTop, m_anchorLayout, Qt::AnchorTop);
    m_anchorLayout->addAnchor(m_actionsWidget, Qt::AnchorRight, m_expandIcon, Qt::AnchorLeft);

    Plasma::TabBar *m_tabBar;
    Plasma::Label *m_titleBar;
    Plasma::Label *m_statusBar;
    Plasma::IconWidget *m_refreshIcon;
    Plasma::PushButton *m_clearButton;

    */
    m_anchorLayout->addCornerAnchors(m_titleBar, Qt::TopLeftCorner,
                                     m_anchorLayout, Qt::TopLeftCorner);
    m_anchorLayout->addCornerAnchors(m_titleBar, Qt::TopRightCorner,
                                     m_anchorLayout, Qt::TopRightCorner);

    m_anchorLayout->addAnchor(m_tabBar, Qt::AnchorLeft,
                              m_anchorLayout, Qt::AnchorLeft);
    m_anchorLayout->addAnchor(m_tabBar, Qt::AnchorRight,
                              m_anchorLayout, Qt::AnchorRight);

    m_anchorLayout->addCornerAnchors(m_statusBar, Qt::BottomLeftCorner,
                                     m_anchorLayout, Qt::BottomLeftCorner);
    m_anchorLayout->addCornerAnchors(m_refreshIcon, Qt::BottomRightCorner,
                                     m_anchorLayout, Qt::BottomRightCorner);

    m_anchorLayout->addAnchor(m_tabBar, Qt::AnchorTop,
                              m_titleBar, Qt::AnchorBottom);
    m_anchorLayout->addAnchor(m_tabBar, Qt::AnchorBottom,
                              m_statusBar, Qt::AnchorTop);



    setLayout(m_anchorLayout);


    setTitleBarShown();
    setStatus(i18nc("no active search, no results shown", "Idle."));
    updateNavIcon(m_tabBar->currentIndex());
    setPreferredSize(540, 320);
}

EmailList* Dialog::unreadEmailList()
{
    return m_unreadList;
}

ImportantEmailList* Dialog::importantEmailList()
{
    return m_importantList;
}

void Dialog::setStatus(const QString status)
{
    m_statusBar->setText(status);
}

void Dialog::updateTabs()
{
    if (m_importantList) {
        m_tabBar->setTabText(0, i18nc("tab text", "New (%1)", m_unreadList->emailsCount()));
        m_tabBar->setTabText(1, i18nc("tab text", "Important (%1)", m_importantList->emailsCount()));
    }
}

void Dialog::setTitleBarShown(bool show)
{
    if (show) {
        kDebug() << "----------------" << "adding title bar";
        //m_titleBar = new Plasma::Label(this);
        setTitle(i18nc("list title", "New Messages"));
        // FIXME //m_gridLayout->addItem(m_titleBar, 0, 0, 1, 2);
        //m_anchorLayout->addCornerAnchors(m_titleBar, Qt::TopLeftCorner,
                                         //m_anchorLayout, Qt::TopLeftCorner);
        //m_anchorLayout->addAnchor(m_tabBar, Qt::AnchorTop,
        //                      m_titleBar, Qt::AnchorBottom);
        m_titleBar->show();
        setTitle(m_unreadList->statusText());
        m_clearButton = new Plasma::PushButton(this);
        m_clearButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        m_clearButton->setText(i18nc("clear button in emailnotifier popup -- keep short", "Clear"));
        m_clearButton->setToolTip(i18nc("tooltip clear button", "Hide all messages"));
        m_clearButton->hide();
        connect(m_clearButton, SIGNAL(clicked()), m_unreadList, SLOT(hideAllMessages()));
        // FIXME //m_gridLayout->addItem(m_clearButton, 0, 2);
        m_anchorLayout->addCornerAnchors(m_clearButton, Qt::TopRightCorner,
                                         m_anchorLayout, Qt::TopRightCorner);

    } else if (!show && m_titleBar) {
        kDebug() << "----------------" << "removing title bar";
        //m_anchorLayout->addAnchor(m_tabBar, Qt::AnchorTop,
        //                          m_anchorLayout, Qt::AnchorTop);

        //m_titleBar->deleteLater();
        //m_titleBar = 0;
        m_titleBar->hide();
    }
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

void Dialog::addImportantTab(QList<Akonadi::Entity::Id> collectionIds)
{
    if (m_tabBar->count() == 1) {
        /*
        Plasma::Label* label = new Plasma::Label(m_tabBar);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        label->setPreferredSize(400, 400);

        label->setText("<h2>Important emails go here</h2>");
        */
        m_importantList = new ImportantEmailList(collectionIds, m_tabBar);
        m_tabBar->addTab(KIcon("mail-mark-important"), i18nc("tab title", "Important Messages"), m_importantList);
        //m_anchorLayout->addAnchor(m_tabBar, Qt::AnchorBottom,
        //                      m_statusBar, Qt::AnchorTop);

        connect(m_importantList, SIGNAL(statusChanged(int, const QString&)), this, SLOT(updateTabs()));
    }
    setTitleBarShown(false);
    m_tabBar->setTabBarShown(true);
}

void Dialog::removeImportantTab()
{
    if (m_tabBar->currentIndex() == 1) {
        m_tabBar->setCurrentIndex(0);
    }
    if (m_tabBar->count() == 2) {
        m_tabBar->removeTab(1);
    }
    setTitleBarShown();
    m_tabBar->setTabBarShown(false);
    m_importantList = 0;
}

void Dialog::openUrl(const QUrl url)
{
    kDebug() << "Opening ..." << url;
    KRun::runUrl(url, "message/rfc822", 0);
}

void Dialog::addCollection(Akonadi::Collection col)
{
    m_collections[col.id()] = col;
}

void Dialog::clearCollections()
{
    m_collections.clear();
}

void Dialog::refreshClicked()
{
    kDebug() << "refresh!";
    setStatus(i18nc("dialog status", "Checking mail...")); // FIXME: needs to be reset when done
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
        m_amConnected = true;
    }

    //am->synchronizeCollectionTree();

    foreach(Akonadi::Collection c, m_collections) {
        kDebug() << "Syncing collection:" << c.id() << c.name() << c.resource();
        am->synchronizeCollection(c);
    }
    /*
    Akonadi::Collection collection;
    foreach(const Akonadi::Entity::Id id, m_unreadList->collectionIds()) {
        //Akonadi::Entity::Id id = 111;
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
    */
    return;
    // demo...
    /*
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
    //setStatus(i18nc("dialog status", "Checking mail finished."));
    */
}

void Dialog::setTitle(const QString &title)
{
    if (m_titleBar) {
        m_titleBar->setText(QString("<b><font size=\"+1\">&nbsp;&nbsp;&nbsp;%1</font></b>").arg(title));
    }
}

void Dialog::setEmailsCount(const int emailsCount)
{
    if (!emailsCount) {
        m_clearButton->hide();
    } else {
        m_clearButton->show();
    }
}

void Dialog::instanceStatusChanged(const Akonadi::AgentInstance &instance)
{
    QString _s;
    switch (instance.status()) {
        case Akonadi::AgentInstance::Running:
            _s = i18nc("sync status running", "Checking mail in %1", instance.name());
            break;
        case Akonadi::AgentInstance::Idle:
            _s = i18nc("sync status idle", "%1 is idle", instance.name());
            break;
        case Akonadi::AgentInstance::Broken:
            _s = i18nc("sync status error", "Error checking mail in %1", instance.name());
            break;
        case Akonadi::AgentInstance::NotConfigured:
            _s = i18nc("sync status not configured", "%1 is not configured", instance.name());
            break;
    }
    m_statusBar->setText(_s);
    kDebug() << "Instance changed:" << _s << instance.statusMessage() << instance.progress();

    // TODO: $time since last email check when idle :)
}

#include "dialog.moc"
