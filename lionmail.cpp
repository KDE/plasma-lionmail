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


#include <KConfigDialog>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/Extender>
#include <Plasma/DataEngine>

#include "mailextender.h"
#include "emailmessage/emailmessage.h"

LionMail::LionMail(QObject *parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args),
    m_theme(0),
    ui(0)
{
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/akonadi");
    m_theme->setContainsMultipleImages(false);
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_subjectList[0] = QString("Hello CampKDE, hallo Jamaica!"); // ;-)
    setBackgroundHints(StandardBackground);
    setPassivePopup(true);
    setMaximumHeight(600);
    setMinimumWidth(300);
    resize(300, 400);

    m_fontFrom = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    m_fontSubject = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    setPopupIcon("akonadi");
    m_allowHtml = false;

    kDebug() << "LionmailArgs" << args;
    foreach (QVariant a, args) {
        kDebug() << args.at(0).toString() << a.toString();
        QString firstarg = a.toString();
        if (firstarg.startsWith("EmailCollection-")) {
            m_collections << firstarg;
            kDebug() << "Loading EmailCollection from commandline argument (" << firstarg << ").";
        } else {
            kWarning() << "argument has to be in the form \"EmailCollection-<id>\", but it doesn't (" << firstarg << ")";
        }
    }
}

LionMail::~LionMail()
{
    delete ui;
}

QString LionMail::collectionName(const QString &id)
{
    if (m_allCollections.count() && m_allCollections.keys().contains(id)) {
        return m_allCollections[id].toString();
    }
    return i18n("Collection %1", id);
}

bool LionMail::allowHtml()
{
    return m_allowHtml;
}

void LionMail::init()
{
    dataEngine("akonadi")->connectSource("EmailCollections", this);

    KConfigGroup cg = config();
    if (m_collections.isEmpty()) { // it's not set from the command line, read configuration file
        m_collections = cg.readEntry("collections", QStringList());
        kDebug() << "using m_collections from applet configuration" << m_collections;
    }

    m_allowHtml = cg.readEntry("allowHtml", false);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    extender()->setEmptyExtenderMessage(i18n("Nothing to display. Please set up Lion Mail."));

    kDebug() << "Loading the following collections:" << m_collections;
    foreach (const QString c, m_collections) {
        if (!c.isEmpty()) {
            initMailExtender(c);
            m_extenders[c]->load();
        }
    }
    updateToolTip("", 0);

    if (m_collections.isEmpty()) {
        setConfigurationRequired(true, i18n("Please select an Email Folder"));
        kDebug() << "config needed ...";
    }
}


void LionMail::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui = new Ui::lionmailConfig();
    ui->setupUi(widget);
    parent->addPage(widget, i18n("Collections"), Applet::icon());

    //m_allCollections = dataEngine("akonadi")->query("EmailCollections");
    ui->addCollection->setEnabled(false);
    ui->saveCollection->setIcon(KIcon("document-save"));
    ui->saveCollection->setEnabled(false);
    kDebug() << m_extenders.keys();
    if (m_allCollections.isEmpty()) {
        ui->collectionsStatus->setText(i18n("Loading collection list, please wait...."));
    } else {
        ui->collectionsStatus->setText("");
    }
    addConfigCollections();
    ui->allowHtml->setChecked(m_allowHtml);

    ui->removeCollection->setEnabled(ui->collectionList->count() != 0);

    ui->sizeCombo->addItem(i18n("Icon and subject"), EmailWidget::Tiny);
    ui->sizeCombo->addItem(i18n("Icon, subject and sender"), EmailWidget::Small);
    ui->sizeCombo->addItem(i18n("Icon, subject, sender and header"), EmailWidget::Medium);
    ui->sizeCombo->addItem(i18n("Full email"), EmailWidget::Large);


    connect(parent, SIGNAL(finished()), this, SLOT(configFinished()));
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui->addCollection, SIGNAL(clicked()), this, SLOT(addListItem()));
    connect(ui->removeCollection, SIGNAL(clicked()), this, SLOT(removeListItem()));
    connect(ui->saveCollection, SIGNAL(clicked()), this, SLOT(saveCurrentCollection()));
    //connect(ui->collectionList, SIGNAL(currentItemChanged(QListWidgetItem, QListWidgetItem)),
    //        this, SLOT(listItemChanged(QListWidgetItem, QListWidgetItem)));
    connect(ui->collectionList, SIGNAL(itemSelectionChanged()),
            this, SLOT(listItemChanged()));
}

void LionMail::addConfigCollections()
{
    if (!ui) {
        return;
    }

    if (m_allCollections.count()) {
        ui->collectionsStatus->setText("");
    }

    if (ui->collectionCombo) {
        foreach ( QString cid, m_allCollections.keys() ) {
            if (m_extenders.keys().contains(cid)) {
                // A currently configured extender / collection
                QString icon = m_extenders[cid]->icon();
                if (icon.isEmpty()) {
                    icon = "mail-folder-inbox";
                }
                QListWidgetItem* item = new QListWidgetItem(QIcon(icon), m_extenders[cid]->description());
                item->setIcon(QIcon(m_extenders[cid]->icon()));
                item->setData(Qt::UserRole, cid);
                ui->collectionList->addItem(item);
            } else {
                ui->collectionCombo->addItem(m_allCollections[cid].toString(), cid);
                ui->addCollection->setEnabled(true);
            }
        }

        if (ui->collectionList->count()) {
            ui->collectionList->setCurrentRow(0);
            listItemChanged();
        }
    }
}

