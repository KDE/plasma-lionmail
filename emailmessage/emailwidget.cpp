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
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>
#include <QWebPage>
#include <QLabel>
#include <QApplication>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KGlobalSettings>

// Akonadi
#include <Akonadi/Item>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Collection>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/Monitor>

#include <kpimutils/email.h>
#include <kpimutils/linklocator.h>
#include <kmime/kmime_dateformatter.h>

// Plasma
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/WebView>

// own
#include "emailmessage.h"
#include "emailwidget.h"

using namespace Plasma;

EmailWidget::EmailWidget(QGraphicsWidget *parent)
    : Frame(parent),
      //id(61771), // more plain example
      //id(97160), // sample html email
      //id(168593), // sample email + image + pdf attached
      id(97881), // sample email + image + pdf attached
      //id(0), // what it's supposed to be

      //id(83964),

      m_applet(0),
      // Are we already fetching the data?
      m_fetching(false),
      m_item(0),

      // Flags
      m_isNew(false),
      m_isUnread(false),
      m_isImportant(false),
      m_isTask(false),

      // Display options
      m_allowHtml(true), // no html emails for now
      m_showSmilies(true),

      // UI Items
      m_header(0),
      m_dateLabel(0),
      m_newIcon(0),
      m_importantIcon(0),
      m_taskIcon(0),
      m_bodyView(0)
{
    m_monitor = 0;
    m_expanded = false;
    buildDialog();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(updateColors()));
}

EmailWidget::~EmailWidget()
{
}

int EmailWidget::widgetHeight(int size)
{
    //kDebug() << "minwidth:" << m_layout->minimumWidth() << m_toLabel->minimumSize().width();
    int h;
    switch (size) {
        case Icon:
            h = KIconLoader::SizeSmall;
            break;
        case Tiny:
            h = qMax((int)KIconLoader::SizeSmall, (int)m_subjectLabel->minimumHeight());
            break;
        case Small:
            return KIconLoader::SizeMedium; // 32
        case Medium:
            return (int)(KIconLoader::SizeHuge * 1.5); // 96
        case Large:
            return (int)(KIconLoader::SizeEnormous * 1.5); // 192
    }
    return h;
}

void EmailWidget::setIcon()
{
    if (m_appletSize == Icon) {
        return;
    }
    if (m_expanded) {
        return;
    }
    kDebug() << "Icon ...";
    m_appletSize = Icon;
    m_subjectLabel->hide();
    m_subjectLabel->setMinimumWidth(0);

    if (m_header) {
        m_header->hide();
        m_dateLabel->hide();
        m_expandIcon->hide();
        refreshFlags(false);
    }
    m_bodyView->hide();
    updateSize(widgetHeight(Icon));
}

void EmailWidget::setTiny()
{
    if (!m_expanded && m_appletSize == Tiny) {
        return;
    }
    m_appletSize = Tiny;

    m_subjectLabel->show();
    m_subjectLabel->setMinimumWidth(140);
    m_expandIcon->show();
    m_expandIcon->setIcon("arrow-down-double");

    m_dateLabel->hide();
    m_header->hide();
    m_bodyView->hide();

    int h = widgetHeight(m_appletSize);
    updateSize(h);
    resizeIcon(h);
    refreshFlags(false);
}

void EmailWidget::updateSize(int h)
{
    setMinimumHeight(h+6);
    setPreferredHeight(h);
    updateGeometry();
    m_layout->updateGeometry();
}

void EmailWidget::setSmall()
{
    if (m_appletSize == Small) {
        return;
    }
    if (m_expanded) {
        return;
    }
    kDebug() << "Small ...";
    m_appletSize = Small;

    m_subjectLabel->show();
    m_subjectLabel->setMinimumWidth(140);
    m_expandIcon->show();
    m_expandIcon->setIcon("arrow-down-double");
    m_dateLabel->show();

    m_header->hide();
    m_bodyView->hide();
    resizeIcon(22);


    refreshFlags(true);
    int h = widgetHeight(m_appletSize);
    updateSize(h);
}

