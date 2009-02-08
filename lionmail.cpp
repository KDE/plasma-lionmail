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

    m_subjectList[0] = QString("Hello CampKDE, hallo Jamaica!"); // ;-)
    setBackgroundHints(StandardBackground);

    m_fontFrom = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    m_fontSubject = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    setPopupIcon("akonadi");
    m_maxEmails = 6;
}

LionMail::~LionMail()
{
}

void LionMail::init()
{
    KConfigGroup cg = config();
    m_activeCollection = cg.readEntry("activeCollection", "");

    if (m_activeCollection.isEmpty()) {
        setConfigurationRequired(true, i18n("Please select an Email Folder"));
    }
    kDebug() << "Active Collection" << m_activeCollection;

    engine = dataEngine("akonadi");
    engine->connectAllSources(this);
    connectCollection(m_activeCollection);
    setBusy(true);
    connect(engine, SIGNAL(sourceAdded(QString)), SLOT(newSource(QString)));
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //resize(300, 400); // move to constraintsevent
    extender()->setEmptyExtenderMessage(i18n("empty..."));

    initMailExtender();

    updateToolTip("", 0);
    m_collections = dataEngine("akonadi")->query("EmailCollections");
}

void LionMail::connectCollection(QString cid)
{
    if (cid.isEmpty()) {
        return;
    }
    kDebug() << "connectSource" << m_activeCollection;
    engine->connectSource(cid, this); // pass collection ID as string
}

void LionMail::disconnectCollection(QString cid)
{
    kDebug() << "disconnectSource" << cid;
    engine->disconnectSource(cid, this); // pass collection ID as string
}

void LionMail::createConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent);

    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("Collections"), Applet::icon());

    foreach ( QString c, m_collections.keys() ) {
        ui.collectionCombo->addItem(m_collections[c].toString(), c);
    }
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void LionMail::configAccepted()
{
    KConfigGroup cg = config();
    QString cid = ui.collectionCombo->itemData(ui.collectionCombo->currentIndex()).toString();

    if (m_activeCollection != cid) {
        QString name = ui.collectionCombo->currentText();
        //engine = dataEngine("akonadi");
        m_extenders[0]->setTitle(name);

        disconnectCollection(m_activeCollection);

        m_activeCollection = cid;
        setConfigurationRequired(false);

        connectCollection(m_activeCollection);

        cg.writeEntry("activeCollection", m_activeCollection);
        kDebug() << "Active Collections changed:" << m_activeCollection;
        emit configNeedsSaving();
    }
}

void LionMail::initMailExtender()
{
    MailExtender* mailView = new MailExtender(this, extender());
    mailView->setName("foobar"); // also make sure we don't recreate this one ...
    mailView->setDescription("Private Emails"); // FIXME: sample text
    mailView->setInfo("2 unread");

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

void LionMail::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    setBusy(false);
    if (source == "Collections") {
        m_collections = data;
        return;
    }
    EmailMessage* email = 0;
    if (emails.count() < m_maxEmails && !emails.contains(source)) {
        kDebug() << "new ...";
        email = static_cast<EmailMessage*>(Plasma::Applet::load("emailmessage"));
        if (m_extenders.count()) {
            m_extenders[0]->addEmail(email); // FIXME: hardcoded, we need to find a way to select the right extender
            emails[source] = email;
        }
    }
    if (emails.contains(source)) {
        email = emails[source];
    }

    if (email == 0) {
        return;
    }
    // Only set email-specific properties here, layouttweaks and the like should go into MailExtender
    email->m_emailWidget->id = data["Id"].toLongLong();
    email->m_emailWidget->setSubject(data["Subject"].toString());
    email->m_emailWidget->setFrom(data["From"].toString());
    email->m_emailWidget->setTo(data["To"].toStringList());
    email->m_emailWidget->setCc(data["Cc"].toStringList());
    email->m_emailWidget->setBcc(data["Bcc"].toStringList());

    m_fromList[0] = data["From"].toString();
    m_subjectList[0] = data["Subject"].toString();

    update();
}

void LionMail::newSource(const QString & source)
{
    //kDebug() << "------------- New:" << source;
    engine->connectSource(source, this);
    // We could create MailExtenders here ...
}

K_EXPORT_PLASMA_APPLET(lionmail, LionMail)

#include "lionmail.moc"
