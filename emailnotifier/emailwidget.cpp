/*
    Copyright 2008-2010 by Sebastian Kügler <sebas@kde.org>

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
#include <QLabel>
#include <QTextDocument>
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
#include <Akonadi/ItemModifyJob>

#include <kpimutils/email.h>
#include <kpimutils/linklocator.h>
#include <kmime/kmime_dateformatter.h>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/kmime/messagestatus.h>

// Plasma
#include <Plasma/Animation>
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/PushButton>

// own
#include "emailwidget.h"

using namespace Plasma;

EmailWidget::EmailWidget(QGraphicsWidget *parent)
    : Frame(parent),
      id(0), // what it's supposed to be
      m_applet(0),
      // Are we already fetching the data?
      m_fetching(false),
      m_item(0),

      // Display options
      m_allowHtml(false), // no html emails for now
      m_showSmilies(true),

      // UI Items
      m_fromLabel(0),
      m_bodyWidget(0),
      m_newIcon(0),
      m_importantIcon(0),
      m_deleteButton(0),
      m_expandIconAnimation(0),
      m_actionsAnimation(0),
      m_bodyAnimation(0),
      m_fontAdjust(0),
      m_hasFullPayload(false)

{
    setAcceptHoverEvents(true);

    m_monitor = 0;
    m_expanded = false;
    buildDialog();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(updateColors()));

    m_disappearAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);

}

EmailWidget::~EmailWidget()
{
}

int EmailWidget::widgetHeight(int size)
{
    //kDebug() << "minwidth:" << m_layout->minimumWidth() << m_toLabel->minimumSize().width();
    int h = 0;
    switch (size) {
        case Icon:
            h = KIconLoader::SizeSmall;
            break;
        case Tiny:
            h = qMax((int)KIconLoader::SizeSmall, (int)(m_subjectLabel->minimumHeight()*1.5));
            break;
        case Small:
            h = KIconLoader::SizeMedium*1.4; // 32 * 1.3
            break;
        case Medium:
            h = (int)(KIconLoader::SizeHuge * 1.5); // 96  FIXME: header is not always that big
            break;
        case Large:
            h = (int)(KIconLoader::SizeEnormous * 1.5); // 192
            break;
    }
    //kDebug() << "size for" << size << h;
    return h;
}

void EmailWidget::setSize(int appletsize)
{
    kDebug() << "setting widgetsize" << appletsize;
    if (appletsize == EmailWidget::Tiny) {
        //setTiny();
    } else if (appletsize == EmailWidget::Small) {
        setSmall();
    } else if (appletsize == EmailWidget::Medium) {
        //setMedium();
    } else if (appletsize == EmailWidget::Large) {
        setLarge();
    } else {
        kDebug() << "Don't understand appletsize" << appletsize;
    }
}

void EmailWidget::updateSize(int h)
{
    setMinimumHeight(-1);
    setMinimumHeight(h);
    setPreferredHeight(h+6);
    setMaximumHeight(h+6);
    // In Layouts, we want to restrict the appletsize as much as possible,
    // on the desktop or more generally, in an applet, we let the applet
    // itself manage the max size
    if (m_appletSize != Large && !m_applet) {
        //setMaximumHeight(h);
        //kDebug() << "MAX HEIGHT" << h;
    } else {
        //setMaximumHeight(QWIDGETSIZE_MAX);
        //kDebug() << "MAX HEIGHT INF" << h;
    }
    m_layout->updateGeometry();
    updateGeometry();
}

void EmailWidget::setSmall()
{
    if (m_appletSize == Small) {
        return;
    }

    //kDebug() << "Small ...";
    m_appletSize = Small;

    //m_subjectLabel->show();
    m_subjectLabel->setMinimumWidth(140);
    //m_expandIcon->show();
    m_expandIcon->setIcon("arrow-down");
    //m_fromLabel->show();
    resizeIcon(32);
    refreshFlags(true);
    int h = widgetHeight(m_appletSize);
    updateSize(h);
}

void EmailWidget::showActions(bool show)
{
    if (!m_expanded) {
        //return;
    }
    if (!m_actionsAnimation) {
        //m_actionsAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
        /*
        m_actionsAnimation->setProperty("startOpacity", 0.0);
        m_actionsAnimation->setProperty("targetOpacity", 1.0);
        //m_actionsAnimation->setProperty("duration", 2000);
        m_actionsAnimation->setProperty("duration", 2000);

        m_actionsAnimation->setTargetWidget(m_actionsWidget);
        */
        m_actionsWidget->setProperty("transformOriginPoint", QPointF(48, 12)); // centered
        m_actionsAnimation  = new QPropertyAnimation(m_actionsWidget, "scale");
        m_actionsAnimation ->setDuration(150);
        m_actionsAnimation ->setStartValue(0.0);
        m_actionsAnimation ->setEndValue(1.0);

    }
    if (m_actionsAnimation->state() == QAbstractAnimation::Running) {
        if (!show) {
            m_actionsAnimation->setDirection(QAbstractAnimation::Backward);
            connect(m_actionsAnimation, SIGNAL(finished()), this, SLOT(hideLater()));
        } else {
            m_actionsAnimation->setDirection(QAbstractAnimation::Forward);
            disconnect(m_actionsAnimation, SIGNAL(finished()), this, SLOT(hideLater()));
        }
        return;
    }
    if (show) {
        m_actionsWidget->show();
        disconnect(m_actionsAnimation, SIGNAL(finished()), this, SLOT(hideLater()));
        m_actionsAnimation->setDirection(QAbstractAnimation::Forward);
    } else {
        m_actionsAnimation->setDirection(QAbstractAnimation::Backward);
        connect(m_actionsAnimation, SIGNAL(finished()), SLOT(hideLater()));
    }
    m_actionsAnimation->start();
}

