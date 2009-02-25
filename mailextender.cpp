/*
    Copyright 2008-2009 by Sebastian Kügler <sebas@kde.org>

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

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KGlobalSettings>

//plasma
#include <Plasma/Applet>
#include <Plasma/Dialog>
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Extender>
//own
#include "mailextender.h"
#include "lionmail.h"
#include "emailmessage/emailmessage.h"


MailExtender::MailExtender(LionMail * applet, const QString collectionId, Plasma::Extender *ext)
    : Plasma::ExtenderItem(ext),
      m_id(0),
      m_info(0),
      m_iconName(QString("mail-folder-inbox")),
      m_icon(0),
      m_widget(0),
      m_label(0)
{
    m_id = collectionId;
    kDebug() << "ctr" << m_id;
    m_applet = applet;
    setTitle("Lion Mail");
    setName("Lion Mail ExenderItem");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setDescription("Some description");
    m_maxEmails = 12;
    if (m_id.isEmpty()) {
        kDebug() << "Empty id:";
        return;
    }
}

void MailExtender::load()
{
    (void)graphicsWidget();
    engine = m_applet->dataEngine("akonadi");
    //connectCollection(m_id);
    setCollection(m_id);
    kDebug() << "loading ...";
    setIcon(m_iconName);
}

void MailExtender::setCollection(const QString id)
{
    if (id != 0 && id == m_id) {
        //return;
    }
    kDebug() << "Setting collection from to " << m_id << id;
    disconnectCollection(m_id);
    m_id = id;
    connectCollection(m_id);
    connect(engine, SIGNAL(sourceAdded(QString)), this, SLOT(newSource(QString)));
    setDescription(m_applet->collectionName(m_id));
}

MailExtender::~MailExtender()
{
}

QString MailExtender::id()
{
    return m_id;
}

void MailExtender::setName(const QString name)
{
    ExtenderItem::setName(name);
    m_collection = name;
}

void MailExtender::connectCollection(QString cid)
{
    if (cid.isEmpty()) {
        return;
    }
    kDebug() << "connectSource" << cid;
    engine->connectSource(cid, this); // pass collection ID as string
}

void MailExtender::disconnectCollection(QString cid)
{
    kDebug() << "disconnectSource" << cid;
    engine->disconnectSource(cid, this); // pass collection ID as string
}

void MailExtender::newSource(const QString & source)
{
    // TODO: make sure we only connect to new emails
    //kDebug() << "------------- New:" << source;
    engine->connectSource(source, this);
}


void MailExtender::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << source << emails.keys();
    //if (source == "EmailCollections" || source == "ContactCollections") {
    if (!source.startsWith("Email-")) {
        // Apparently the signal ends up in here, while it should in these
        // cases happen in the applet, just pass it on for now
        m_applet->dataUpdated(source, data);
        return;
    }

    if (emails.count() >= m_maxEmails) {
        //kDebug() << "Max emails reached";
        return;
    }

    EmailWidget* email = 0;
    
    if (!emails.keys().contains(source)) {
        //kDebug() << "new ...";
        kDebug() << "New email:" << source;
        //email = static_cast<EmailMessage*>(Plasma::Applet::load("emailmessage"));
        email = new EmailWidget(this);
        if (!m_showUnreadOnly || data["Flag-New"].toBool()) {
            addEmail(email);
            emails[source] = email;
            if (m_infoLabel) {
                m_infoLabel->setText(i18n("%1 emails", emails.count()));
            }
            //kDebug() << "::Yes, showing" << source << "New?" << data["Flag-New"].toBool() << data;
        } else {
            //kDebug() << "::Not showing" << source << "New?" << data["Flag-New"].toBool();
        }
    } else {    
        // update the data on an existing one
        email = emails[source];
    }

    kDebug() << "??????" << emails.keys() << source << data;
    if (email == 0) {
        kDebug() << "didn't load email" << source;
        return;
    }
    kDebug() << "Subject:" << data["Subject"].toString();
    // Only set email-specific properties here, layouttweaks and the like should go into MailExtender
    email->id = data["Id"].toLongLong();
    email->setUrl(QUrl(data["Url"].toString()));
    email->setSubject(data["Subject"].toString());
    email->setFrom(data["From"].toString());
    email->setTo(data["To"].toStringList());
    email->setDate(data["DateTime"].toDateTime());

    email->setNew(data["Flag-New"].toBool());
    email->setTask(data["Flag-Task"].toBool());
    email->setImportant(data["Flag-Important"].toBool());

    email->setCc(data["Cc"].toStringList());
    email->setBcc(data["Bcc"].toStringList());

    update();
    //setPreferredSize(m_layout->preferredSize());
}

QGraphicsWidget* MailExtender::graphicsWidget()
{
    /*
    +-----+---------------------+
    | m_i |  m_label            |
    | con |  m_infoLabel        |
    +-----+---------------------+
    | emailmessage ...          |
    +---------------------------+
    | emailmessage ...          |
    +---------------------------+
    | emailmessage ...          |
    +---------------------------+
    */

    int iconsize = 32;

    // Main widget and layout

    m_widget = new QGraphicsWidget(this);

    m_layout = new QGraphicsGridLayout(m_widget);
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->setColumnFixedWidth(0, iconsize);
    m_layout->setColumnMinimumWidth(1, 100);
    m_layout->setHorizontalSpacing(4);
    m_layout->setVerticalSpacing(4);

    // larger icon on the left
    m_icon = new Plasma::IconWidget(m_widget);
    //m_icon->setIcon(icon());
    m_icon->setIcon("mail-folder-inbox");
    m_icon->resize(iconsize, iconsize);
    m_icon->setMinimumHeight(iconsize);
    m_icon->setMaximumHeight(iconsize);
    m_icon->setAcceptHoverEvents(false);
    m_layout->addItem(m_icon, 0, 0, 2, 1);

    // top label
    m_label = new Plasma::Label(m_widget);
    setDescription(m_id);
    m_layout->addItem(m_label, 0, 1);

    // smaller label
    m_infoLabel = new Plasma::Label(m_widget);
    m_infoLabel->setText(i18n("No emails found"));
    m_infoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_infoLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_infoLabel, 1, 1);

    m_messageLayout = new QGraphicsLinearLayout(m_layout);
    m_messageLayout->setSpacing(8);
    m_messageLayout->setOrientation(Qt::Vertical);
    m_messageLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_layout->addItem(m_messageLayout, 3, 0, 1, 3);
    m_messageLayout->addStretch(-1);
    m_messageLayout->addStretch(-1);
    m_widget->setLayout(m_layout);

    setWidget(m_widget);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
    //resize(200, 400);
    return m_widget;
}

