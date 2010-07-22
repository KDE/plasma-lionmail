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

// Akonadi
#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/EntityTreeModel>
#include <akonadi/kmime/messageparts.h>

//KDE
#include <KDebug>
#include <KGlobalSettings>
#include <KLineEdit>
#include <KRun>

//plasma
#include <Plasma/Theme>


//own
#include "emaillist.h"
//#include "helpers.cpp"

#include "copied_classes/messagestatus.h"


using namespace Akonadi;

EmailList::EmailList(QGraphicsWidget *parent)
    : Plasma::ScrollWidget(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    buildEmailList();
}

EmailList::~EmailList()
{
}

void EmailList::buildEmailList()
{
    m_innerWidget = new QGraphicsWidget(this);
    setWidget(m_innerWidget);
    m_listLayout = new QGraphicsLinearLayout(m_innerWidget);
    m_listLayout->setOrientation(Qt::Vertical);
    m_innerWidget->setLayout(m_listLayout);

    /*
    Plasma::IconWidget *w1 = new Plasma::IconWidget(this);
    w1->setOrientation(Qt::Horizontal);
    w1->setIcon("mail-unread-new");
    w1->setText("Fake Email");
    m_listLayout->addItem(w1);
    
    Plasma::IconWidget *w2 = new Plasma::IconWidget(this);
    w2->setOrientation(Qt::Horizontal);
    w2->setIcon("mail-unread-new");
    w2->setText("Yet Another Fake Email");
    m_listLayout->addItem(w2);
    */
    setPreferredSize(400, 400);
    initETM();
}

void EmailList::initETM()
{
    /*
    Monitor *monitor = new Monitor( this );
    monitor->setMimeTypeMonitored("message/rfc822");
    monitor->setCollectionMonitored(Collection::root(), false);
    monitor->itemFetchScope().fetchPayloadPart( MessagePart::Envelope );
    */
    
    Session *session = new Session( QByteArray( "PlasmaEmailNotifier-" ) + QByteArray::number( qrand() ), this );
 
    ChangeRecorder *changeRecorder = new ChangeRecorder( this );
    
    //changeRecorder->setCollectionMonitored( Collection(201) );
    // // 201 is lion mail local, 191 is INBOX, 111 is lion mail imap
    changeRecorder->setCollectionMonitored( Collection(201) ); 
    //changeRecorder->setMimeTypeMonitored("inode/directory");
    changeRecorder->itemFetchScope().fetchPayloadPart(MessagePart::Header);
    changeRecorder->setMimeTypeMonitored("message/rfc822");
    changeRecorder->setSession( session );
    
    m_model = new Akonadi::EntityTreeModel(changeRecorder, this);
    //m_model->setItemPopulationStrategy( EntityTreeModel::NoItemPopulation );
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(rowAdded(const QModelIndex&, int, int)));
    connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(rowsRemoved(const QModelIndex&, int, int)));
    connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex&)), this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));
    kDebug() << "Model created and connected. :)";
}

void EmailList::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int r = topLeft.row();
    int c = topLeft.column(); // We ignore columns, everything is in 0
    if (topLeft.row() >= 0) {
        while (bottomRight.row() >= r) {
            QModelIndex itemindex = m_model->index(r, c);
            Akonadi::Item item = itemindex.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();
            if (m_emailWidgets.keys().contains(item.url())) {
                kDebug() << "one of our items changed ..." << item.url() << item.flags();
                m_emailWidgets[item.url()]->itemChanged(item);
                // Should we remove it?
                if (!accept(item)) {
                    m_emailWidgets[item.url()]->setDeleted();
                } else {
                    // this is a widget scheduled for deletion
                    m_emailWidgets[item.url()]->setDeleted(false);
                }
            } else {
                kDebug() << "an item becomes visible" << item.url();
                addItem(item);
            }
            r++;
        }
    }
}

void EmailList::rowAdded(const QModelIndex &index, int start, int end)
{
    kDebug() << "New ROW!!!!" << start << end;
    kDebug() << "Total rows:" << m_model->rowCount() << m_model->columnCount();
    kDebug() << index.data(EntityTreeModel::MimeTypeRole).value<QString>();
    kDebug() << index.data(EntityTreeModel::ItemIdRole).value<int>();
    
    for (int i = start; i <= end; i++) {
        QModelIndex itemindex =  m_model->index(i, 0, index);
        Akonadi::Item item = itemindex.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();
        if (!accept(item)) {
            kDebug() << "item not interesting" << item.url();
        } else if (m_emailWidgets.keys().contains(item.url())) {
            kDebug() << "skipping, item already exists:" << item.url();
        } else {
            addItem(item);
        }
    }
}

void EmailList::addItem(Akonadi::Item item)
{
    if (!accept(item)) {
        kWarning() << "adding not-accepted item!";
    }
    EmailWidget* ew = new EmailWidget(this);
    connect(ew, SIGNAL(activated(const QUrl)), SIGNAL(activated(const QUrl)));
    connect(ew, SIGNAL(collapsed()), SLOT(fixLayout()));
    m_emailWidgets[item.url()] = ew;
    ew->setSmall();
    ew->itemChanged(item);
    m_listLayout->addItem(ew);
    kDebug() << "Item URL:" << item.url() << item.flags();

}

void EmailList::rowsRemoved(const QModelIndex &index, int start, int end)
{
    kDebug() << "ROWs Removed!!!!" << start << end;
    kDebug() << "Total rows, cols:" << m_model->rowCount() << m_model->columnCount();
    kDebug() << index.data(EntityTreeModel::MimeTypeRole).value<QString>();
    kDebug() << index.data(EntityTreeModel::ItemIdRole).value<int>();
    
    for (int i = start; i <= end; i++) {
        QModelIndex itemindex =  m_model->index(i, 0, index);
        Akonadi::Item item = itemindex.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();
        //EmailWidget* ew = new EmailWidget(this);
        if (m_emailWidgets.keys().contains(item.url())) {
            QGraphicsWidget* ew = m_emailWidgets[item.url()];
            m_listLayout->removeItem(ew);
            delete ew;
            m_emailWidgets.remove(item.url());
        }
        //ew->setSmall();
        //ew->itemChanged(item);
        kDebug() << "Item gone URL:" << item.url();
    }
    fixLayout();
}

void EmailList::fixLayout()
{
    kDebug() << "fixlayout";
    setMinimumHeight(-1);
    m_listLayout->setMaximumHeight(-1);
    m_listLayout->updateGeometry();
    updateGeometry();
}

bool EmailList::accept(const Akonadi::Item email)
{
    KPIM::MessageStatus status;
    status.setStatusFromFlags(email.flags());

    if (status.isUnread()) {
        return true;
    }
    if (status.isImportant()) {
        return true;
    }
    return false;
}

#include "emaillist.moc"