void EmailWidget::showBody(bool show)
{
    if (!m_expanded) {
        return;
    }
    if (!m_bodyAnimation) {
        m_bodyAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
        m_bodyAnimation->setProperty("startOpacity", 0.0);
        m_bodyAnimation->setProperty("targetOpacity", 8.0);
        m_bodyAnimation->setProperty("duration", 300);
        m_bodyAnimation->setTargetWidget(m_bodyWidget);
    }
    if (m_bodyAnimation->state() == QAbstractAnimation::Running) {
        if (!show) {
            m_bodyAnimation->setDirection(QAbstractAnimation::Backward);
            connect(m_bodyAnimation, SIGNAL(finished()), this, SLOT(resizeLater()));
        } else {
            m_bodyAnimation->setDirection(QAbstractAnimation::Forward);
            disconnect(m_bodyAnimation, SIGNAL(finished()), this, SLOT(resizeLater()));
        }
        return;
    }
    if (show) {
        setLarge();
        m_bodyWidget->show();
        disconnect(m_bodyAnimation, SIGNAL(finished()), this, SLOT(resizeLater()));
        m_bodyAnimation->setDirection(QAbstractAnimation::Forward);
    } else {
        m_bodyAnimation->setDirection(QAbstractAnimation::Backward);
        connect(m_bodyAnimation, SIGNAL(finished()), SLOT(resizeLater()));
    }
    m_bodyAnimation->start();
}

void EmailWidget::showExpandIcon(bool show)
{
    if (!m_expandIconAnimation) {
        m_expandIconAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
        m_expandIconAnimation->setProperty("startOpacity", 0.0);
        m_expandIconAnimation->setProperty("targetOpacity", 1.0);
        m_expandIconAnimation->setProperty("duration", 300);
        m_expandIconAnimation->setTargetWidget(m_expandIcon);
    }
    if (m_expandIconAnimation->state() == QAbstractAnimation::Running) {
        if (!show) {
            m_expandIconAnimation->setDirection(QAbstractAnimation::Backward);
            connect(m_expandIconAnimation, SIGNAL(finished()), this, SLOT(hideLater()));
        } else {
            m_expandIconAnimation->setDirection(QAbstractAnimation::Forward);
            disconnect(m_expandIconAnimation, SIGNAL(finished()), this, SLOT(hideLater()));
        }
        return;
    }
    if (show) {
        m_expandIcon->show();
        disconnect(m_expandIconAnimation, SIGNAL(finished()), this, SLOT(hideLater()));
        m_expandIconAnimation->setDirection(QAbstractAnimation::Forward);
    } else {
        m_expandIconAnimation->setDirection(QAbstractAnimation::Backward);
        connect(m_expandIconAnimation, SIGNAL(finished()), SLOT(hideLater()));
    }
    m_expandIconAnimation->start();
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

    m_appletSize = Large;
    m_expandIcon->setIcon("arrow-up");
    m_subjectLabel->show();
    m_fromLabel->show();

    showActions(true);
    refreshFlags(true);
    resizeIcon(32);
    setMinimumWidth(m_layout->minimumSize().width());
    if (!m_fetching) {
        fetchPayload();
    }
    updateSize(widgetHeight(Large));
}

