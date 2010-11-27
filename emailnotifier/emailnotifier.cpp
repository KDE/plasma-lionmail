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

// Qt
#include <QTreeView>

// KDE
#include <KConfigDialog>
#include <kselectionproxymodel.h>
#include <kcheckableproxymodel.h>

// Plasma
#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/Extender>
#include <Plasma/DataEngine>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

// Akonadi
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityTreeModel>
#include <akonadi/etmviewstatesaver.h>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ServerManager>
#include <Akonadi/Session>
#include <akonadi/entitymimetypefiltermodel.h>

// Own
#include "dialog.h"
#include "emailnotifier.h"


using namespace Akonadi;

EmailNotifier::EmailNotifier(QObject *parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args),
    m_allowHtml(false),
    //m_theme(0),
    ui(0),
    m_dialog(0)
{
    setPopupIcon("mail-mark-unread");
    setHasConfigurationInterface(true);
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPassivePopup(true);
    //setStatus(Plasma::ActiveStatus);

    kDebug() << "LionmailArgs" << args;
    foreach (QVariant a, args) {
        kDebug() << args.at(0).toString() << a.toString();
        int id = args.at(0).toString().toInt();
        if (id > 0) {
            //m_collectionIds << id;
            m_newCollectionIds << id;
        }
    }

    if (!m_newCollectionIds.count()) {
        kDebug() << "found init collections:" << m_newCollectionIds;
    }
}

EmailNotifier::~EmailNotifier()
{
    delete ui;
}

void EmailNotifier::findDefaultCollections()
{
    //return;
    if (m_dialog) {
        m_dialog->setStatus(i18nc("dialog status", "Searching for Inbox folders..."));
    }
    Collection emailCollection(Collection::root());
    emailCollection.setContentMimeTypes(QStringList() << "message/rfc822");
    CollectionFetchJob *fetch = new CollectionFetchJob(emailCollection, CollectionFetchJob::Recursive);
    connect( fetch, SIGNAL(result(KJob*)), SLOT(findDefaultCollectionsDone(KJob*)) );
}

void EmailNotifier::findDefaultCollectionsDone(KJob* job)
{
    kDebug() << "-------------------------------- results:";
    // called when the job fetching email collections from Akonadi emits result()
    if ( job->error() ) {
        kDebug() << "Job Error:" << job->errorString();
        return;
    }
    if (m_collectionIds.count()) {
        kDebug() << "Config has changed in between, not setting default collections.";
        return;
    }

    CollectionFetchJob* cjob = static_cast<CollectionFetchJob*>( job );
    QList<quint64> defaultCollections;
    QString _inbox = i18nc("used for string comparison for finding default email folder", "Inbox");
    foreach( const Collection &collection, cjob->collections() ) {
        if (collection.contentMimeTypes().contains("message/rfc822")) {
            QString n = collection.name();
            kDebug() << "collection" << n;
            if ((n.toLower() == QString("inbox")) ||
                (n.toLower() == _inbox.toLower())) {
                kDebug() << "Found an INBOX:" << collection.name();
                defaultCollections << (quint64)(collection.id());
            }
        }
    }
    kDebug() << defaultCollections.count() << "Email collections are in now" << defaultCollections;

    if (!defaultCollections.count()) {
        kWarning() << "No default collections found.";
        return;
    }
    if (m_dialog->importantEmailList()) {
        m_dialog->importantEmailList()->clear();
    }
    m_dialog->unreadEmailList()->clear();
    foreach(const quint64 _id, defaultCollections) {
        if (m_dialog->importantEmailList()) {
            // Then we add those collections that weren't previously in the list
            m_dialog->importantEmailList()->addCollection(_id);
        }
        m_dialog->unreadEmailList()->addCollection(_id);
    }
    if (m_dialog) {
        m_dialog->setStatus(i18ncp("dialog status", "Added %1 inbox.", "Added %1 inbox folders.",
                                    defaultCollections.count()));
    }
}
/*
void EmailNotifier::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    //setBusy(false);
    kDebug() << "source" << source;
    if (source == "EmailCollections") {
        kDebug() << "EmailCollections are in..." << data.keys();
        foreach(const QString &k, data.keys()) {
            QString _k = k;
            quint64 _id = (quint64)(_k.remove("EmailCollection-").toInt());
            m_allCollections[_id] = data[k].toString();
            kDebug() << k << "id" << _id << m_allCollections[_id];
        }
        //addConfigCollections();
        //update();
        //return;
    }
    / *
    if (source == "ContactCollections") {
        kDebug() << "Akonadi contacts collections:" << data.keys() << data;
    }
    * /
}
*/
QGraphicsWidget* EmailNotifier::graphicsWidget()
{
    if (!m_dialog) {
        m_dialog = new Dialog(m_showImportant == ShowMerged, this);
        m_dialog->unreadEmailList()->setShowImportant(m_showImportant == ShowMerged);
        if (m_showImportant == ShowSeparately) {
            m_dialog->addImportantTab(m_collectionIds);
        } else {
            m_dialog->removeImportantTab(); // no-op if the tab isn't there to begin with
        }

        connect(m_dialog, SIGNAL(statusChanged(int, const QString&)), this, SLOT(statusChanged(int, const QString&)));
        foreach (const quint64 id, m_collectionIds) {
            kDebug() << "adding unread:" << id;
            m_dialog->unreadEmailList()->addCollection(id);
        }
    }

    return m_dialog;
}

