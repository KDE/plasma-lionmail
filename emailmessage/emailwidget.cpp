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
#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/collection.h>
#include <akonadi/itemfetchjob.h>

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
      id(0),
      m_fetching(false),
      m_toLabel(0),
      m_fromLabel(0),
      m_ccLabel(0),
      m_bccLabel(0),
      m_dateLabel(0),
      m_bodyView(0),
      m_abstractLabel(0)
{
    m_emailMessage = emailmessage;
    m_expanded = false;
    buildDialog();
}

EmailWidget::~EmailWidget()
{
}

void EmailWidget::setIcon()
{
    kDebug() << "Icon ...";
    /*
    m_toLabel->hide();
    m_bodyView->hide();
    //setSmall(); // TODO
    */
}

void EmailWidget::setTiny()
{
    m_expanded = false;
    m_expandIcon->setIcon("arrow-down-double");
    m_toLabel->hide();
    m_bodyView->hide();
    kDebug() << "Tiny ...";
    resizeIcon(16);
    //m_layout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_layout->setRowFixedHeight(1,0);
    m_appletSize = Tiny;
    m_layout->invalidate();
    m_layout->updateGeometry();
    kDebug() << m_layout->geometry().size() << preferredSize() << minimumSize();
    setPreferredHeight(minimumSize().height());
}

void EmailWidget::setSmall()
{
    m_toLabel->show();
    m_bodyView->hide();
    m_layout->setRowFixedHeight(2,0);
    kDebug() << "Small ...";
    resizeIcon(22);

    m_expanded = false;
    m_expandIcon->setIcon("arrow-down-double");

    m_layout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_appletSize = Small;
    m_layout->invalidate();
    m_layout->updateGeometry();
    kDebug() << m_layout->geometry().size();
    setPreferredHeight(minimumSize().height());
}

void EmailWidget::setMedium()
{
    m_expanded = false;
    m_expandIcon->setIcon("arrow-down-double");
    m_toLabel->show();
    m_bodyView->hide();
    kDebug() << "Medium ...";
    resizeIcon(32);
    m_layout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_appletSize = Medium;
    m_layout->invalidate();
    m_layout->updateGeometry();
    kDebug() << m_layout->geometry().size();
}

void EmailWidget::setLarge(bool expanded)
{
    kDebug() << "Before ......." << m_layout->geometry().size() << m_layout->minimumSize();
    m_layout->setRowMinimumHeight(3, 40);
    m_expanded = true;
    m_expandIcon->setIcon("arrow-up-double");
    m_toLabel->show();
    m_bodyView->setMinimumHeight(50);
    m_bodyView->show();
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    kDebug() << "Large ...";
    resizeIcon(48);
    if (!expanded) {
        m_appletSize = Large;
    }
    sizeChanged();
    m_layout->invalidate();
    m_layout->updateGeometry();
    kDebug() << "After ........." << m_layout->geometry().size() << m_layout->minimumSize();
}

void EmailWidget::buildDialog()
{

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->setColumnMinimumWidth(1, 140);
    m_layout->setColumnFixedWidth(2, 22);
    m_layout->setHorizontalSpacing(4);

    m_layout->setRowStretchFactor(0, 0);
    m_layout->setRowStretchFactor(1, 0);
    m_layout->setRowStretchFactor(3, 5); // body
    m_layout->setRowFixedHeight(0, 14); // FIXME: hardcoded, this height should be based on the label's height
    m_layout->setRowFixedHeight(1, 14); // FIXME: hardcoded

    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("view-pim-mail");
    m_icon->setAcceptHoverEvents(false);
    resizeIcon(32);
    m_layout->addItem(m_icon, 0, 0, 2, 1, Qt::AlignTop);

    m_subjectLabel = new Plasma::Label(this);
    m_subjectLabel->setText("<b>Re: sell me a beer, mon</b>");
    //m_subjectLabel->setMaximumHeight(iconsize/2);
    m_layout->addItem(m_subjectLabel, 0, 1, 1, 1, Qt::AlignTop);

    m_toLabel = new Plasma::Label(this);
    m_toLabel->setText("<b>Recipient:</b> Bob Marley <bmarley@kde.org>");
    //m_toLabel->setMaximumHeight(iconsize/2);
    m_toLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_toLabel->nativeWidget()->setWordWrap(false);

    m_layout->addItem(m_toLabel, 1, 1, 1, 2);

    m_bodyView = new Plasma::WebView(this);
    //m_bodyView->setMinimumSize(20, 40);
    QString html("<h3>Hi everybody</h3>I hope you're all having a great time on Jamaica, my home country (which you might have noticed after yesterday's bob-marley-on-repeat-for-the-whole-night. I wish I could be with you, but it wasn't meant to be. I'll go out with my friends Kurt and Elvis tonight instead and wish you a happy CampKDE.<br /><br /><em>-- Bob</em>");

    setBody(html);
    m_layout->addItem(m_bodyView, 3, 0, 1, 3);

    m_expandIcon = new Plasma::IconWidget(this);
    m_expandIcon->setIcon("arrow-down-double");
    m_expandIcon->setMinimumSize(12, 12);
    connect(m_expandIcon, SIGNAL(clicked()), this, SLOT(toggleBody()));
    m_layout->addItem(m_expandIcon, 0, 2, 1, 1, Qt::AlignRight | Qt::AlignTop);

    setLayout(m_layout);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
    kDebug() << "EmailWidget built";
}