void EmailWidget::buildDialog()
{
    //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum); 
    updateColors();

    m_emailWidget = new QGraphicsWidget(this);
    
    m_layout = new QGraphicsGridLayout(m_emailWidget);
    m_layout->setColumnFixedWidth(0, 40); // This could probably be a bit more dynamic should be dynamic
    m_layout->setColumnPreferredWidth(1, 180);
    m_layout->setColumnFixedWidth(2, 22);
    m_layout->setRowFixedHeight(0, 16);
    m_layout->setRowFixedHeight(1, 16);
    m_layout->setHorizontalSpacing(4);
    
    m_icon = new Plasma::IconWidget(m_emailWidget);
    m_icon->setToolTip(i18nc("open icon tooltip", "Open this Email"));
    m_icon->setIcon("mail-mark-read");
    m_icon->setAcceptHoverEvents(false);
    resizeIcon(32);
    m_layout->addItem(m_icon, 0, 0, 2, 1, Qt::AlignTop);
    connect(m_icon, SIGNAL(clicked()), SLOT(itemActivated()));

    m_subjectLabel = new Plasma::Label(m_emailWidget);
    m_subjectLabel->nativeWidget()->setWordWrap(false);
    m_subjectLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_subjectLabel->setMinimumWidth(100);
    m_layout->addItem(m_subjectLabel, 0, 1, 1, 1, Qt::AlignTop);
    setSubject(QString());

    m_fromLabel = new Plasma::Label(m_emailWidget);
    m_fromLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fromLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    //m_fromLabel->setStyleSheet(m_stylesheet);
    m_fromLabel->setOpacity(0.8);
    setFrom(m_from);

    m_actionsWidget = new QGraphicsWidget(m_emailWidget);
    //m_actionsWidget->setZValue(10);
    m_actionsLayout = new QGraphicsLinearLayout(m_actionsWidget);
    m_actionsLayout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //m_actionsLayout->setOrientation(Qt::Vertical);

    int s = KIconLoader::SizeSmall * 1.5;
    m_newIcon = new PushButton(m_actionsWidget);
    m_newIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_newIcon->setIcon(KIcon("mail-mark-unread-new"));
    m_newIcon->setMinimumWidth(s);
    m_newIcon->setMaximumHeight(s);
    m_newIcon->setMaximumWidth(s);
    m_newIcon->setCheckable(true);
    connect(m_newIcon, SIGNAL(clicked()), this, SLOT(flagNewClicked()));

    m_importantIcon = new PushButton(m_actionsWidget);
    m_importantIcon->setIcon(KIcon("mail-mark-important"));
    m_importantIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_importantIcon->setMinimumWidth(s);
    m_importantIcon->setMaximumHeight(s);
    m_importantIcon->setMaximumWidth(s);
    m_importantIcon->setCheckable(true);
    connect(m_importantIcon, SIGNAL(clicked()), this, SLOT(flagImportantClicked()));

    QGraphicsWidget* spacer = new QGraphicsWidget(m_actionsWidget);
    spacer->setMinimumHeight(8);
    spacer->setMaximumHeight(8);
    spacer->setMinimumWidth(8);
    spacer->setMaximumWidth(8);

    m_deleteButton = new PushButton(m_actionsWidget);
    m_deleteButton->setIcon(KIcon("edit-delete"));
    m_deleteButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_deleteButton->setMinimumWidth(s);
    m_deleteButton->setMaximumHeight(s);
    m_deleteButton->setMaximumWidth(s);
    m_deleteButton->setCheckable(true);
    connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));

    m_actionsLayout->addItem(m_newIcon);
    m_actionsLayout->addItem(m_importantIcon);
    m_actionsLayout->addItem(spacer);
    m_actionsLayout->addItem(m_deleteButton);

    //m_layout->addItem(m_actionsWidget, 2, 0, 1, 1, Qt::AlignCenter | Qt::AlignLeft);
    m_actionsWidget->hide(); // for now

    m_layout->addItem(m_fromLabel, 1, 1, 1, 2, Qt::AlignTop | Qt::AlignRight);

    // From and date
    m_bodyWidget = new Plasma::Label(m_emailWidget);
    m_bodyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_bodyWidget->setMinimumHeight(80);
    //m_bodyWidget->setOpacity(.8);
    m_bodyWidget->hide();

    m_bodyWidget->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    connect(m_bodyWidget, SIGNAL(linkActivated(const QString&)), this, SLOT(linkClicked(const QString&)));
    //setFrom(i18n("Unknown Sender"));
    m_layout->addItem(m_bodyWidget, 2, 1, 1, 2, Qt::AlignTop);


    m_expandIcon = new Plasma::IconWidget(m_emailWidget);
    m_expandIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_expandIcon->setIcon("arrow-down");
    m_expandIcon->setMinimumSize(22, 22);
    m_expandIcon->setMaximumSize(22, 22);
    connect(m_expandIcon, SIGNAL(clicked()), this, SLOT(toggleBody()));
    m_layout->addItem(m_expandIcon, 0, 2, 1, 1, Qt::AlignRight | Qt::AlignTop);
    m_expandIcon->setOpacity(0);

    m_emailWidget->setLayout(m_layout);
    //setNew(m_isNew);
    //setImportant(m_isImportant);
    refreshFlags();
    
    m_anchorLayout = new QGraphicsAnchorLayout(this);
    // Fix the actual email widget on top-left and bottom-right corners
    m_anchorLayout->addCornerAnchors(m_emailWidget, Qt::TopLeftCorner, 
                                     m_anchorLayout, Qt::TopLeftCorner);
    m_anchorLayout->addCornerAnchors(m_emailWidget, Qt::BottomRightCorner, 
                                     m_anchorLayout, Qt::BottomRightCorner);
    
    m_anchorLayout->addCornerAnchors(m_expandIcon, Qt::TopRightCorner,
                                     m_anchorLayout, Qt::TopRightCorner);
    m_anchorLayout->addAnchor(m_actionsWidget, Qt::AnchorTop, m_anchorLayout, Qt::AnchorTop);
    m_anchorLayout->addAnchor(m_actionsWidget, Qt::AnchorRight, m_expandIcon, Qt::AnchorLeft);


    setLayout(m_anchorLayout);

    updateColors();
}