void LionMail::saveCurrentCollection()
{
    // Retrieve current ID
    if (!ui->collectionList->currentItem()) {
        return;
    }
    QString collectionId = ui->collectionList->currentItem()->data(Qt::UserRole).toString();

    // Create new collection if necessary
    saveCollection(collectionId);

    // Write UI into extenderitem
    m_extenders[collectionId]->setDescription(ui->labelEdit->text());
    m_extenders[collectionId]->setIcon(ui->icon->icon());
    m_extenders[collectionId]->setMaxEmails(ui->maxEmails->value());
    m_extenders[collectionId]->setShowUnreadOnly(ui->showUnreadOnly->isChecked());
    m_extenders[collectionId]->setEmailSize(ui->sizeCombo->itemData(ui->sizeCombo->currentIndex()).toInt());

    // Update collection list item as well
    ui->collectionList->currentItem()->setIcon(QIcon(ui->icon->icon()));
    ui->collectionList->currentItem()->setText(ui->labelEdit->text());
}

void LionMail::saveCollection(const QString &collectionId)
{
    setConfigurationRequired(false);
    if (!m_extenders.keys().contains(collectionId)) {
        kDebug() << "New collection ... " << collectionId << collectionName(collectionId);
        initMailExtender(collectionId);
        m_extenders[collectionId]->load();
    }
    kDebug() << ui->labelEdit->text() << ui->showUnreadOnly->isChecked() << ui->maxEmails->value() << ui->icon->icon();
    config().writeEntry("activeCollection", collectionId);
    config().sync();
}

void LionMail::configFinished()
{
    delete ui;
    ui = 0;
}

void LionMail::configAccepted()
{
    KConfigGroup cg = config();

    if (ui->allowHtml->isChecked() != m_allowHtml) {
        m_allowHtml = !m_allowHtml;
        cg.writeEntry("allowHtml", m_allowHtml);
    }

    int c = ui->collectionList->count();
    if (ui->collectionList->count()) {
        // FIXME: save collections from listview to config
    }
    for (int i = 0; i < c; i++) {
        setConfigurationRequired(false);
        QString itemid = ui->collectionList->item(i)->data(Qt::UserRole).toString();
        kDebug() << "selected id:" << itemid << collectionName(itemid);
        saveCollection(itemid);
    }
}

void LionMail::addListItem()
{
    QString name = ui->collectionCombo->currentText();
    QListWidgetItem* item = new QListWidgetItem(KIcon("mail-folder-inbox"), name);
    QString cid = ui->collectionCombo->itemData(ui->collectionCombo->currentIndex()).toString();
    item->setData(Qt::UserRole, cid);
    ui->collectionList->addItem(item);
    ui->collectionList->setCurrentItem(item);
    ui->collectionCombo->removeItem(ui->collectionCombo->currentIndex());
    ui->removeCollection->setEnabled(true);
}

void LionMail::removeListItem()
{
    int row = ui->collectionList->currentRow();
    QListWidgetItem* take = 0;
    if (row != -1) {
        take = ui->collectionList->takeItem(row);
        if (ui->collectionList->count() == 0) {
            ui->removeCollection->setEnabled(false);
        }
    }
    delete take;
}

void LionMail::listItemChanged()
{
    if (ui->collectionList->currentRow() != -1) {
        kDebug() << "item selection:" << ui->collectionList->currentItem()->text();
        enableCollectionWidgets(true);

        QString itemid = ui->collectionList->currentItem()->data(Qt::UserRole).toString();
        if (m_extenders.keys().contains(itemid)) {
            ui->labelEdit->setText(m_extenders[itemid]->description());
            ui->maxEmails->setValue(m_extenders[itemid]->maxEmails());
            ui->icon->setIcon(m_extenders[itemid]->icon());
            ui->showUnreadOnly->setChecked(m_extenders[itemid]->showUnreadOnly());

        } else {
            // Default values for our collection
            ui->labelEdit->setText(collectionName(itemid));
            ui->maxEmails->setValue(8);
            ui->icon->setIcon("mail-folder-inbox");
            ui->showUnreadOnly->setChecked(false);
        }
    } else {
        ui->labelEdit->setText("");
        enableCollectionWidgets(false);
    }
}

void LionMail::enableCollectionWidgets(bool enable)
{
    ui->removeCollection->setEnabled(enable);
    ui->clabel1->setEnabled(enable);
    ui->clabel2->setEnabled(enable);
    ui->clabel3->setEnabled(enable);
    ui->clabel4->setEnabled(enable);
    ui->clabel5->setEnabled(enable);
    ui->sizeCombo->setEnabled(enable);
    ui->labelEdit->setEnabled(enable);
    ui->saveCollection->setEnabled(enable);
    ui->maxEmails->setEnabled(enable);
    ui->icon->setEnabled(enable);
    ui->showUnreadOnly->setEnabled(enable);
}

void LionMail::initMailExtender(const QString id)
{
    if (id.isEmpty()) {
        kDebug() << "Empty id :(";
        return;
    }
    if (m_extenders.keys().contains(id)) {
        kDebug() << "Extender already exists" << id;
        return;
    }
    kDebug() << "Initialising " << id;
    MailExtender* mailView = new MailExtender(this, id, extender());
    kDebug() << "Initialising " << id;
    m_extenders[id] = mailView;
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

void LionMail::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    setBusy(false);
    kDebug() << "source" << source;
    if (source == "EmailCollections") {
        m_allCollections = data;
        addConfigCollections();
        update();
        return;
    }
    if (source == "ContactCollections") {
        kDebug() << "Akonadi contacts collections:" << data.keys() << data;
    }
}

void LionMail::newSource(const QString & source)
{
    //dataEngine("akonadi")->connectSource(source, this);
}

K_EXPORT_PLASMA_APPLET(lionmail, LionMail)

#include "lionmail.moc"
