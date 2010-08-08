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
#include <Akonadi/CollectionFetchScope>
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

EmailList::EmailList(quint64 collectionId, QGraphicsWidget *parent)
    : Plasma::ScrollWidget(parent),
    m_session(0),
    m_collectionId(collectionId)
{
    //m_collectionId = 108; // FIXME: comment, collection id is hardcoded atm

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    buildEmailList();

    if (m_collectionId > 0) {
        addCollection(collectionId);
    }
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
}

void EmailList::addCollection(const quint64 collectionId)
{
    if (m_etms.keys().contains(collectionId)) {
        kDebug() << "collection already monitored, skipping...";
        return;
        //m_session = new Session( QByteArray( "PlasmaEmailNotifier-" ) + QByteArray::number( qrand() ), this );

    }
    Session* session = new Session( QByteArray( "PlasmaEmailNotifier-" ) + QByteArray::number( qrand() ), this );

    /*
    Monitor *monitor = new Monitor( this );
    monitor->setMimeTypeMonitored("message/rfc822");
    monitor->setCollectionMonitored(Collection::root(), false);
    monitor->itemFetchScope().fetchPayloadPart( MessagePart::Envelope );
    */
    kDebug() << " =====> New ETM, monitoring:" << collectionId;
 
    ChangeRecorder *changeRecorder = new ChangeRecorder( this );

    //changeRecorder->setCollectionMonitored( Collection(201) );
    // // 201 is lion mail local, 191 is INBOX, 111 is lion mail imap
    //changeRecorder->setCollectionMonitored( Collection(201) );
    changeRecorder->setCollectionMonitored( Collection(collectionId) );
    //changeRecorder->setMimeTypeMonitored("inode/directory");
    //changeRecorder->itemFetchScope().fetchPayloadPart(MessagePart::Header);
    changeRecorder->itemFetchScope().fetchPayloadPart(MessagePart::Envelope);
    changeRecorder->collectionFetchScope().setIncludeUnsubscribed(false);
    changeRecorder->setMimeTypeMonitored("message/rfc822");
    changeRecorder->setSession( session );

    Akonadi::EntityTreeModel* model = new Akonadi::EntityTreeModel(changeRecorder, this);
    //m_model->setItemPopulationStrategy( EntityTreeModel::NoItemPopulation );
    model->setCollectionFetchStrategy( EntityTreeModel::FetchNoCollections );
    model->setItemPopulationStrategy(EntityTreeModel::ImmediatePopulation);
    connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(rowAdded(const QModelIndex&, int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(rowsRemoved(const QModelIndex&, int, int)));
    connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex&)), this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));
    m_etms[collectionId] = model;
    kDebug() << "Model created and connected. :) Now holding models for col-ids:" << m_etms.keys() << m_etms.values();

    m_model = model; // FIXME: gnnn..
}

void EmailList::removeCollection(const quint64 collectionId)
{
    kDebug() << "removing collection (not implemented):" << collectionId;
}

void EmailList::deleteItem()
{
    EmailWidget* ew = dynamic_cast<EmailWidget*>(sender());
    if (ew) {
        kDebug() << "Scheduling item for deletion, let's hope we don't crash ;-)";
        QUrl _url = m_emailWidgets.key(ew);
        m_emailWidgets.remove(_url);
        m_listLayout->removeItem(ew);
        delete ew;
        //item->deleteLater();
    } else {
        kDebug() << "Sender is not a QGraphicsWidget, something's wrong with your code.";
    }
}

void EmailList::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    kDebug() << "data changed...";
    int r = topLeft.row();
    int c = topLeft.column(); // We ignore columns, everything is in 0s
    if (topLeft.row() >= 0) {
        while (bottomRight.row() >= r) {
            QModelIndex itemindex = topLeft.model()->index(r, c);
            Akonadi::Item item = itemindex.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();
            if (m_emailWidgets.keys().contains(item.url())) {
                kDebug() << "one of our items changed ..." << item.url() << item.flags();
                m_emailWidgets[item.url()]->itemChanged(item);
                // Should we remove it?
                if (!accept(item)) {
                    kDebug() << "Setting deleted";
                    m_emailWidgets[item.url()]->setDeleted();
                } else {
                    // this is a widget scheduled for deletion
                    m_emailWidgets[item.url()]->setDeleted(false);
                }
                if (!item.isValid()) {
                    kDebug() << "invalid item";
                    m_emailWidgets[item.url()]->setDeleted();
                }
            } else {
                kDebug() << "an item becomes visible" << item.url();
                addItem(item);
            }
            r++;
        }
    }
    updateStatus();
}

void EmailList::rowAdded(const QModelIndex &index, int start, int end)
{
    Akonadi::EntityTreeModel* _model = dynamic_cast<Akonadi::EntityTreeModel*>(sender());
    if (!_model) {
        kDebug() << "sender - model is not OK, damnit!";
    }
    for (int i = start; i <= end; i++) {
        QModelIndex itemindex =  _model->index(i, 0, index);
        Akonadi::Item item = itemindex.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();
        if (!item.isValid()) {
            continue;
        }
        if (!accept(item)) {
            //kDebug() << "item not interesting" << item.url();
        } else if (m_emailWidgets.keys().contains(item.url())) {
            kDebug() << "skipping, item already exists:" << item.url();
        } else {
            addItem(item);
        }
    }
    updateStatus();
}

void EmailList::addItem(Akonadi::Item item)
{
    if (!accept(item)) {
        kWarning() << "adding not-accepted item!";
    }
    EmailWidget* ew = new EmailWidget(this);

    connect(ew, SIGNAL(activated(const QUrl)), SIGNAL(activated(const QUrl)));
    connect(ew, SIGNAL(collapsed()), SLOT(fixLayout()));
    connect(ew, SIGNAL(deleteMe()), SLOT(deleteItem()));

    m_emailWidgets[item.url()] = ew;
    ew->setSmall();
    ew->itemChanged(item);
    m_listLayout->insertItem(0, ew);
    kDebug() << "Item URL:" << item.url() << item.flags() << item.storageCollectionId();
}

void EmailList::rowsRemoved(const QModelIndex &index, int start, int end)
{
    kDebug() << "ROWs Removed!!!!" << start << end;
    //kDebug() << "Total rows, cols:" << index.model()->rowCount() << index.model()->columnCount();
    kDebug() << index.data(EntityTreeModel::MimeTypeRole).value<QString>();
    kDebug() << index.data(EntityTreeModel::ItemIdRole).value<int>();

    for (int i = start; i <= end; i++) {
        QModelIndex itemindex =  index.model()->index(i, 0, index);
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
    updateStatus();
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

    if (!m_etms.keys().contains((quint64)(email.storageCollectionId()))) {
        kDebug() << "wrong collection, doei ...";
        return false;
    } else {

    }
    KPIM::MessageStatus status;
    status.setStatusFromFlags(email.flags());
    //kDebug() << "Flags:" <<     email.flags();

    if (status.isUnread()) {
        //kDebug() << "message is unread";
        return true;
    }
    if (status.isImportant()) {
        //kDebug() << "message is important";
        return true;
    }
    return false;
}

void EmailList::updateStatus()
{
    m_emailsCount = m_emailWidgets.count();
    m_statusText = i18np("%1 new email", "%1 new emails", m_emailsCount);

    emit statusChanged(m_emailsCount, m_statusText);
}

int EmailList::emailsCount()
{
    return m_emailsCount;
}

QString EmailList::statusText()
{
    return m_statusText;
}

#include "emaillist.moc"