void EmailWidget::refreshFlags()
{
    refreshFlags(m_flagsShown);
}

void EmailWidget::flagNewClicked()
{
    kDebug() << "New clicked";
    if (!m_item.isValid()) {
        kDebug() << "Item invalid, making a new one...";
        m_item = Akonadi::Item(id);
    }

    if (m_status.isRead()) {
        m_status.setUnread();
    } else {
        m_status.setRead();
    }
    syncItemToAkonadi();
    //refreshFlags();
}

/*
bool EmailWidget::isNew()
{
    return m_status.isRead();
}
*/
void EmailWidget::flagImportantClicked()
{
    kDebug() << "Important clicked";

    if (!m_item.isValid()) {
        m_item = Akonadi::Item(id);
    }
    m_status.setImportant(!(m_status.isImportant()));
    syncItemToAkonadi();
    refreshFlags();
}

void EmailWidget::syncItemToAkonadi()
{
    m_item.setFlags(m_status.statusFlags());
    Akonadi::ItemModifyJob* mjob = new Akonadi::ItemModifyJob(m_item);

    // FIXME: pending revision conflict check in Akonadi, consult with Volker
    // we're disabling it for now.
    mjob->disableRevisionCheck();


    mjob->start(); // Fire and forget, we're assuming no conflicts
    kDebug() << "Sending modifications to Akonadi now ...";
    connect(mjob, SIGNAL(result(KJob*)), SLOT(syncItemResult(KJob*)));
}

void EmailWidget::syncItemResult(KJob* job)
{
    if (job->error()) {
        kDebug() << "SyncJob Failed:" << job->errorString();
    } else {
        kDebug() << "SyncJob Success!";
    }
}

