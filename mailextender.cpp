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
#include <Plasma/ScrollWidget>

// Akonadi
#include <akonadi/collectionstatistics.h>
#include <akonadi/collectionstatisticsjob.h>

//own
#include "mailextender.h"
#include "lionmail.h"

MailExtender::MailExtender(LionMail * applet, const QString collectionId, Plasma::Extender *ext)
    : Plasma::ExtenderItem(ext),
      m_id(0),
      m_info(0),
      m_iconName(QString("mail-folder-inbox")),
      m_unreadCount(0),
      m_count(0),
      m_applet(0),
      engine(0),
      m_messageLayout(0),
      m_layout(0),
      m_icon(0),
      m_widget(0),
      m_label(0),
      m_infoLabel(0),
      m_emailScroll(0),
      m_emailsWidget(0),
      m_zoomIn(0),
      m_zoomOut(0),
      m_refresh(0),
      m_monitor(0)
{
    kDebug() << "ctr" << m_id;
    m_applet = applet;
    setTitle("Lion Mail");
    setName("Lion Mail ExenderItem");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setDescription("Some description");
    m_maxEmails = 12;
    m_emailSize = EmailWidget::Small;
    engine = m_applet->dataEngine("akonadi");
    (void)graphicsWidget();
    setCollection(collectionId);
}

void MailExtender::load()
{
    kDebug() << "loading ...";
    setInfo(i18n("Loading emails..."));
    setIcon(m_iconName);
}