bool EmailNotifier::allowHtml()
{
    return m_allowHtml;
}

void EmailNotifier::init()
{
    //Akonadi::ServerManager::start();
    //dataEngine("akonadi")->connectSource("EmailCollections", this);
    setStatus(Plasma::PassiveStatus);

    configChanged();
    // We use this var the first time for adding the collections passed on the
    // command line, and subsequently to compare selections
    m_collectionIds << m_newCollectionIds;
    kDebug() << "add collection ids:" << m_collectionIds;
    if (m_dialog) {
        foreach(const quint64 _id, m_collectionIds) {
            kDebug() << "ID" << _id << "adding to monitored collections...";
            m_dialog->unreadEmailList()->addCollection(_id);
        }
    }

    updateToolTip(i18nc("tooltip on startup", "No new email"), "mail-mark-unread");
    //dataEngine("akonadi")->connectSource("EmailCollections", this);
    if (!m_collectionIds.count()) {
        findDefaultCollections();
    }
}


void EmailNotifier::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui = new Ui::emailnotifierConfig();
    ui->setupUi(widget);
    parent->addPage(widget, i18n("Mail"), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    kDebug() << "CREATING SESSION _______________________________";
    Akonadi::Session* session = new Session(QByteArray( "PlasmaEmailNotifier-" ) + QByteArray::number( qrand() ), this);

    // Set a model that displays only folders containing emails onto the treeView
    ChangeRecorder *changeRecorder = new ChangeRecorder( this );
    changeRecorder->setMimeTypeMonitored("message/rfc822");
    changeRecorder->setCollectionMonitored(Collection::root());
    changeRecorder->collectionFetchScope().setIncludeUnsubscribed(false);
    changeRecorder->fetchCollection(true);
    changeRecorder->setAllMonitored(true);
    //changeRecorder->itemFetchScope().fetchFullPayload( true );
    //changeRecorder->itemFetchScope().fetchAllAttributes( true );
    changeRecorder->setSession(session);

    EntityTreeModel *etm = new EntityTreeModel(changeRecorder, this);
    etm->setCollectionFetchStrategy(Akonadi::EntityTreeModel::FetchCollectionsRecursive);
    etm->setItemPopulationStrategy(Akonadi::EntityTreeModel::ImmediatePopulation);
    Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);

    collectionFilter->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
    collectionFilter->setSourceModel(etm);
    collectionFilter->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);

    m_checkSelection = new QItemSelectionModel(collectionFilter, this);
    m_checkable = new KCheckableProxyModel(this);
    m_checkable->setSourceModel(collectionFilter);
    m_checkable->setSelectionModel(m_checkSelection);

    QTreeView *treeView = ui->collectionsTreeView;
    treeView->setModel(m_checkable);
    treeView->expandAll();

    /*
    EntityTreeModel *etm = new EntityTreeModel( changeRecorder, this );

    Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);

    collectionFilter->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
    collectionFilter->setSourceModel(etm);
    collectionFilter->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);

    CheckableItemProxyModel *checkablePM = new CheckableItemProxyModel(this);

    m_checkSelection = new QItemSelectionModel(collectionFilter, this);

    checkablePM->setSelectionModel(m_checkSelection);
    checkablePM->setSourceModel(collectionFilter);
    treeView->setModel(checkablePM);
    //treeView->setSelectionModel(m_checkSelection );
    */

    // Restore the selection, expansion and scrollstate of our treeview
    // restoring on the heap, as KViewStateSaver's api docs suggest
    Akonadi::ETMViewStateSaver* viewState = new Akonadi::ETMViewStateSaver(this);
    viewState->setView(treeView);
    viewState->setSelectionModel(m_checkSelection);
    viewState->restoreState(config());

    // Make sure the UI is in sync with the collections of the applet
    // this is important so we can add default collections programmatically

    // FIXME: this doesn't work, as the collections come in async
    /*
    foreach (quint64 cid, m_collectionIds) {
        QModelIndex idx = EntityTreeModel::modelIndexForCollection( etm, Collection( cid ) );
        if (idx.isValid()) {
            // FIXME: collections come up async, we cannot actually just setChecked() here...
            m_checkSelection->select(idx, QItemSelectionModel::Select);
        }
    }
    */
    ui->allowHtml->setChecked(m_allowHtml);
    ui->showImportant->setChecked(m_showImportant != None);
    //ui->showImportantMerged->setChecked(m_showImportant == ShowMerged);
    ui->showImportantSeparately->setChecked(m_showImportant == ShowSeparately);
}