void EmailWidget::setMedium()
{
    if (m_appletSize == Medium) {
        return;
    }
    if (m_expanded) {
        return;
    }

    m_appletSize = Medium;
    m_expandIcon->setIcon("arrow-down-double");
    m_expandIcon->show();
    m_subjectLabel->show();
    m_header->show();
    m_dateLabel->show();

    m_bodyView->hide();
    kDebug() << "Medium ...";

    refreshFlags(true);
    resizeIcon(32);
    int h = widgetHeight(m_appletSize);
    updateSize(h);
    kDebug() << m_layout->geometry().size() << preferredSize() << minimumSize();
}

void EmailWidget::setLarge(bool expanded)
{
    if (m_expanded && m_appletSize == Large ) {
        return;
    }
    if (m_appletSize == Large) {
        return;
    }
    if (!expanded) {
        m_appletSize = Large;
    }

    m_expandIcon->setIcon("arrow-up-double");
    m_subjectLabel->show();
    m_expandIcon->show();
    m_header->show();
    m_dateLabel->show();
    m_bodyView->show();

    //m_layout->setRowMinimumHeight(3, 80);
    m_bodyView->setMinimumHeight(80);

    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    kDebug() << "Large ...";
    refreshFlags(true);
    resizeIcon(32);
    setMinimumHeight(m_layout->minimumSize().height());
    setMinimumWidth(m_layout->minimumSize().width());

    if (!m_fetching) {
        fetchPayload();
    }
    updateSize(widgetHeight(Large));
}

void EmailWidget::buildDialog()
{
    updateColors();

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_layout->setColumnFixedWidth(0, 40); // This could probably be a bit more dynamic should be dynamic
    m_layout->setColumnPreferredWidth(1, 180);
    m_layout->setColumnFixedWidth(2, 22);
    m_layout->setHorizontalSpacing(4);

    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("mail-mark-read");
    m_icon->setAcceptHoverEvents(false);
    resizeIcon(32);
    m_layout->addItem(m_icon, 0, 0, 2, 1, Qt::AlignTop);

    m_subjectLabel = new Plasma::Label(this);
    m_subjectLabel->nativeWidget()->setWordWrap(false);
    m_subjectLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_subjectLabel->setMinimumWidth(100);
    m_layout->addItem(m_subjectLabel, 0, 1, 1, 1, Qt::AlignTop);
    setSubject("Re: sell me a beer, mon");

    m_dateLabel = new Plasma::Label(this);
    m_dateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_dateLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    setDate(QDateTime());

    m_actionsLayout = new QGraphicsLinearLayout(m_layout);
    m_actionsLayout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_actionsLayout->addItem(m_dateLabel);

    int s = KIconLoader::SizeSmall;
    m_newIcon = new IconWidget(this);
    m_newIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_newIcon->setIcon("mail-mark-unread");
    m_newIcon->setMinimumWidth(s);
    m_newIcon->setMaximumHeight(s);
    m_newIcon->setMaximumWidth(s);
    connect(m_newIcon, SIGNAL(clicked()), this, SLOT(flagNewClicked()));

    m_importantIcon = new IconWidget(this);
    m_importantIcon->setIcon("mail-mark-important");
    m_importantIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_importantIcon->setMinimumWidth(s);
    m_importantIcon->setMaximumHeight(s);
    m_importantIcon->setMaximumWidth(s);
    connect(m_importantIcon, SIGNAL(clicked()), this, SLOT(flagImportantClicked()));

    m_taskIcon = new IconWidget(this);
    m_taskIcon->setIcon("mail-mark-task");
    m_taskIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_taskIcon->setMinimumWidth(s);
    m_taskIcon->setMaximumHeight(s);
    m_taskIcon->setMaximumWidth(s);
    connect(m_taskIcon, SIGNAL(clicked()), this, SLOT(flagTaskClicked()));

    m_actionsLayout->addItem(m_newIcon);
    m_actionsLayout->addItem(m_importantIcon);
    m_actionsLayout->addItem(m_taskIcon);

    m_layout->addItem(m_actionsLayout, 1, 1, 1, 2, Qt::AlignTop | Qt::AlignRight);

    // From and date
    m_header = new Plasma::WebView(this);
    m_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_header->setPreferredHeight(48);
    //m_header->page()->setFont(KGlobalSettings::smallestReadableFont());
    setFrom(i18n("Unknown Sender"));
    m_layout->addItem(m_header, 2, 0, 1, 3, Qt::AlignTop);

    // The Body
    m_bodyView = new Plasma::WebView(this);
    //m_bodyView->setMinimumSize(20, 40);

    m_bodyView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setRawBody("<b>Fetching data ...</b>");

    m_layout->addItem(m_bodyView, 3, 0, 1, 3);

    m_expandIcon = new Plasma::IconWidget(this);
    m_expandIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_expandIcon->setIcon("arrow-down-double");
    m_expandIcon->setMinimumSize(16, 16);
    m_expandIcon->setMaximumSize(16, 16);
    connect(m_expandIcon, SIGNAL(clicked()), this, SLOT(toggleBody()));
    connect(m_icon, SIGNAL(clicked()), this, SLOT(toggleMeta()));
    m_layout->addItem(m_expandIcon, 0, 2, 1, 1, Qt::AlignRight | Qt::AlignTop);

    setLayout(m_layout);

    // Refresh flags
    setNew(m_isNew);
    setTask(m_isTask);
    setImportant(m_isImportant);

    updateColors();
}

