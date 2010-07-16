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
    
    Session *session = new Session( QByteArray( "PlasmaAkonadiDataEngine-" ) + QByteArray::number( qrand() ), this );
 
    ChangeRecorder *changeRecorder = new ChangeRecorder( this );
    
    //changeRecorder->setCollectionMonitored( Collection(201) );
    //
    changeRecorder->setCollectionMonitored( Collection(201) ); // 191 is INBOX
    //changeRecorder->setMimeTypeMonitored("inode/directory");
    changeRecorder->itemFetchScope().fetchPayloadPart(MessagePart::Envelope);
    changeRecorder->setMimeTypeMonitored("message/rfc822");
    changeRecorder->setSession( session );
    
    m_model = new Akonadi::EntityTreeModel(changeRecorder, this);
    //m_model->setItemPopulationStrategy( EntityTreeModel::NoItemPopulation );
    connect(m_model, SIGNAL(rowsInserted ( const QModelIndex&, int, int)), this, SLOT(rowAdded(const QModelIndex&, int, int)));
    kDebug() << "Model created and connected. :)";
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
        EmailWidget* ew = new EmailWidget(this);
        ew->setSmall();
        ew->itemChanged(item);
        m_listLayout->addItem(ew);
        kDebug() << "Item URL:" << item.url();
    }
    
    
}


#include "emaillist.moc"