void EmailWidget::sizeChanged()
{
    qreal left, top, right, bottom; // TODO: Use them...
    getContentsMargins(&left, &top, &right, &bottom);

    setMinimumSize(m_layout->sizeHint(Qt::MinimumSize));
    setPreferredSize(m_layout->sizeHint(Qt::PreferredSize));

    setMinimumSize(m_layout->minimumSize()); // FIXME: borders? setSpacing(0)?
    resize(m_layout->minimumSize()); // FIXME: borders? setSpacing(0)?
    emit geometryChanged(m_layout->minimumSize());
}

void EmailWidget::resizeIcon(int iconsize)
{
    m_layout->setColumnFixedWidth(0, iconsize);
    m_icon->resize(iconsize, iconsize);
    m_icon->setMinimumHeight(iconsize);
    m_icon->setMaximumHeight(iconsize);
    m_layout->updateGeometry();
}

void EmailWidget::toggleBody()
{
    if (!m_expanded) {
        expand();
    } else {
        collapse();
    }
}

void EmailWidget::collapse()
{
    kDebug() << "hiding body";
    m_expandIcon->setIcon("arrow-down-double");
    m_expanded = false;
    setTiny();
    //kDebug() << "after:" << m_layout->geometry();
}


void EmailWidget::expand()
{
    setLarge(true);
    m_expanded = true;
    kDebug() << "showing body";
    if (!m_fetching) {
        fetchPayload();
    } else {
        kDebug() << "not fetching payload";
    }
}

void EmailWidget::updateColors()
{
    QPalette p = palette();
    //p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    p.setColor(QPalette::Window, Qt::transparent);

    // FIXME: setting foreground color apparently doesn't work?
    p.setColor(QPalette::Text, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    //p.setColor(QPalette::WindowText, Qt::green);
    //kDebug() << "Textcolor:" << Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);

    setPalette(p);
    m_bodyView->page()->setPalette(p);
}

void EmailWidget::setSubject(const QString& subject)
{
    //m_subject = subject;
    if (m_subjectLabel && !subject.isEmpty()) {
        m_subjectLabel->setText(subject);
    }
    m_subject = subject;
}

void EmailWidget::setTo(const QStringList& toList)
{
    kDebug() << "Setting recipient" << toList;
    if (m_toLabel && toList.count()) {
        m_toLabel->setText(toList.join(", "));
    }
}

void EmailWidget::setBody(const QString& body)
{
    if (m_bodyView && !body.isEmpty()) {
        QString c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name(); // FIXME: nasty color hack
        m_bodyView->setHtml("<font color=\"" + c + "\">" + body + "</font>"); // FIXME: Urks. :(
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
    kDebug() << "Fetched" << items.count() << "Items.";
    foreach( const Akonadi::Item &item, items ) {

        MessagePtr msg = item.payload<MessagePtr>();

        /*
        Id", item.id()
        Subject", msg->subject()->asUnicodeString()
        From", msg->from()->asUnicodeString()
        DateTime", msg->date()->dateTime().date()
        To", msg->to()->asUnicodeString()
        Cc", msg->cc()->asUnicodeString()
        Bcc", msg->bcc()->asUnicodeString()
        */
        kDebug() << item.id() << msg->mainBodyPart()->body();
        setBody(msg->mainBodyPart()->body());
    }
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