void MailExtender::setCollection(const QString id)
{
    if (id.isEmpty() || id == m_id) {
        kDebug() << "id empty or m_id" << id << m_id; //return; // FIXME: investigate
    }
    kDebug() << "Setting collection from to " << m_id << id;
    if (m_id != 0) {
        disconnectCollection(m_id);
    }
    m_id = id;
    connectCollection(m_id);
    connect(engine, SIGNAL(sourceAdded(const QString&)), this, SLOT(sourceAdded(const QString&)));
    connect(engine, SIGNAL(sourceRemoved(const QString&)), this, SLOT(sourceRemoved(const QString&)));
    updateStatistics();

    // Keep collection stats updated
    m_monitor = new Akonadi::Monitor(this);
    m_monitor->fetchCollectionStatistics(true);
    m_monitor->setCollectionMonitored(Akonadi::Collection(m_id.split("-")[1].toInt()));
    connect(m_monitor, SIGNAL(collectionStatisticsChanged(Akonadi::Collection::Id, const Akonadi::CollectionStatistics &)),
            this, SLOT(setStatistics(Akonadi::Collection::Id, const Akonadi::CollectionStatistics &)));
    setDescription(m_applet->collectionName(m_id));
    setInfo(i18n("Loading emails..."));
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

void MailExtender::updateStatistics()
{
    int _id = m_id.split("-")[1].toInt();
    kDebug() << "Retrieving statistics for" << m_id << _id;
    Akonadi::Collection collection = Akonadi::Collection(_id);
    Akonadi::CollectionStatisticsJob *job = new Akonadi::CollectionStatisticsJob(collection);
    connect( job, SIGNAL(result(KJob*)), SLOT(statisticsFetchDone(KJob*)) );
}

void MailExtender::statisticsFetchDone(KJob* job)
{
    Akonadi::CollectionStatisticsJob* statsjob = static_cast<Akonadi::CollectionStatisticsJob*>(job);
    if ( job->exec() ) {
        // We're passing 0 as id since we don't have anything better handy,
        // and since it's thrown away anyway, only to match the signature of the slot
        setStatistics(0, statsjob->statistics());
    }
    if (job->error()) {
        kDebug() << "statistics job failed" << job->errorString();
    }
}

void MailExtender::setStatistics(Akonadi::Collection::Id id, const Akonadi::CollectionStatistics &statistics)
{
    Q_UNUSED( id );
    m_unreadCount = statistics.unreadCount();
    m_count = statistics.count();
    kDebug() << "stats are updated: total (unread):" << m_count << "(" << m_unreadCount << ")";
    setInfo();
    setDescription();

}

void MailExtender::disconnectCollection(QString cid)
{
    kDebug() << "disconnectSource" << cid;
    engine->disconnectSource(cid, this); // pass collection ID as string
}

void MailExtender::sourceAdded(const QString & source)
{
    // TODO: make sure we only connect to new emails
    if (source.startsWith("EmailCollection-")) {
        kDebug() << "------------- New source, connecting to:" << source;
        engine->connectSource(source, this);

    }
    if (source.startsWith("Email-")) {
        if (emails.count() < m_maxEmails) {
            engine->connectSource(source, this);
        }
    }
}

void MailExtender::sourceRemoved(const QString & source)
{
    // TODO: make sure we only connect to new emails
    //kDebug() << "------------- New:" << source;
    if (source.startsWith("EmailCollection-") || source.startsWith("Email-")) {
        //kDebug() << "source removed, disconnecting from" << source;
        engine->disconnectSource(source, this);
    }
}

int MailExtender::unreadEmails()
{
    return m_unreadCount;
}

void MailExtender::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (!source.startsWith("Email-")) {
        // Apparently the signal ends up in here, while it should in these
        // cases happen in the applet, just pass it on for now
        m_applet->dataUpdated(source, data);
        return;
    }

    if (emails.count() >= m_maxEmails) {
        return;
    }

    EmailWidget* email = 0;

    if (!emails.keys().contains(source)) {
        //kDebug() << "New email:" << source;
        email = new EmailWidget(this);
        if (!m_showUnreadOnly || data["Flag-New"].toBool()) {
            addEmail(email);
            emails[source] = email;
        }
    } else {
        // update the data on an existing one
        email = emails[source];
    }
    setInfo();

    if (email == 0) {
        kDebug() << "didn't load email" << source;
        return;
    }
    //kDebug() << "Subject:" << data["Subject"].toString();
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

    //setEmailSize(EmailWidget::Small);
    update();
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

    // top label and actions
    m_actionsLayout = new QGraphicsLinearLayout(m_layout);
    m_actionsLayout->setOrientation(Qt::Horizontal);
    m_actionsLayout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    m_label = new Plasma::Label(m_widget);
    setDescription(m_id);
    m_actionsLayout->addItem(m_label);

    int s = KIconLoader::SizeSmall;

    m_zoomOut = new Plasma::IconWidget(m_widget);
    m_zoomOut->setIcon("zoom-out");
    m_zoomOut->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_zoomOut->setMaximumHeight(s);
    m_zoomOut->setMaximumWidth(s);
    m_zoomOut->setMinimumWidth(s);
    m_actionsLayout->addItem(m_zoomOut);

    m_zoomIn = new Plasma::IconWidget(m_widget);
    m_zoomIn->setIcon("zoom-in");
    m_zoomIn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_zoomIn->setMaximumHeight(s);
    m_zoomIn->setMaximumWidth(s);
    m_zoomIn->setMinimumWidth(s);
    m_actionsLayout->addItem(m_zoomIn);

    m_refresh = new Plasma::IconWidget(m_widget);
    m_refresh->setIcon("view-refresh");
    m_refresh->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_refresh->setMaximumHeight(s);
    m_refresh->setMaximumWidth(s);
    m_refresh->setMinimumWidth(s);
    m_actionsLayout->addItem(m_refresh);

    connect(m_zoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(m_zoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(m_refresh, SIGNAL(clicked()), this, SLOT(refresh()));


    m_layout->addItem(m_actionsLayout, 0, 1);

    // smaller label
    m_infoLabel = new Plasma::Label(m_widget);
    m_infoLabel->setText(i18n("No emails found"));
    m_infoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_infoLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_infoLabel, 1, 1);


    m_emailScroll = new Plasma::ScrollWidget(m_widget);
    m_emailsWidget = new QGraphicsWidget(m_emailScroll);

    m_messageLayout = new QGraphicsLinearLayout(m_emailsWidget);
    m_messageLayout->setSpacing(8);
    m_messageLayout->setOrientation(Qt::Vertical);
    m_messageLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_emailsWidget->setLayout(m_messageLayout);
    m_emailScroll->setWidget(m_emailsWidget);
    m_layout->addItem(m_emailScroll, 3, 0, 1, 2);
    //m_messageLayout->addStretch(-1);
    //m_messageLayout->addStretch(-1);
    m_widget->setLayout(m_layout);

    setWidget(m_widget);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
    //resize(200, 400);
    return m_widget;
}

void MailExtender::refresh()
{
    kDebug() << "Refresh";
}

void MailExtender::zoomIn()
{
    kDebug() << "zooming in";
    switch (m_emailSize) {
        case EmailWidget::Icon:
        case EmailWidget::Tiny:
            kDebug() << "setting small";
            setEmailSize(EmailWidget::Small);
            break;
        case EmailWidget::Small:
            kDebug() << "setting medium";
            setEmailSize(EmailWidget::Medium);
            break;
        case EmailWidget::Medium:
            kDebug() << "setting large";
            setEmailSize(EmailWidget::Large);
            break;
        case EmailWidget::Large:
            kDebug() << "Already large";
            break;
        default:
            kDebug() << "hmz.";
            break;
    }
    //emit configNeedsSaving();
}

void MailExtender::zoomOut()
{
    kDebug() << "zooming out";
    switch (m_emailSize) {
        case EmailWidget::Icon:
        case EmailWidget::Tiny:
            kDebug() << "Already tiny";
            break;
        case EmailWidget::Small:
            setEmailSize(EmailWidget::Tiny);
            break;
        case EmailWidget::Medium:
            setEmailSize(EmailWidget::Small);
            break;
        case EmailWidget::Large:
            setEmailSize(EmailWidget::Medium);
            break;
        default:
            break;
    }
    //emit configNeedsSaving();
}

void MailExtender::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_zoomOut->show();
    m_zoomIn->show();
    m_refresh->show();
}

