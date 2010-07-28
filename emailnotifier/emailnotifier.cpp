/***************************************************************************
 *   Copyright 2010 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "emailnotifier.h"

#include <QTreeView>


#include <KConfigDialog>
#include <kselectionproxymodel.h>


#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/Extender>
#include <Plasma/DataEngine>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/EntityTreeModel>

#include <Akonadi/ServerManager>
//#include <akonadi/entitytreemodel.h>
//#include <akonadi/changerecorder.h>
//#include <akonadi/itemfetchscope.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include "copied_classes/checkableitemproxymodel.h"

#include "../emailmessage/emailmessage.h"
#include "dialog.h"

using namespace Akonadi;

EmailNotifier::EmailNotifier(QObject *parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args),
    m_theme(0),
    ui(0),
    m_dialog(0),
    m_collectionId(0)
{
    setPopupIcon("mail-unread-new");
    setHasConfigurationInterface(true);
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPassivePopup(true);
    setStatus(Plasma::ActiveStatus);

    kDebug() << "LionmailArgs" << args;
    foreach (QVariant a, args) {
        kDebug() << args.at(0).toString() << a.toString();
        m_collectionId = args.at(0).toString().toInt();
        /*
        QString firstarg = a.toString();
        if (firstarg.startsWith("EmailCollection-")) {
            //m_collections << firstarg;
            kDebug() << "Loading EmailCollection from commandline argument (" << firstarg << ").";
        } else {
            kWarning() << "argument has to be in the form \"EmailCollection-<id>\", but it isn't (" << firstarg << ")";
        }
        */
    }
    if (!m_collectionId == 0) {
        kDebug() << "No valid collection ID given";
    }
}

EmailNotifier::~EmailNotifier()
{
    delete ui;
}

void EmailNotifier::configChanged()
{
    KConfigGroup cg = config();
    if (!m_collectionId) {
        m_collectionId = cg.readEntry("unreadCollectionId", 0);
        kDebug() << "using config" << m_collectionId;
    } else {
        cg.writeEntry("unreadCollectionId", m_collectionId);
        kDebug() << "writing config" << m_collectionId;
    }
    kDebug() << "Using collection ID" << m_collectionId;
    m_allowHtml = cg.readEntry("allowHtml", false);
}

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
    /*
    if (source == "ContactCollections") {
        kDebug() << "Akonadi contacts collections:" << data.keys() << data;
    }
    */
}

QGraphicsWidget* EmailNotifier::graphicsWidget()
{
    if (!m_dialog) {
        kDebug() << "============================================== NEW";
        m_dialog = new Dialog(m_collectionId, this);
        connect(m_dialog, SIGNAL(statusChanged(int, const QString&)), this, SLOT(statusChanged(int, const QString&)));
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

    updateToolTip(i18nc("tooltip on startup", "No new email"), 0);
    //dataEngine("akonadi")->connectSource("EmailCollections", this);
}


void EmailNotifier::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui = new Ui::emailnotifierConfig();
    ui->setupUi(widget);
    parent->addPage(widget, i18n("Collections"), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QTreeView *treeView = ui->collectionsTreeView;


    // Set a model that displays only folders containing emails onto the treeView
    ChangeRecorder *changeRecorder = new ChangeRecorder( this );
    changeRecorder->setMimeTypeMonitored("message/rfc822");
    changeRecorder->setCollectionMonitored( Collection::root() );
    changeRecorder->fetchCollection( true );
    //changeRecorder->setAllMonitored( true );
    changeRecorder->itemFetchScope().fetchFullPayload( true );
    changeRecorder->itemFetchScope().fetchAllAttributes( true );

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
}


void EmailNotifier::configAccepted()
{
    KConfigGroup cg = config();

    if (ui->allowHtml->isChecked() != m_allowHtml) {
        m_allowHtml = !m_allowHtml;
        cg.writeEntry("allowHtml", m_allowHtml);
    }
    kDebug() << "Looking at our treeview selection...";
    //QItemSelectionModel *QModelIndexList    selectedIndexes () const
    foreach (QModelIndex itemindex, m_checkSelection->selectedIndexes()) {
        //QModelIndex itemindex = m_checkSelection->index(r, c);
        Akonadi::Item item = itemindex.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();
        if (!item.isValid()) {
            kDebug() << "invalid item";
        }
        quint64 itemid = itemindex.data(EntityTreeModel::ItemIdRole).value<quint64>();

        quint64 _id = item.id();
        kDebug() << "Collection selected:" << _id << item.url() << itemid;
    }
}

void EmailNotifier::statusChanged(int emailsCount, const QString& statusText)
{
    QString icon = "mail-mark-unread";
    if (emailsCount) {
        icon = "mail-mark-unread-new";
        if (!statusText.isEmpty()) {
            updateToolTip(statusText, icon);
        } else {
            const QString _t = i18np("%1 new email", "%1 new emails", emailsCount);
            updateToolTip(_t, icon);
        }
        setStatus(Plasma::ActiveStatus);
    } else {
        updateToolTip(i18nc("tooltip: no new emails", "No new email"), icon);
        setStatus(Plasma::PassiveStatus);
    }
    setPopupIcon(icon);
}

void EmailNotifier::updateToolTip(const QString& statusText, const QString& icon)
{
    m_toolTip = Plasma::ToolTipContent(statusText,
            i18nc("Tooltip sub text", "Click on the icon to view your emails"),
                    KIcon(icon).pixmap(IconSize(KIconLoader::Desktop))
                );
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}


K_EXPORT_PLASMA_APPLET(emailnotifier, EmailNotifier)

#include "emailnotifier.moc"
