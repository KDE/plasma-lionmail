/***************************************************************************
 *   Copyright 2007 by Thomas Moenicke <thomas.moenicke@kdemail.net>       *
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
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

#include "lionmail.h"

#include <QPainter>
#include <QLayout>

#include <KConfigDialog>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/Extender>
#include <Plasma/DataEngine>

#include "mailextender.h"
#include "emailmessage/emailmessage.h"

LionMail::LionMail(QObject *parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args)
{
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/akonadi");
    m_theme->setContainsMultipleImages(false);
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_subjectList[0] = QString("Hello CampKDE, hallo Jamaica!"); // ;-)
    setBackgroundHints(StandardBackground);
    setPassivePopup(true);

    m_fontFrom = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    m_fontSubject = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    setPopupIcon("akonadi");
    m_allowHtml = false;
}

LionMail::~LionMail()
{
}

QString LionMail::collectionName(const QString &id)
{
    if (m_collections.count() && m_collections.keys().contains(id)) {
        return m_collections[id].toString();
    }
    return i18n("Collection %1", id);
}

bool LionMail::allowHtml()
{
    return m_allowHtml;
}
void LionMail::init()
{
    KConfigGroup cg = config();
    m_activeCollection = cg.readEntry("activeCollection", "");
    m_allowHtml = cg.readEntry("allowHtml", false);

    if (m_activeCollection.isEmpty()) {
        setConfigurationRequired(true, i18n("Please select an Email Folder"));
    }
    kDebug() << "Active Collection" << m_activeCollection;

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resize(300, 400);
    extender()->setEmptyExtenderMessage(i18n("empty..."));

    initMailExtender(m_activeCollection);

    updateToolTip("", 0);
    dataEngine("akonadi")->connectSource("EmailCollections", this);
    //dataEngine("akonadi")->connectSource("ContactCollections", this); // FIXME: remove, only for testing the contacts in the dataengine

}


void LionMail::createConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent);

    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("Collections"), Applet::icon());

    //m_collections = dataEngine("akonadi")->query("EmailCollections");
    foreach ( QString c, m_collections.keys() ) {
        ui.collectionCombo->addItem(m_collections[c].toString(), c);
    }
    ui.allowHtml->setChecked(m_allowHtml);

    ui.removeCollection->setEnabled(ui.collectionList->count() != 0);


    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui.addCollection, SIGNAL(clicked()), this, SLOT(addListItem()));
    connect(ui.removeCollection, SIGNAL(clicked()), this, SLOT(removeListItem()));
    //connect(ui.collectionList, SIGNAL(currentItemChanged(QListWidgetItem, QListWidgetItem)),
    //        this, SLOT(listItemChanged(QListWidgetItem, QListWidgetItem)));
    connect(ui.collectionList, SIGNAL(itemSelectionChanged()),
            this, SLOT(listItemChanged()));
}

void LionMail::configAccepted()
{
    KConfigGroup cg = config();
    QString cid = ui.collectionCombo->itemData(ui.collectionCombo->currentIndex()).toString();

    if (m_activeCollection != cid) {
        QString name = ui.collectionCombo->currentText();

        // FIXME: hardcoded to first extender, needs UI change to support more than one extender
        m_extenders[0]->setCollection(cid);


        m_activeCollection = cid;
        setConfigurationRequired(false);

        cg.writeEntry("activeCollection", m_activeCollection);
        kDebug() << "Active Collections changed:" << m_activeCollection;
        emit configNeedsSaving();
    }
    if (ui.allowHtml->isChecked() != m_allowHtml) {
        m_allowHtml = !m_allowHtml;
        cg.writeEntry("allowHtml", m_allowHtml);
    }
}

void LionMail::addListItem()
{
    QString name = ui.collectionCombo->currentText();
    QListWidgetItem* item = new QListWidgetItem(KIcon("mail-folder-inbox"), name);
    QString cid = ui.collectionCombo->itemData(ui.collectionCombo->currentIndex()).toString();
    item->setData(Qt::UserRole, cid);
    ui.collectionList->addItem(item);
    ui.collectionCombo->removeItem(ui.collectionCombo->currentIndex());
    ui.removeCollection->setEnabled(false);

}

void LionMail::removeListItem()
{
    int row = ui.collectionList->currentRow();
    if (row != -1) {
        ui.collectionList->takeItem(row);
        if (ui.collectionList->count() == 0) {
            ui.removeCollection->setEnabled(false);
        }
    }
/*
    kDebug() << "removing item:" << ui.collectionList->currentItem()->text();
    //ui.collectionList->removeItemWidget(ui.collectionList->currentItem());
    QListWidgetItem* take = ui.collectionList->takeItem(ui.collectionList->currentRow());

    ui.collectionCombo->addItem(ui.collectionList->currentItem()->text(), 
                                ui.collectionList->currentItem()->data(Qt::UserRole).toString());

    if (take) {
        ui.collectionCombo->addItem(ui.collectionList->currentItem()->text(), 
                                ui.collectionList->currentItem()->data(Qt::UserRole).toString());
    } else {
        ui.removeCollection->setEnabled(false);
    }
    delete take;
    */
}

void LionMail::listItemChanged()
{
    if (ui.collectionList->currentRow() != -1) {
        kDebug() << "item selection:" << ui.collectionList->currentItem()->text();
        ui.removeCollection->setEnabled(true);
        ui.labelEdit->setText(ui.collectionList->currentItem()->text());
    }
}

void LionMail::initMailExtender(const QString id)
{
    MailExtender* mailView = new MailExtender(this, id, extender());
    m_extenders << mailView;
}

void LionMail::updateToolTip(const QString query, const int matches)
{
    Q_UNUSED(query);
    Q_UNUSED(matches);
    m_toolTip = Plasma::ToolTipContent(i18nc("No search has been done yet", "Lion Mail"),
            i18nc("Tooltip sub text", "Click on the icon to monitor your emails"),
                    KIcon("akonadi").pixmap(IconSize(KIconLoader::Desktop))
                );
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}

void LionMail::popupEvent(bool show)
{
    kDebug() << "POPUP" << show;
    if (show) {
        Plasma::ToolTipManager::self()->setState(Plasma::ToolTipManager::Inhibited);
    } else {
        Plasma::ToolTipManager::self()->setState(Plasma::ToolTipManager::Activated);
    }
}

void LionMail::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    setBusy(false);
    //kDebug() << "source";
    if (source == "EmailCollections") {
        foreach (MailExtender* ext, m_extenders) {
            ext->setDescription(collectionName(ext->id()));
        }
        //kDebug() << "Akonadi Email Received collections" << source << data.keys();
        m_collections = data;
        //kDebug() << "Akonadi:" << m_collections.keys();
        return;
    }
    if (source == "ContactCollections") {
        kDebug() << "Akonadi contacts collections:" << data.keys() << data;
    }
    //m_extenders[0]->dataUpdated(source, data); // FIXME: put data into the right extender
    update();
}

void LionMail::newSource(const QString & source)
{
    //kDebug() << "------------- New:" << source;
    dataEngine("akonadi")->connectSource(source, this);
    // We could create MailExtenders here ...
}

K_EXPORT_PLASMA_APPLET(lionmail, LionMail)

#include "lionmail.moc"