void EmailWidget::refreshFlags()
{
    refreshFlags(m_flagsShown);
}

void EmailWidget::flagNewClicked()
{
    kDebug() << "New clicked";
    m_isNew = !m_isNew;


    // sync to Akonadi
    if (!m_item) {
        m_item = new Akonadi::Item(id);
    }
    if (m_isNew) {
        m_item->clearFlag("\\Seen");
    } else {
        m_item->setFlag("\\Seen");
    }
    refreshFlags();
}

void EmailWidget::flagImportantClicked()
{
    // TODO: sync to Akonadi
    kDebug() << "Important clicked";
    m_isImportant = !m_isImportant;

    // sync to Akonadi
    if (!m_item) {
        m_item = new Akonadi::Item(id);
    }
    if (m_isImportant) {
        m_item->setFlag("important");
    } else {
        m_item->clearFlag("important");
    }
    refreshFlags();
}

void EmailWidget::flagTaskClicked()
{
    // TODO: sync to Akonadi
    kDebug() << "Task clicked";
    m_isTask = !m_isTask;
    // sync to Akonadi
    if (!m_item) {
        m_item = new Akonadi::Item(id);
    }
    if (m_isTask) {
        m_item->setFlag("\\Task");
    } else {
        m_item->clearFlag("\\Task");
    }
    refreshFlags();
}

void EmailWidget::refreshFlags(bool show)
{
    m_flagsShown = show;

    if (!m_icon) {
        return;
    }

    // Update larger icon with most important flag
    if (m_isImportant) {
        m_icon->setIcon("mail-mark-important");
    } else if (m_isNew) {
        m_icon->setIcon("mail-mark-unread");
    } else {
        m_icon->setIcon("mail-mark-read");
    }

    if (show) {
        m_newIcon->show();
        m_newIcon->setPressed(m_isNew);
        // As iconwidget doesn't provide a good visual clue for
        // the activated flag, FIXME: remove
        m_newIcon->setDrawBackground(m_isNew);
        setSubject(m_subject); // for updating font weight on the subject line
    } else {
        m_newIcon->hide();
    }

    if (show) {
        m_importantIcon->show();
        m_importantIcon->setPressed(m_isImportant);
        m_importantIcon->setDrawBackground(m_isImportant);
    } else {
        m_importantIcon->hide();
    }

    if (show) {
        m_taskIcon->show();
        m_taskIcon->setPressed(m_isTask);
        m_taskIcon->setDrawBackground(m_isTask);
    } else {
        m_taskIcon->hide();
    }
}

void EmailWidget::resizeIcon(int iconsize)
{
    m_layout->setColumnFixedWidth(0, iconsize);
    m_layout->setColumnPreferredWidth(1, 180-iconsize);
    m_icon->resize(iconsize, iconsize);
    m_icon->setMinimumHeight(iconsize);
    m_icon->setMaximumHeight(iconsize);
    m_icon->setPreferredHeight(iconsize);
    m_layout->updateGeometry();
}

void EmailWidget::toggleBody()
{
    kDebug() << preferredSize() << minimumSize();
    if (!m_expanded) {
        expand();
    } else {
        collapse();
    }
    //kDebug() << preferredSize() << minimumSize();
}

