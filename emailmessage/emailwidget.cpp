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

EmailWidget::EmailWidget(EmailMessage* emailmessage, QGraphicsWidget *parent)
    : Frame(parent),
      //id(61771), // more plain example
      //id(97160), // sample html email
      id(97162), // sample email + patch attached
      //id(0), // what it's supposed to be

      // Are we already fetching the data?
      m_fetching(false),

      // Flags
      m_isNew(false),
      m_isUnread(false),
      m_isImportant(false),
      m_isActionItem(false),

      // Display options
      m_allowHtml(true), // no html emails for now
      m_showSmilies(true),

      // UI Items
      m_toLabel(0),
      m_fromLabel(0),
      m_ccLabel(0),
      m_bccLabel(0),
      m_dateLabel(0),
      m_bodyView(0),
      m_abstractLabel(0)
{
    m_monitor = 0;
    m_emailMessage = emailmessage;
    m_expanded = false;
    buildDialog();
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
            h = 16;
            break;
        case Tiny:
            h = qMax(22, (int)m_subjectLabel->minimumHeight());
            break;
        case Small:
            return 32;
        case Medium:
            return 84;
        case Large:
            return 200;
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
    m_toLabel->hide();
    if (m_fromLabel) {
        m_fromLabel->hide();
    }
    m_bodyView->hide();
    updateSize(widgetHeight(Icon));
}

void EmailWidget::setTiny()
{
    if (!m_expanded && m_appletSize == Tiny) {
        kDebug() << "size is tiny already";
        return;
    }
    m_appletSize = Tiny;
    m_expandIcon->setIcon("arrow-down-double");
    m_subjectLabel->show();
    m_subjectLabel->setMinimumWidth(140);

    m_toLabel->hide();
    if (m_fromLabel && m_dateLabel) {
        m_fromLabel->hide();
        m_dateLabel->hide();
    }
    m_bodyView->hide();
    int h = widgetHeight(m_appletSize);
    updateSize(h);
    resizeIcon(h);
    //kDebug() << m_layout->minimumSize();
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

    m_subjectLabel->show();
    if (m_fromLabel) {
        m_fromLabel->show();
    }
    m_subjectLabel->setMinimumWidth(140);
    m_toLabel->show();
    if (m_fromLabel && m_dateLabel) {
        m_fromLabel->hide();
        m_dateLabel->hide();
    }
    m_bodyView->hide();
    //m_layout->setRowFixedHeight(2,0);
    resizeIcon(22);

    m_expandIcon->setIcon("arrow-down-double");

    m_appletSize = Small;
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
    m_expandIcon->setIcon("arrow-down-double");
    m_toLabel->show();
    if (m_fromLabel && m_dateLabel) {
        m_fromLabel->show();
        m_dateLabel->show();
    }
    m_bodyView->hide();
    kDebug() << "Medium ...";
    resizeIcon(32);
    m_appletSize = Medium;
    int h = widgetHeight(m_appletSize);
    updateSize(h);
    kDebug() << m_layout->geometry().size() << preferredSize() << minimumSize();
}

void EmailWidget::setLarge(bool expanded)
{
    if (m_expanded && m_appletSize == Large ) {
        return;
    }
    m_expandIcon->setIcon("arrow-up-double");
    m_toLabel->show();
    m_subjectLabel->show();
    if (m_fromLabel && m_dateLabel) {
        m_fromLabel->show();
        m_dateLabel->show();
    }

    m_bodyView->show();

    //m_layout->setRowMinimumHeight(3, 80);
    m_bodyView->setMinimumHeight(80);

    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    kDebug() << "Large ...";
    resizeIcon(32);
    setMinimumHeight(m_layout->minimumSize().height());
    setMinimumWidth(m_layout->minimumSize().width());
    if (!expanded) {
        m_appletSize = Large;
    }
    if (!m_fetching) {
        fetchPayload();
    }
    updateSize(widgetHeight(Large));
}

