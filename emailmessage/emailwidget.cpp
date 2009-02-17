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
#include <QWebPage>
#include <QLabel>
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

#include <kmime/kmime_message.h>

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<KMime::Message> MessagePtr;

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
    : QGraphicsWidget(parent),
      id(61771),
      m_fetching(false),
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
    switch (size) {
        case Icon:
            return 16;
        case Tiny:
            return 16;
        case Small:
            return 22;
        case Medium:
            return 48;
        case Large:
            return 256;
    }
    return 16;
}

void EmailWidget::setIcon()
{
    if (m_expanded) {
        return;
    }
    kDebug() << "Icon ...";
    setSmall();
    /*
    m_toLabel->hide();
    m_bodyView->hide();
    //setSmall(); // TODO
    */
}

void EmailWidget::setTiny()
{
    m_appletSize = Tiny;
    if (m_expanded) {
        return;
    }
    //m_expanded = false;
    m_expandIcon->setIcon("arrow-down-double");
    m_toLabel->hide();
    m_bodyView->hide();
    kDebug() << "Tiny ...";
    int h = widgetHeight(m_appletSize);
    updateSize(h);
    resizeIcon(h);
    kDebug() << m_layout->minimumSize();
}

void EmailWidget::updateSize(int h)
{
    setMinimumHeight(h);
    setPreferredHeight(h);
    updateGeometry();
    m_layout->updateGeometry();
}

void EmailWidget::setSmall()
{
    if (m_expanded) {
        return;
    }
    kDebug() << "Small ...";

    m_toLabel->show();
    m_bodyView->hide();
    m_layout->setRowFixedHeight(2,0);
    resizeIcon(22);

    m_expandIcon->setIcon("arrow-down-double");

    m_appletSize = Small;
    int h = widgetHeight(m_appletSize);
    updateSize(h);
}

void EmailWidget::setMedium()
{
    if (m_expanded) {
        return;
    }
    m_expandIcon->setIcon("arrow-down-double");
    m_toLabel->show();
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
    if (!m_fetching) {
        fetchPayload();
    } else {
        kDebug() << "not fetching payload";
    }
    kDebug() << "Before ......." << m_layout->geometry().size() << m_layout->minimumSize();
    m_expandIcon->setIcon("arrow-up-double");
    m_toLabel->show();
    m_bodyView->show();

    m_layout->setRowMinimumHeight(3, 120);
    m_bodyView->setMinimumHeight(120);

    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    kDebug() << "Large ...";
    resizeIcon(32);
    setMinimumHeight(m_layout->minimumSize().height());
    if (!expanded) {
        m_appletSize = Large;
    }
    updateSize(widgetHeight(Large));
    updateGeometry();
    kDebug() << "After ........." << m_layout->geometry().size() << m_layout->minimumSize();
}