void MailExtender::setShowUnreadOnly(bool show)
{
    m_showUnreadOnly = show;
}

bool MailExtender::showUnreadOnly()
{
    return m_showUnreadOnly;
}

void MailExtender::setMaxEmails(int max)
{
    m_maxEmails = max;
}

int MailExtender::maxEmails()
{
    return m_maxEmails;
}

QString MailExtender::description()
{
    return m_description;
}

void MailExtender::addEmail(EmailWidget* email)
{
    email->setParent(this);
    email->setParentItem(m_widget);
    //email->setBackgroundHints(Plasma::Applet::NoBackground);
    //email->init();
    //email->setPopupIcon(QIcon());
    email->setTiny();
    email->setAllowHtml(m_applet->allowHtml());
    //email->updateConstraints(Plasma::StartupCompletedConstraint);

    m_messageLayout->addItem(email);
}

void MailExtender::setDescription(const QString& desc)
{
    setTitle(desc);
    if (m_label) {
        QString html = "<b>" + desc + "</b>";
        m_label->setText(html);
    }
    m_description = desc;
}

void MailExtender::setIcon(const QString& icon)
{
    if (m_icon) {
        m_icon->setIcon(icon);
    }
    m_iconName = icon;
    ExtenderItem::setIcon(icon);
}

QString MailExtender::icon()
{
    return m_iconName;
}

void MailExtender::setInfo(const QString& info)
{
    if (m_infoLabel) {
        m_infoLabel->setText(info);
    }
    m_info = info;
}

void MailExtender::updateColors()
{
    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    //m_widget->setPalette(p);
}

#include "mailextender.moc"
