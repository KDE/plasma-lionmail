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
#include <QGraphicsSceneDragDropEvent>
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
#include "importantemaillist.h"

// In order to reverse the layout, we subtract the modelindex's row
// from MAX_EMAILS, so newer emails end up at the top of the list
#define MAX_EMAILS 10000000

using namespace Akonadi;

ImportantEmailList::ImportantEmailList(QList<Akonadi::Entity::Id> collectionIds, QGraphicsWidget *parent)
    : EmailList(false, parent)
{
    foreach(const Akonadi::Entity::Id colId, collectionIds) {
        addCollection(colId);
    }
}

ImportantEmailList::~ImportantEmailList()
{
}

bool ImportantEmailList::accept(const Akonadi::Item email)
{
    // discard collections we're not interested in
    if (!m_etms.keys().contains((Akonadi::Entity::Id)(email.storageCollectionId()))) {
        return false;
    }

    Akonadi::MessageStatus status;
    status.setStatusFromFlags(email.flags());

    // deleted emails be gone
    if (status.isDeleted()) {
        return false;
    }
    // Show important emails
    if (status.isImportant()) {
        return true;
    }
    // We accept unread emails
    return false;
}

int ImportantEmailList::emailsCount()
{
    return m_emailWidgets.count();
}

#include "importantemaillist.moc"