void EmailWidget::buildDialog()
{
    updateColors();
    //m_frame = new Plasma::Frame(this);

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_layout->setColumnFixedWidth(0, 40); // This should be dynamic, we'll also want to decrease the second column if the first one expands
    m_layout->setColumnPreferredWidth(1, 140);
    m_layout->setColumnFixedWidth(2, 22);
    m_layout->setHorizontalSpacing(4);

    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("view-pim-mail");
    m_icon->setAcceptHoverEvents(false);
    resizeIcon(32);
    //m_layout->addItem(m_icon, 0, 0, 1, 1, Qt::AlignTop);
    m_layout->addItem(m_icon, 0, 0, 3, 1, Qt::AlignTop);


    m_subjectLabel = new Plasma::Label(this);
    m_subjectLabel->nativeWidget()->setWordWrap(false);
    m_subjectLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //m_subjectLabel->setMaximumWidth(240);
    m_subjectLabel->setMinimumWidth(100);
    m_layout->addItem(m_subjectLabel, 0, 1, 1, 1, Qt::AlignTop);
    setSubject("Re: sell me a beer, mon");


    m_toLabel = new Plasma::Label(this);
    m_toLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_toLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_toLabel->nativeWidget()->setWordWrap(false);
    setTo(QStringList("Bob Marley <marley@kde.org>"));

    m_layout->addItem(m_toLabel, 1, 1, 1, 2);

    // From and date
    m_fromLabel = new Plasma::Label(this);
    m_fromLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fromLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    setFrom(i18n("Unknown Sender"));
    m_layout->addItem(m_fromLabel, 2, 0, 1, 3);

    m_dateLabel = new Plasma::Label(this);
    m_dateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_dateLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    setDate(QDateTime());
    m_layout->addItem(m_dateLabel, 3, 0, 1, 3);

    // The Body
    m_bodyView = new Plasma::WebView(this);
    //m_bodyView->setMinimumSize(20, 40);

    m_bodyView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setRawBody("<b>Fetching data ...</b>");

    m_layout->addItem(m_bodyView, 4, 0, 1, 3);

    m_expandIcon = new Plasma::IconWidget(this);
    m_expandIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_expandIcon->setIcon("arrow-down-double");
    m_expandIcon->setMinimumSize(16, 16);
    m_expandIcon->setMaximumSize(16, 16);
    connect(m_expandIcon, SIGNAL(clicked()), this, SLOT(toggleBody()));
    connect(m_icon, SIGNAL(clicked()), this, SLOT(toggleMeta()));
    m_layout->addItem(m_expandIcon, 0, 2, 1, 1, Qt::AlignRight | Qt::AlignTop);

    setLayout(m_layout);

    updateColors();
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
        kDebug() << "expanding";
        expand();
    } else {
        kDebug() << "collapsing";
        collapse();
    }
    kDebug() << preferredSize() << minimumSize();
}