void EmailWidget::buildDialog()
{

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

    m_bodyView = new Plasma::WebView(this);
    //m_bodyView->setMinimumSize(20, 40);
    //m_bodyView->page()->view()->setAutoFillBackground(false);

    m_bodyView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QString html("<h3>Hi everybody</h3>I hope you're all having a great time on Jamaica, my home country (which you might have noticed after yesterday's bob-marley-on-repeat-for-the-whole-night. I wish I could be with you, but it wasn't meant to be. I'll go out with my friends Kurt and Elvis tonight instead and wish you a happy CampKDE.<br /><br /><em>-- Bob</em>");
    setBody(html);

    m_layout->addItem(m_bodyView, 3, 0, 1, 3);

    m_expandIcon = new Plasma::IconWidget(this);
    m_expandIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_expandIcon->setIcon("arrow-down-double");
    m_expandIcon->setMinimumSize(16, 16);
    m_expandIcon->setMaximumSize(16, 16);
    connect(m_expandIcon, SIGNAL(clicked()), this, SLOT(toggleBody()));
    m_layout->addItem(m_expandIcon, 0, 2, 1, 1, Qt::AlignRight | Qt::AlignTop);

    setLayout(m_layout);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
    kDebug() << "EmailWidget built";
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

void EmailWidget::collapse()
{
    kDebug() << "hiding body";
    m_expandIcon->setIcon("arrow-down-double");
    m_expanded = false;
    setTiny();
}


void EmailWidget::expand()
{
    kDebug() << "showing body";
    m_expanded = true;
    setLarge(true);
    //m_bodyView->setMinimumSize(200, 200);
}

void EmailWidget::updateColors()
{

    QPalette p = palette();
    //p.setColor(QPalette::Window, Plasma::Thme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    p.setColor(QPalette::Window, Qt::transparent);

    // FIXME: setting foreground color apparently doesn't work?
    p.setColor(QPalette::Text, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    p.setColor(QPalette::WindowText, Qt::green);
    kDebug() << "Textcolor:" << Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);

    setPalette(p);
    m_bodyView->page()->setPalette(p);
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
    kDebug() << "Setting recipient" << toList;
    if (m_toLabel && toList.count()) {
        m_toLabel->setText(i18n("<b>To:</b> %1", toList.join(", ")));
    }
}

void EmailWidget::setBody(const QString& body)
{
    if (m_bodyView && !body.isEmpty()) {
        QString html = body;
        html.replace("\n", "<br />\n");
        html.replace("\r", "<br />\n");
        html.replace("\r\n", "<br />\n");
        html.replace("=20<br />", "<br />");
        kDebug() << html;

        QString c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name(); // FIXME: nasty color
        QString b = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor).name(); // FIXME: nasty color hack
        //b = "transparent";

        m_bodyView->setHtml(QString("<body style=\"color: %1; background-color: %2\">%3</body>").arg(c, b, html)); // FIXME: Urks. :(
    }
    m_body = body;
}

void EmailWidget::fetchPayload()
{
    if (id == 0) {
        kDebug() << "id is 0";
        return;
    }
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
        return;
    }
    Akonadi::Item::List items = static_cast<Akonadi::ItemFetchJob*>( job )->items();

    //Akonadi::Item::List items = job->items();
    kDebug() << "Fetched" << items.count() << " body Items.";
    foreach( const Akonadi::Item &item, items ) {

        // Start monitoring this item
        if (m_monitor ==0) {
            m_monitor = new Akonadi::Monitor(this);
        }
        m_monitor->setItemMonitored(item);
        connect( m_monitor, SIGNAL(itemChanged(Akonadi::Item, QSet<QByteArray>)),
            SLOT(itemChanged(Akonadi::Item)) );

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
        setDate(msg->date()->dateTime().date());
        setTo(QStringList(msg->to()->asUnicodeString()));
        setCc(QStringList(msg->cc()->asUnicodeString()));
        setBcc(QStringList(msg->bcc()->asUnicodeString()));
        setBody(msg->mainBodyPart()->body());
    } else {
        setSubject(i18n("Couldn't fetch email payload"));
    }
}

void setDate(KDateTime* date)
{
    // TODO
}

void EmailWidget::setAbstract(const QString& abstract)
{
    if (m_abstractLabel && abstract.isEmpty()) {
        m_abstractLabel->setText(abstract);
    }
    m_abstract = abstract;
}

void EmailWidget::setDate(const QDate& date)
{
    if (m_dateLabel && date.isValid()) {
        m_dateLabel->setText(date.toString());
    }
    m_date = date;
}

void EmailWidget::setFrom(const QString& from)
{
    if (m_fromLabel && !from.isEmpty()) {
        m_fromLabel->setText(from);
    }
    m_from = from;
}

void EmailWidget::setCc(const QStringList& ccList)
{
    if (m_ccLabel && ccList.count()) {
        m_ccLabel->setText(ccList.join(", "));
    }
    m_cc = ccList;
}

void EmailWidget::setBcc(const QStringList& bccList)
{
    if (m_bccLabel && bccList.count()) {
        m_bccLabel->setText(bccList.join(", "));
    }
    m_bcc = bccList;
}


#include "emailwidget.moc"