void EmailWidget::refreshFlags(bool show)
{
    m_flagsShown = show;

    if (!m_icon) {
        return;
    }

    // Update larger icon with most important flag
    if (m_status.isImportant()) {
        m_icon->setIcon("mail-mark-important");
    } else if (m_status.isUnread()) {
        m_icon->setIcon("mail-mark-unread-new");
    } else {
        m_icon->setIcon("mail-mark-read");
    }

    m_newIcon->setChecked(m_status.isUnread());
    setSubject(m_subject); // for updating font weight on the subject line
    if (m_status.isUnread()) {
        m_newIcon->setIcon(KIcon("mail-mark-read"));
        m_newIcon->setToolTip(i18nc("flag new", "Message is marked as New, click to mark as Read"));
    } else {
        m_newIcon->setIcon(KIcon("mail-mark-unread-new"));
        m_newIcon->setToolTip(i18nc("flag new", "Message is marked as Read, click to mark as New"));
    }

    m_importantIcon->setChecked(m_status.isImportant());
    if (m_status.isImportant()) {
        m_importantIcon->setToolTip(i18nc("flag important", "Message is marked as Important, click to remove this flag"));
    } else {
        m_importantIcon->setToolTip(i18nc("flag important", "Click to mark message as Important"));
    }
}

void EmailWidget::deleteClicked()
{
    setDeleted(m_deleteButton->isChecked());
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
    if (!m_expanded) {
        expand();
    } else {
        collapse();
    }
}

void EmailWidget::collapse()
{
    //kDebug() << "hiding body";
    m_expandIcon->setIcon("arrow-down");
    showBody(false);
    //showActions(false);
    m_expanded = false; // needs to be unset last, otherwise animations won't trigger
    emit collapsed();
}


void EmailWidget::expand()
{
    //kDebug() << "showing body";
    m_expanded = true;  // needs to be set first, otherwise animations won't trigger
    showBody();
}

void EmailWidget::updateColors()
{
    QPalette p = palette();

    // Set background to transparent and use the theme to provide contrast with the text
    p.setColor(QPalette::Base, Qt::transparent); // new in Qt 4.5
    p.setColor(QPalette::Window, Qt::transparent); // For Qt 4.4, remove when we depend on 4.5

    QColor text = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    text.setAlphaF(.8);
    QColor link = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    link.setAlphaF(qreal(.8));
    QColor linkvisited = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    linkvisited.setAlphaF(qreal(.6));

    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Link, link);
    p.setColor(QPalette::LinkVisited, linkvisited);

    setPalette(p);

    qreal fontsize = KGlobalSettings::smallestReadableFont().pointSize();
    fontsize = qMax(qreal(4.0), fontsize+m_fontAdjust);
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
                    opacity: 0.8; \
                } \
                .header { \
                    overflow: hidden; \
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

    if (m_fromLabel) {
        // FIXME: links in fromLabel are still blue :/
        //m_fromLabel->setPalette(p);
        //m_fromLabel->setStyleSheet(m_stylesheet);
        //kDebug() << "Setting palette, text:" << text;
        //m_fromLabel->nativeWidget()->setPalette(p);
    }
}

void EmailWidget::setUrl(KUrl url)
{
    kDebug() << url.url() << url.queryItemValue("item");
    id = url.queryItemValue("item").toLongLong();
    kDebug() << "Setting id from url:" << id << url.url();
    m_url = url;
    fetchPayload(false);
}

void EmailWidget::setAllowHtml(bool allow)
{
    m_allowHtml = allow;
}

void EmailWidget::setSubject(const QString& subject)
{
    QString tmpSubject = subject;
    if (m_subjectLabel) {
        if (subject.isEmpty()) {
            tmpSubject = i18nc("empty subject label", "(No Subject)");
        }
        if (m_status.isUnread()) {
            m_subjectLabel->setText(QString("<b>%1</b>").arg(tmpSubject));
        } else {
            m_subjectLabel->setText(tmpSubject);
        }
    }
    m_subject = subject;
}

void EmailWidget::setTo(const QStringList& toList)
{
    m_to = toList;
}

void EmailWidget::setRawBody(const QString& body)
{
    if (m_bodyWidget) {
        QString html;
        if (m_body.isEmpty() && m_fetching) {
            html = i18n("<h3>Loading body...</h3>");
        } else {
            //html = i18n("<h3>Empty body loaded.</h3>");
        }

        if (body.isEmpty() && !m_body.isEmpty()) {
            html = m_body;
            m_fetching = false;
        } else if (!body.isEmpty()) {
            html = body;
            m_fetching = false;
        }
        if (!html.isEmpty()) {
            html = i18n("<style type=\"text/css\">%1</style><body>%2</body>", m_stylesheet, html);
            html.replace("<br />\n<br />", "<br />\n");
            //kDebug() << html;
            m_bodyWidget->setText(html);
        }
    }
}