void MailExtender::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    m_zoomIn->hide();
    m_zoomOut->hide();
    m_refresh->hide();
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
    email->setParentItem(m_emailScroll);
    email->setSmall();
    email->setAllowHtml(m_applet->allowHtml());

    m_messageLayout->addItem(email);
    m_messageLayout->updateGeometry();
    updateGeometry();
}

void MailExtender::setEmailSize(int appletsize)
{
    kDebug() << "------------ Set applet size" << appletsize;

    if (appletsize == EmailWidget::Large) {
        m_zoomIn->setEnabled(false);
    } else {
        m_zoomIn->setEnabled(true);
    }
    if (appletsize == EmailWidget::Tiny || appletsize == EmailWidget::Icon) {
        m_zoomOut->setEnabled(false);
    } else {
        m_zoomOut->setEnabled(true);
    }
    m_emailSize = appletsize;
    foreach (EmailWidget* e, emails.values()) {
        e->setSize(appletsize);
    }
    m_messageLayout->updateGeometry();
    m_layout->updateGeometry();
}


void MailExtender::setDescription()
{
    QString desc;
    QString cname = m_applet->collectionName(m_id);
    if (m_unreadCount) {
        desc = i18nc("title of the extenderitem", "%1 (%2 unread)", cname, m_unreadCount);
    } else {
        desc = cname;
    }
    setTitle(desc);
    setDescription(cname);
}

void MailExtender::setDescription(const QString& desc)
{
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

void MailExtender::setInfo()
{
    if (m_unreadCount + m_count == 0) {
        m_info = i18nc("Label for stats in the collection", "Loading statistics...");
    } else {

        if (!m_unreadCount) {
            m_info = i18np("1 email", "%1 emails", m_count);
        } else {
            m_info = i18np("1 email (%2 new)", "%1 emails (%2 new)", m_count, m_unreadCount);
        }
    }
    if (m_infoLabel) {
        m_infoLabel->setText(m_info);
    }
}

void MailExtender::updateColors()
{
    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    //m_widget->setPalette(p);
}

#include "mailextender.moc"
