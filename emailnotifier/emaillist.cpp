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
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/EntityTreeModel>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/kmime/messagestatus.h>

//KDE
#include <KDebug>
#include <KGlobalSettings>
#include <KLineEdit>
#include <KRun>

//plasma
#include <Plasma/Theme>

//own
#include "emaillist.h"

using namespace Akonadi;

EmailList::EmailList(bool showImportant, QGraphicsWidget *parent)
    : Plasma::ScrollWidget(parent),
    m_session(0),
    m_showImportant(showImportant)
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

    setPreferredSize(400, 400);
}

QList<quint64> EmailList::collectionIds()
{
    return m_etms.keys();
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
}

void EmailList::removeCollection(const quint64 collectionId)
{
    kDebug() << "removing collection (not implemented):" << collectionId;
}

void EmailList::deleteItem()
{
    EmailWidget* ew = dynamic_cast<EmailWidget*>(sender());
    if (ew) {
        QUrl _url = m_emailWidgets.key(ew);
        m_emailWidgets.remove(_url);
        m_listLayout->removeItem(ew);
        delete ew;
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
            quint64 id = itemindex.data(EntityTreeModel::ItemIdRole).value<quint64>();
            fetchItem(id);
            r++;
        }
    }
    updateStatus();
}

void EmailList::fetchItem(const quint64 id)
{
    if (id <= 0) {
        kDebug() << "id invalid";
        return;
    }
    //kDebug() << "Fetching payload for " << id;
    Akonadi::ItemFetchJob* fetchJob = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
    //fetchJob->fetchScope().fetchFullPayload();
    fetchJob->fetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope );
    connect( fetchJob, SIGNAL(result(KJob*)), SLOT(fetchDone(KJob*)) );
}

void EmailList::fetchDone(KJob* job)
{
    if ( job->error() ) {
        kDebug() << "!!! Error fetching item: " << job->errorString();
        return;
    }
    Akonadi::Item::List items = static_cast<Akonadi::ItemFetchJob*>(job)->items();

    //kDebug() << "Fetched" << items.count() << "email Items.";
    if (items.count() == 0) {
        kDebug() << "job ok, but no item returned";
        return;
    }
    foreach( const Akonadi::Item &item, items ) {
        itemChanged(item);
    }
}

void EmailList::itemChanged(Akonadi::Item item)
{
    /*
    if (!item.isValid()) {
        kDebug() << "Invalid item, skipping itemChanged()";
        return;
    }
    */
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
        //kDebug() << "an item becomes visible" << item.url();
        if (accept(item)) {
            addItem(item);
        }
    }
}

void EmailList::rowAdded(const QModelIndex &index, int start, int end)
{
    Akonadi::EntityTreeModel* _model = dynamic_cast<Akonadi::EntityTreeModel*>(sender());
    if (!_model) {
        kDebug() << "sender - model is not OK, damnit!";
    }
    for (int i = start; i <= end; i++) {
        QModelIndex itemindex =  _model->index(i, 0, index);
        quint64 id = itemindex.data(EntityTreeModel::ItemIdRole).value<quint64>();
        fetchItem(id);
    }
}

void EmailList::addItem(Akonadi::Item item)
{
    if (!accept(item)) {
        kWarning() << "adding not-accepted item!";
    }
    EmailWidget* ew = new EmailWidget(this);

    connect(ew, SIGNAL(activated(const QUrl)), SIGNAL(activated(const QUrl)));
    //connect(ew, SIGNAL(collapsed()), SLOT(fixLayout()));
    connect(ew, SIGNAL(deleteMe()), SLOT(deleteItem()));

    m_emailWidgets[item.url()] = ew;
    ew->setSmall();
    ew->itemChanged(item);
    m_listLayout->insertItem(0, ew);
    kDebug() << "Added Item:" << item.url() << item.flags() << item.storageCollectionId();
    updateStatus();
}

void EmailList::rowsRemoved(const QModelIndex &index, int start, int end)
{
    //kDebug() << "ROWs Removed!!!!" << start << end;
    //kDebug() << "Total rows, cols:" << index.model()->rowCount() << index.model()->columnCount();
    //kDebug() << index.data(EntityTreeModel::MimeTypeRole).value<QString>();
    //kDebug() << index.data(EntityTreeModel::ItemIdRole).value<int>();
    if (!index.isValid()) {
        kDebug() << "invalid ModelIndex while removing item";
        return;
    }
    for (int i = start; i <= end; i++) {
        QModelIndex itemindex =  index.model()->index(i, 0, index);
        quint64 _id = itemindex.data(EntityTreeModel::ItemIdRole).value<quint64>();
        Akonadi::Item item = Akonadi::Item(_id);
        //EmailWidget* ew = new EmailWidget(this);
        if (m_emailWidgets.keys().contains(item.url())) {
            kDebug() << "Removing item" << item.url();
            QGraphicsWidget* ew = m_emailWidgets[item.url()];
            m_listLayout->removeItem(ew);
            delete ew;
            m_emailWidgets.remove(item.url());
        }
        //ew->setSmall();
        //ew->itemChanged(item);
        kDebug() << "Item gone URL:" << item.url();
    }
    //fixLayout();
    updateStatus();
}

void EmailList::setShowImportant(bool show)
{
    m_showImportant = show;
    filter();
}


void EmailList::filter()
{
    // Filter out items that don't match anymore
    foreach(EmailWidget* widget, m_emailWidgets) {
        itemChanged(widget->item());
    }
    // TODO:: add important items items
}

bool EmailList::accept(const Akonadi::Item email)
{
    // discard collections we're not interested in
    if (!m_etms.keys().contains((quint64)(email.storageCollectionId()))) {
        return false;
    }
    Akonadi::MessageStatus status;
    status.setStatusFromFlags(email.flags());

    // Conditionally show important emails
    if (m_showImportant && status.isImportant()) {
        return true;
    }
    // We accept unread emails
    return status.isUnread();
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