void EmailWidget::toggleMeta()
{
    if (m_appletSize == Medium) {
        kDebug() << "tinying";
        setTiny();
    } else {
        kDebug() << "smalling";
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

    m_stylesheet = QString("\
                body { color: %1; }\
                a:visited   { color: %1; }\
                a:link   { color: %2; }\
                a:visited   { color: %3; }\
                a:hover { text-decoration: none; } \
    ").arg(text.name()).arg(link.name()).arg(linkvisited.name());

    if (m_bodyView) {
        m_bodyView->page()->setPalette(p);
        m_subjectLabel->setStyleSheet(m_stylesheet);
        m_toLabel->setStyleSheet(m_stylesheet);
        m_fromLabel->setStyleSheet(m_stylesheet);
    }
}

void EmailWidget::setAllowHtml(bool allow)
{
    m_allowHtml = allow;
}

void EmailWidget::setSubject(const QString& subject)
{
    if (m_subjectLabel && !subject.isEmpty()) {
        m_subjectLabel->setText(subject);
    }
    m_subject = subject;
}

void EmailWidget::setTo(const QStringList& toList)
{
    //kDebug() << "Setting recipient" << toList;
    if (m_toLabel && toList.count()) {
        QString html = KPIMUtils::LinkLocator::convertToHtml(toList.join(", "));
        m_toLabel->setText(i18n("<style>%1</style><b>To:</b> %2", m_stylesheet, html));
    }
}

void EmailWidget::setRawBody(const QString& body)
{
    if (m_bodyView && !body.isEmpty()) {
        QString html = QString("<style type=\"text/css\">%1</style><body>%2</body>").arg(m_stylesheet, body);
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
    if ( part ) {
        plainPart = part->decodedText( true, true );
    }
    part = m_msg->mainBodyPart( "text/html" );
    if ( part ) {
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

    //KConfigGroup config = KGlobal::config()->group( "General" );
    if ( m_showSmilies ) {
        flags |= LinkLocator::ReplaceSmileys;
    }
    flags |= LinkLocator::HighlightText;

    if ( m_allowHtml ) {
        if ( htmlPart.trimmed().isEmpty() ) {
            m_body = LinkLocator::convertToHtml( plainPart, flags );
        } else {
            m_body = htmlPart;
        }
        // when replying prefer plain
        !plainPart.isEmpty() ? raw = plainPart : raw = htmlPart;
    } else {
        // show plain
        if ( plainPart.trimmed().isEmpty() ) {
            m_body = LinkLocator::convertToHtml( htmlPart, flags );
        } else {
            m_body = LinkLocator::convertToHtml( plainPart, flags );
        }
        // when replying prefer plain
        !plainPart.isEmpty() ? raw = plainPart : raw = htmlPart;
    }

    // make the quotation colors.
    //m_body = Global::highlightText( m_body );
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
    m_bodyView->setMinimumHeight(80);
    updateSize(widgetHeight(Large));
    Akonadi::Item::List items = static_cast<Akonadi::ItemFetchJob*>( job )->items();

    kDebug() << "Fetched" << items.count() << " email Items.";
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
    if (item->hasPayload<MessagePtr>()) {
        MessagePtr msg = item->payload<MessagePtr>();

        id = item->id(); // This shouldn't change ... right?
        setSubject(msg->subject()->asUnicodeString());
        setFrom(msg->from()->asUnicodeString());
        setDate(msg->date()->dateTime().dateTime());
        setTo(QStringList(msg->to()->asUnicodeString()));
        setCc(QStringList(msg->cc()->asUnicodeString()));
        setBcc(QStringList(msg->bcc()->asUnicodeString()));
        setBody(msg);
    } else {
        setSubject(i18n("Couldn't fetch email payload"));
    }
}

void EmailWidget::setAbstract(const QString& abstract)
{
    if (m_abstractLabel && abstract.isEmpty()) {
        QString html = KPIMUtils::LinkLocator::convertToHtml(abstract);
        m_abstractLabel->setText(html);
    }
    m_abstract = abstract;
}

void EmailWidget::setDate(const QDateTime& date)
{
    if (m_dateLabel) {
        if (date.isValid()) {
            m_date = date;
            QString d = KGlobal::locale()->formatDateTime( m_date, KLocale::FancyShortDate );
            m_dateLabel->setText(i18n("<b>Date:</b> %1", date.toString()));
        } else {
            m_dateLabel->setText(i18n("<b>Date:</b> unknown"));
        }
    }
}

void EmailWidget::setFrom(const QString& from)
{
    if (m_fromLabel && !from.isEmpty()) {
        QString html = KPIMUtils::LinkLocator::convertToHtml(from);
        m_fromLabel->setText(i18n("<style>%1</style><b>From:</b> %2", m_stylesheet, html));
    }
    m_from = from;
}

void EmailWidget::setCc(const QStringList& ccList)
{
    if (m_ccLabel && ccList.count()) {
        QString html = KPIMUtils::LinkLocator::convertToHtml(ccList.join(", "));
        m_ccLabel->setText(i18n("<style>%1</style><b>Cc:</b> %2", m_stylesheet, html));
    }
    m_cc = ccList;
}

void EmailWidget::setBcc(const QStringList& bccList)
{
    if (m_bccLabel && bccList.count()) {
        QString html = KPIMUtils::LinkLocator::convertToHtml(bccList.join(", "));
        m_bccLabel->setText(i18n("<style>%1</style><b>Bcc:</b> %2", m_stylesheet, html));
    }
    m_bcc = bccList;
}

void EmailWidget::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        kDebug() << "clicked";
        m_startPos = event->pos();
    }
}

void EmailWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - m_startPos).toPoint().manhattanLength();
        kDebug() << "moved + pressed";
        if (distance >= QApplication::startDragDistance()) {
            startDrag();
        }
    }
}

void EmailWidget::startDrag()
{
    kDebug() << "Starting drag!";
    QMimeData* mimeData = new QMimeData();
    mimeData->setText("This is some random email");

    // This is a bit random, but we need a QWidget for the constructor
    // not sure if this works at all ... :/
    QDrag* drag = new QDrag(m_subjectLabel->nativeWidget());
    drag->setMimeData(mimeData);
    drag->setPixmap(m_icon->icon().pixmap(64, 64));
}



#include "emailwidget.moc"
