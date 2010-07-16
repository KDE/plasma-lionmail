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
//#include "helpers.cpp"


Dialog::Dialog(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_navIcon(0),
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

    m_navIcon = new Plasma::IconWidget(this);
    m_navIcon->setIcon("go-next");
    m_navIcon->setMaximumSize(22, 22);
    m_navIcon->setMinimumSize(22, 22);
    gridLayout->addItem(m_navIcon, 0, 0);
    
    m_titleBar = new Plasma::Label(this);
    m_titleBar->setText(i18nc("list title", "<b><font size=\"+1\">New Emails</font></b>"));
    gridLayout->addItem(m_titleBar, 0, 1);


    m_tabBar = new Plasma::TabBar(this);


    //m_resultsView = new Crystal::ResultWebView(this);
    //m_resultsView = new Crystal::ResultWidget(m_tabBar);
    m_unreadList = new EmailList(this);
    m_tabBar->addTab(KIcon("mail-unread-new"), i18n("Unread"), m_unreadList);
    m_tabBar->setTabBarShown(false);

    gridLayout->addItem(m_tabBar, 1, 0, 1, 3);

    m_statusBar = new Plasma::Label(this);
    //m_statusBar->setText("status");
    m_statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_statusBar->setMaximumHeight(22);
    m_statusBar->setFont(KGlobalSettings::smallestReadableFont());
    gridLayout->addItem(m_statusBar, 2, 0, 1, 3);

    connect(m_tabBar, SIGNAL(currentChanged(int)), SLOT(updateNavIcon(int)));

    updateStatus(i18nc("no active search, no results shown", "Idle."));
    updateNavIcon(m_tabBar->currentIndex());
    setPreferredSize(400, 400);
}

void Dialog::updateStatus(const QString status)
{
    m_statusBar->setText(status);
}


void Dialog::updateNavIcon(int tabIndex)
{
    if (tabIndex == 0) {
        m_navIcon->setEnabled(true); // may be false ;)
        m_navIcon->setIcon("go-next");
    } else {
        m_navIcon->setEnabled(true);
        m_navIcon->setIcon("go-previous");
    }
}

void Dialog::toggleTab()
{
    if (m_tabBar->currentIndex() == 0) {
        m_tabBar->setCurrentIndex(1);
    } else {
        m_tabBar->setCurrentIndex(0);
    }
}

#include "dialog.moc"