void EmailWidget::setBody(MessagePtr msg)
{
    // Borrowed from Tom Albers' mailody/src/messagedata.cpp
    m_msg = msg;

    QString body = m_body;
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
    // FIXME: Attachments ...
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
            body = LinkLocator::convertToHtml(plainPart, flags);
        } else {
            body = htmlPart;
        }
        // when replying prefer plain
        !plainPart.isEmpty() ? raw = plainPart : raw = htmlPart;
    } else {
        // show plain
        if ( plainPart.trimmed().isEmpty() ) {
            body = LinkLocator::convertToHtml(htmlPart, flags);
        } else {
            body = LinkLocator::convertToHtml(plainPart, flags);
        }
        // when replying prefer plain
        !plainPart.isEmpty() ? raw = plainPart : raw = htmlPart;
    }
    if (!body.isEmpty()) {
        m_hasFullPayload = true;
    }
    setRawBody(body);
}

void EmailWidget::fetchPayload(bool full)
{
    if (id <= 0) {
        kDebug() << "id invalid";
        return;
    }
    if (m_hasFullPayload) {
        return;
    }
    //m_bodyView->setMinimumHeight(30);
    //updateSize(widgetHeight(Large)-50);
    kDebug() << "Fetching payload for " << id;
    Akonadi::ItemFetchJob* fetchJob = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
    if (full) {
        fetchJob->fetchScope().fetchFullPayload();
        m_fetching = true;
    } else {
        fetchJob->fetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope );
    }
    connect( fetchJob, SIGNAL(result(KJob*)), SLOT(fetchDone(KJob*)) );
    if (m_body.isEmpty()) {
        m_bodyWidget->setText(i18n("<h3>Loading body...</h3>"));
    }
}

void EmailWidget::fetchDone(KJob* job)
{
    kDebug() << "fetchjob returning";
    if ( job->error() ) {
        kDebug() << "Error fetching item" << id << ": " << job->errorString();
        setRawBody(i18n("<h3>Fetching email body %1 failed: <p /></h3><pre>%2</pre>", id, job->errorString()));
        return;
    }
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
        connect( m_monitor, SIGNAL(itemChanged(const Akonadi::Item&, const QSet<QByteArray>&)),
            this, SLOT(itemChanged(const Akonadi::Item&)) );

        itemChanged(item);
    }
}

Akonadi::Item EmailWidget::item()
{
    return m_item;
}

void EmailWidget::itemChanged(const Akonadi::Item& item)
{
    if (!item.isValid()) {
        kDebug() << "item is gone";
        return;
    }
    //m_item = Akonadi::Item(item.id());
    m_item = item;
    if (item.hasPayload<MessagePtr>()) {
        MessagePtr msg = item.payload<MessagePtr>();
        id = item.id(); // This shouldn't change ... right?
        m_status.setStatusFromFlags(item.flags());
        setSubject(msg->subject()->asUnicodeString());
        setFrom(msg->from()->asUnicodeString());
        setDate(msg->date()->dateTime().dateTime());
        setTo(QStringList(msg->to()->asUnicodeString()));
        m_cc = QStringList(msg->cc()->asUnicodeString());
        m_bcc = QStringList(msg->bcc()->asUnicodeString());
        setBody(msg);
        refreshFlags();
        //kDebug() << "=== item changed" << id << msg->subject()->asUnicodeString() << item.flags();
        //kDebug() << "new:" << m_isNew << "important:" << m_isImportant << "task:" << m_isTask;
    } else {
        kDebug() << "Could not fetch email payload for" << m_item.url();
        //setSubject(i18n("Could not fetch email payload"));
    }
}

void EmailWidget::setDate(const QDateTime& date)
{
    m_date = date;
}