void EmailWidget::toggleMeta()
{
    if (m_appletSize == Medium) {
        setTiny();
    } else {
        setMedium();
    }
}

void EmailWidget::collapse()
{
    kDebug() << "hiding body";
    m_expandIcon->setIcon("arrow-down-double");
    setTiny();
    m_expanded = false;
}


void EmailWidget::expand()
{
    kDebug() << "showing body";
    m_expanded = true;
    setLarge(true);
}

void EmailWidget::updateColors()
{
    QPalette p = palette();

    // Set background to transparent and use the theme to provide contrast with the text
    p.setColor(QPalette::Base, Qt::transparent); // new in Qt 4.5
    p.setColor(QPalette::Window, Qt::transparent); // For Qt 4.4, remove when we depend on 4.5

    QColor text = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QColor link = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    link.setAlphaF(qreal(.8));
    QColor linkvisited = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    linkvisited.setAlphaF(qreal(.6));

    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Link, link);
    p.setColor(QPalette::LinkVisited, linkvisited);

    setPalette(p);

    qreal fontsize = KGlobalSettings::smallestReadableFont().pointSize();
    m_stylesheet = QString("\
                body { \
                    color: %1; \
                    font-size: %4pt; \
                    /* background-color: orange; border-style: dotted; border-width: thin; */ \
                    width: 100%, \
                    margin-left: 0px; \
                    margin-top: 0px; \
                    margin-right: 0px; \
                    margin-bottom: 0px; \
                    padding: 0px; \
                } \
                .header { \
                    overflow: hidden; \
                    font-size: 1em; \
                    opacity: .9; \
                    cell-padding: 0; \
                    cell-spacing: 0; \
                    /* background-color: green; border-style: dotted; border-width: thin; */\
                } \
                .headerlabel { \
                    font-size: 1em; \
                    font-weight: bold;\
                    opacity: 1; \
                    text-align: right; \
                    vertical-align: top; \
                } \
\
                a:visited   { color: %1; }\
                a:link   { color: %2; opacity: .8; }\
                a:visited   { color: %3; opacity: .6; }\
                a:hover { text-decoration: none; opacity: .4; } \
\
    ").arg(text.name()).arg(link.name()).arg(linkvisited.name()).arg(fontsize);

    if (m_bodyView) {
        m_header->page()->setPalette(p);
        m_bodyView->page()->setPalette(p);
        m_subjectLabel->setStyleSheet(m_stylesheet);
    }
    updateHeader();
}

void EmailWidget::updateHeader()
{
    if (!m_header) {
        return;
    }
    QString table = QString("<table class=\"header\">");
    int r = 0;
    if (!m_from.isEmpty()) {
        r++;
        table += QString("<tr><td class=\"headerlabel\">%1</td><td>%2</td></tr>").arg(
                            i18n("From:"),
                            KPIMUtils::LinkLocator::convertToHtml(m_from));
    }
    if (!m_to.isEmpty()) {
        r++;
        table += QString("<tr><td class=\"headerlabel\" valign=\"top\" >%1</td><td>%2</td></tr>").arg(
                            i18n("To:"),
                            KPIMUtils::LinkLocator::convertToHtml(m_to.join(", ")));
    }
    if (!m_cc.isEmpty()) {
        kDebug() << "CC:" << m_cc;
        r++;
        table += QString("<tr><td class=\"headerlabel\">%1</td><td>%2</td></tr>").arg(
                            i18n("CC:"),
                            KPIMUtils::LinkLocator::convertToHtml(m_cc.join(", ")));
    }
    if (!m_bcc.isEmpty()) {
        kDebug() << "BCC:" << m_bcc;
        r++;
        table += QString("<tr><td class=\"headerlabel\">%1</td><td>%2</td></tr>").arg(
                            i18n("BCC:"),
                            KPIMUtils::LinkLocator::convertToHtml(m_bcc.join(", ")));
    }

    QFontMetrics fm(KGlobalSettings::smallestReadableFont());
    qreal header_height = (fm.height() * 1.3) * r;
    m_header->setMinimumHeight(header_height);
    m_header->setPreferredHeight(header_height);
    m_header->setMaximumHeight(header_height);

    table += "</table>";
    m_header->setHtml(QString("<style>%1 %2</style>%3").arg("body { overflow: hidden; }", m_stylesheet, table));
    updateGeometry();
    // TODO: attachments
    //kDebug() << QString("<style>%1</style>%2").arg(m_stylesheet, table);

}