void EmailNotifier::configAccepted()
{
    KConfigGroup cg = config();

    // Save the selection, expansion and scrollstate of our treeview
    // saving on the stack, according to apidocs
    Akonadi::ETMViewStateSaver viewState(this);
    viewState.setView(ui->collectionsTreeView);
    viewState.setSelectionModel(m_checkSelection);
    viewState.saveState(cg);

    // Display of important emails
    ImportantDisplay d = None;
    if (!ui->showImportant->isChecked()) {
        kDebug() << "None...";
        d = None;
    } else if (ui->showImportantSeparately->isChecked()) {
        kDebug() << "Separately...";
        d = ShowSeparately;
    } else {
        kDebug() << "Merged..." << ShowMerged;
        d = ShowMerged;
    }
    // Check if the show important settings affect the unread list, so we can reset it if needed
    bool unreadListChanged = ((m_showImportant == ShowMerged) == (d == None || d == ShowSeparately));
    kDebug() << "show important has changed for the unread list?" << unreadListChanged;
    bool addImportantTab = ((m_showImportant == None || m_showImportant == ShowMerged) && (d == ShowSeparately));
    bool removeImportantTab = ((m_showImportant == ShowSeparately) && (d == None || d == ShowMerged));
    kDebug() << "********** Add Tab:" << addImportantTab << " Remove Tab:" << removeImportantTab;

    if (d != m_showImportant) {
        m_showImportant = d;
        m_dialog->unreadEmailList()->setShowImportant(d == ShowMerged);
        kDebug() << "showing important messages in unread list" << (m_showImportant == ShowMerged);

        if (m_showImportant == ShowSeparately) {
            // TODO: create important tab
        }
    }
    cg.writeEntry("showImportant", (int)(d));

    // HTML display
    if (ui->allowHtml->isChecked() != m_allowHtml) {
        m_allowHtml = !m_allowHtml;
        kDebug() << "HTML Allowed changed";
    }
    cg.writeEntry("allowHtml", m_allowHtml);

    // Collections
    m_newCollectionIds.clear();

    foreach (QModelIndex itemindex, m_checkSelection->selectedIndexes()) {
        // We're only interested in the collection ID
        quint64 _id = itemindex.data(EntityTreeModel::CollectionIdRole).value<quint64>();
        m_newCollectionIds << _id;

        // .. remove me
        Akonadi::Collection col = itemindex.data(
                                    EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        kDebug() << "collection selected:" << col.name() << col.resource();

    }

    if (addImportantTab) {
        QList<quint64> l;
        m_dialog->addImportantTab(l);
    } else if (removeImportantTab) {
        m_dialog->removeImportantTab();
    }

    qSort(m_collectionIds.begin(), m_collectionIds.end());
    qSort(m_newCollectionIds.begin(), m_newCollectionIds.end());

    kDebug() << "Collection IDs changed from " << m_collectionIds << "to" << m_newCollectionIds;
    // write collections to config, update important tab
    if ((m_collectionIds != m_newCollectionIds)) {
        cg.writeEntry("unreadCollectionIds", m_collectionIds);
        kDebug() << "new config value:" << m_collectionIds;
        if (m_dialog->importantEmailList()) {
            m_dialog->importantEmailList()->clear();
            // Then we add those collections that weren't previously in the list
            kDebug() << "Cleared IMPORTANT emails, now adding back collections" << m_collectionIds;
            foreach(const quint64 _id, m_newCollectionIds) {
                m_dialog->importantEmailList()->addCollection(_id);
            }
        }
    }
    // update unread tab
    kDebug() << "repopulate new tab?" << (m_collectionIds != m_newCollectionIds) << unreadListChanged;
    if ((m_collectionIds != m_newCollectionIds) || unreadListChanged) {
        m_dialog->unreadEmailList()->clear();
        // Then we add those collections that weren't previously in the list
        kDebug() << "Cleared NEW emails, now adding back collections" << m_newCollectionIds;
        foreach(const quint64 _id, m_newCollectionIds) {
            kDebug() << "adding new collection" << _id;
            m_dialog->unreadEmailList()->addCollection(_id);
        }
    }

    m_collectionIds = m_newCollectionIds;
    emit configNeedsSaving();
}

void EmailNotifier::configChanged()
{
    KConfigGroup cg = config();
    m_allowHtml = config().readEntry("allowHtml", false);

    // FIXME: we want to compare old and new and if necessary add the new collections and remove the old one
    // this should probably be shared through configChanged()
    m_collectionIds = cg.readEntry("unreadCollectionIds", QList<quint64>());

    m_showImportant = (ImportantDisplay)(cg.readEntry("showImportant", 0));
    if (m_dialog) {
        m_dialog->unreadEmailList()->setShowImportant(m_showImportant == ShowMerged);
        if (m_showImportant == ShowSeparately) {
            m_dialog->addImportantTab(m_collectionIds);
        } else {
            m_dialog->removeImportantTab(); // no-op if the tab isn't there to begin with
        }
    }
}

void EmailNotifier::statusChanged(int emailsCount, const QString& statusText)
{
    //kDebug() << "----------------- Status changed: " << emailsCount << statusText;
    QString icon = "mail-mark-unread";
    if (emailsCount) {
        icon = "mail-mark-unread-new";
        if (!statusText.isEmpty()) {
            updateToolTip(statusText, icon);
            m_dialog->setTitle(statusText);
        } else {
            const QString _t = i18np("%1 New Message", "%1 New Messages", emailsCount);
            updateToolTip(_t, icon);
            m_dialog->setTitle(_t);
        }
        setStatus(Plasma::ActiveStatus);
    } else {
        QString _t = i18nc("tooltip: no new emails", "No New Messages");
        updateToolTip(_t, icon);
        setStatus(Plasma::PassiveStatus);
        m_dialog->setTitle(_t);
    }
    setPopupIcon(icon);
}

void EmailNotifier::updateToolTip(const QString& statusText, const QString& icon)
{
    m_toolTip = Plasma::ToolTipContent(statusText, QString(), KIcon(icon));
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}


K_EXPORT_PLASMA_APPLET(emailnotifier, EmailNotifier)

#include "emailnotifier.moc"