void EmailWidget::setFrom(const QString& from)
{
    m_from = from;
    if (m_fromLabel) {
        if (!m_from.isEmpty()) {
            QString label = from;
            //m_fromLabel->setText(KPIMUtils::LinkLocator::convertToHtml(from));
            QStringList p = from.split("<");
            //kDebug() << "FROM:" << from << p.count();
            if (p.count()) {
                label = p[0].trimmed();                
            }
            if (m_date.isNull()) {
                label = i18nc("sender", "%1", label);
            } else {
                label = i18nc("sender and date", "%1, %2", label, KGlobal::locale()->formatDateTime(m_date, KLocale::FancyShortDate));
            }
            m_fromLabel->setText(Qt::escape(label));
        } else {
            m_fromLabel->setText(i18n("<i>Sender unkown</i>"));
        }
    }
}

void EmailWidget::setDeleted(bool deleted)
{
    if (!deleted && m_isDeleted) {
    }
    m_isDeleted = deleted;
    m_deleteButton->setChecked(m_isDeleted);

    // Make it tranlucent for now

    qreal o = .7;
    if (!m_isDeleted) {
        o = 1.0;
        //setOpacity(1.0);
        //return;
    }
    //setOpacity(o);
    m_disappearAnimation->setProperty("duration", 2000);
    if (m_isDeleted) {
        m_disappearAnimation->setProperty("startOpacity", 1.0);
        m_disappearAnimation->setProperty("targetOpacity", 0.0);
        connect(m_disappearAnimation, SIGNAL(finished()), this, SLOT(disappearAnimationFinished()));
    } else {
        m_disappearAnimation->setProperty("startOpacity", opacity());
        m_disappearAnimation->setProperty("targetOpacity", 1.0);
        disconnect(m_disappearAnimation, SIGNAL(finished()), this, SLOT(disappearAnimationFinished()));
    }
    m_disappearAnimation->setTargetWidget(this);
    m_disappearAnimation->start();
}

void EmailWidget::disappearAnimationFinished()
{
    if (!m_isDeleted) {
        return;
    }
    disconnect( m_monitor, SIGNAL(itemChanged(const Akonadi::Item&, const QSet<QByteArray>&)),
            this, SLOT(itemChanged(const Akonadi::Item&)) );

    emit deleteMe();
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

void EmailWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    if (m_appletSize > Tiny) {
        showActions(true);
    }
    showExpandIcon(true);
}

void EmailWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    showActions(false);
    showExpandIcon(false);
}

void EmailWidget::hideLater()
{
    // Used as endpoint for a hide animation
    Plasma::Animation* a = dynamic_cast<Plasma::Animation*>(sender());
    if (a) {
        QGraphicsWidget* w = a->targetWidget();
        if (w) {
            w->hide();
        }
    }
}

void EmailWidget::resizeLater()
{
    //kDebug() << "Smalling";
    setSmall();
}

void EmailWidget::itemActivated()
{
    if (m_item.isValid()) {
        emit activated(m_item.url());        
    }
}

void EmailWidget::linkClicked(const QString &link)
{
    kDebug() << "Link clicked:" << link;
    emit activated(QUrl(link));
    }


void EmailWidget::wheelEvent (QGraphicsSceneWheelEvent * event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->delta() < 0) {
            kDebug() << "-Decrease size";
            m_fontAdjust--;
        } else {
            kDebug() << "+Increase size";
            m_fontAdjust++;
        }
        updateColors();
        setRawBody(QString());
    }
}

KUrl EmailWidget::url()
{
    return KUrl(QString("akonadi:?item=%1").arg(id));
}

void EmailWidget::startDrag()
{
    //kDebug() << "Starting drag!";
    QMimeData* mimeData = new QMimeData();
    QList<QUrl> urls;
    urls << url();
    mimeData->setUrls(urls);

    // This is a bit random, but we need a QWidget for the constructor
    QDrag* drag = new QDrag(m_subjectLabel->nativeWidget());
    drag->setMimeData(mimeData);
    drag->setPixmap(m_icon->icon().pixmap(64, 64));
    if (drag->start(Qt::CopyAction | Qt::MoveAction)) {
        kDebug() << "dragging starting" << url();
    }
}

QString EmailWidget::stripTags(QString input)
{
    if (input.isEmpty()) {
        return input;
    }
    QString txt = input;
    QRegExp regex("<.*>");
    regex.setMinimal(true);
    txt = txt.remove(regex);
    txt.replace("\n", " ");
    txt.replace("\t", " ");
    return txt;
}



#include "emailwidget.moc"