void EmailWidget::setUrl(KUrl url)
{
    //kDebug() << url.url();
    m_url = url;
}

void EmailWidget::setAllowHtml(bool allow)
{
    m_allowHtml = allow;
}

void EmailWidget::setSubject(const QString& subject)
{
    if (m_subjectLabel && !subject.isEmpty()) {
        if (m_isNew) {
            m_subjectLabel->setText(QString("<b>%1</b>").arg(subject));
        } else {
            m_subjectLabel->setText(subject);
        }
    }
    m_subject = subject;
}

void EmailWidget::setTo(const QStringList& toList)
{
    m_to = toList;
    updateHeader();
}

void EmailWidget::setRawBody(const QString& body)
{
    if (m_bodyView) {
        QString html = i18n("<h3>Empty body loaded.</h3>");
        html = i18n("<style type=\"text/css\">%1</style><body><h3>Empty body loaded.</h3></body>", m_stylesheet);
        if (!body.isEmpty()) {
            html = QString("<style type=\"text/css\">%1</style><body>%2</body>").arg(m_stylesheet, body);
        }
        //kDebug() << html;
        m_bodyView->setHtml(html);
    }
    m_body = body;
}

void EmailWidget::setBody(MessagePtr msg)
{
    // Borrowed from Tom Albers' mailody/src/messagedata.cpp
    m_msg = msg;
    // Retrieve the plain and html part.
    QString plainPart, htmlPart;
    KMime::Content* part = m_msg->mainBodyPart( "text/plain" );
    if (part) {
        plainPart = part->decodedText( true, true );
    }
    part = m_msg->mainBodyPart( "text/html" );
    if (part) {
        htmlPart = part->decodedText( true, true );
    }
    //kDebug() << plainPart;
    //kDebug() << htmlPart;
/*
    // replace all cid: entries with their filenames.
    QHash<KUrl,QString>::Iterator ita;
    for ( ita = m_attachments.begin(); ita != m_attachments.end(); ++ita ) {
        kDebug() << "cid:"+ita.key().path() << " -> " << ita.value() << endl;
        htmlPart.replace( "cid:"+ita.value(), "file://" + ita.key().path() );
    }
*/
    // Assign m_body
    using KPIMUtils::LinkLocator;
    int flags = LinkLocator::PreserveSpaces | LinkLocator::HighlightText;
    QString raw;

    if (m_showSmilies) {
        flags |= LinkLocator::ReplaceSmileys;
    }
    flags |= LinkLocator::HighlightText;

    if (m_allowHtml) {
        if (htmlPart.trimmed().isEmpty()) {
            m_body = LinkLocator::convertToHtml(plainPart, flags);
        } else {
            m_body = htmlPart;
        }
        // when replying prefer plain
        !plainPart.isEmpty() ? raw = plainPart : raw = htmlPart;
    } else {
        // show plain
        if ( plainPart.trimmed().isEmpty() ) {
            m_body = LinkLocator::convertToHtml(htmlPart, flags);
        } else {
            m_body = LinkLocator::convertToHtml(plainPart, flags);
        }
        // when replying prefer plain
        !plainPart.isEmpty() ? raw = plainPart : raw = htmlPart;
    }

    setRawBody(m_body);
}

void EmailWidget::fetchPayload()
{
    if (id == 0) {
        kDebug() << "id is 0";
        return;
    }
    m_bodyView->setMinimumHeight(30);
    updateSize(widgetHeight(Large)-50);
    m_fetching = true;
    kDebug() << "Fetching payload for " << id;
    Akonadi::ItemFetchJob* fetchJob = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
    fetchJob->fetchScope().fetchFullPayload();
    connect( fetchJob, SIGNAL(result(KJob*)), SLOT(fetchDone(KJob*)) );
}

void EmailWidget::fetchDone(KJob* job)
{

    if ( job->error() ) {
        kDebug() << "Error fetching item" << id << ": " << job->errorString();
        setRawBody(i18n("<h3>Fetching email body %1 failed: <p /></h3><pre>%2</pre>", id, job->errorString()));
        return;
    }
    //m_bodyView->setMinimumHeight(80);
    //updateSize(widgetHeight(Large));
    Akonadi::Item::List items = static_cast<Akonadi::ItemFetchJob*>(job)->items();

    kDebug() << "Fetched" << items.count() << "email Items." << id;
    if (items.count() == 0) {
        setRawBody(i18n("<h3>Could not find Email with id: <p /></h3><pre>%1</pre>", id));
    }
    foreach( const Akonadi::Item &item, items ) {

        // Start monitoring this item
        if (m_monitor ==0) {
            m_monitor = new Akonadi::Monitor(this);
        }
        m_monitor->setItemMonitored(item);
        connect( m_monitor, SIGNAL(itemChanged(Akonadi::Item, QSet<QByteArray>)),
            this, SLOT(itemChanged(Akonadi::Item)) );

        itemChanged(&item);
    }
}

void EmailWidget::itemChanged(const Akonadi::Item* item)
{
    delete m_item;
    m_item = new Akonadi::Item(item->id());
    if (item->hasPayload<MessagePtr>()) {
        MessagePtr msg = item->payload<MessagePtr>();

        id = item->id(); // This shouldn't change ... right?
        setSubject(msg->subject()->asUnicodeString());
        setFrom(msg->from()->asUnicodeString());
        setDate(msg->date()->dateTime().dateTime());
        setTo(QStringList(msg->to()->asUnicodeString()));
        setCc(QStringList(msg->cc()->asUnicodeString()));
        setBcc(QStringList(msg->bcc()->asUnicodeString()));
        updateHeader();
        setBody(msg);
    } else {
        setSubject(i18n("Couldn't fetch email payload"));
    }
}

void EmailWidget::setAbstract(const QString& abstract)
{
    if (abstract.isEmpty()) {
        QString html = KPIMUtils::LinkLocator::convertToHtml(abstract);
        // TODO: something sensible ...
    }
    m_abstract = abstract;
}

void EmailWidget::setDate(const QDateTime& date)
{
    if (m_dateLabel) {
        if (date.isValid() && !date.isNull()) {
            m_date = date;
            QString d = KGlobal::locale()->formatDateTime( m_date, KLocale::FancyLongDate );
            m_dateLabel->setText(i18n("<b>Date:</b> %1", d));
        } else {
            m_dateLabel->setText(i18n("<b>Date:</b> unknown"));
        }
    }
}

void EmailWidget::setFrom(const QString& from)
{
    m_from = from;
    updateHeader();
}

void EmailWidget::setCc(const QStringList& ccList)
{
    m_cc = ccList;
    updateHeader();
}

void EmailWidget::setBcc(const QStringList& bccList)
{
    m_bcc = bccList;
    updateHeader();
}

void EmailWidget::setNew(bool isnew)
{
    m_isNew = isnew;
    //if (m_applet) {
    //    kDebug() << "Setting popupicon";
        //m_applet->setPopupIcon("mail-mark-unread");
    //}
    refreshFlags();
}

void EmailWidget::setImportant(bool important)
{
    m_isImportant = important;
    refreshFlags();
}

void EmailWidget::setTask(bool task)
{
    m_isTask = task;
    refreshFlags();
}

void EmailWidget::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        //kDebug() << "clicked";
        m_startPos = event->pos();
    }
}

void EmailWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - m_startPos).toPoint().manhattanLength();
        if (distance >= QApplication::startDragDistance()) {
            startDrag();
        }
    }
}

void EmailWidget::startDrag()
{
    //kDebug() << "Starting drag!";
    QMimeData* mimeData = new QMimeData();
    QString url = m_url.url();

    mimeData->setData(QString("message/rfc822"), url.toUtf8());
    QList<QUrl> urls;
    urls << m_url;
    //mimeData->setUrls(urls);
    mimeData->setText(QString("Email with URL: %1<br /><br />%2").arg(m_url.url()).arg(m_body));

    // This is a bit random, but we need a QWidget for the constructor
    QDrag* drag = new QDrag(m_subjectLabel->nativeWidget());
    drag->setMimeData(mimeData);
    drag->setPixmap(m_icon->icon().pixmap(64, 64));
    if (drag->start(Qt::CopyAction | Qt::MoveAction)) {
        kDebug() << "dragging starting";
    }
}


#include "emailwidget.moc"
