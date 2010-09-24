/***************************************************************************
 *   Copyright 2008-2010 by Sebastian Kügler <sebas@kde.org>               *
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

//own
#include "emailmessage.h"
#include "emailwidget.h"

//Qt
#include <QFile>

//KDE

//plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>
#include <Plasma/ToolTipManager>



K_EXPORT_PLASMA_APPLET(emailmessage, EmailMessage)

using namespace Plasma;

EmailMessage::EmailMessage(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_emailWidget(0),
      m_icon(0),
      m_url(0),
      m_animId(-1),
      m_fadeIn(false)
{
    KGlobal::locale()->insertCatalog("plasma_applet_lionmail");
    //setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setAcceptsHoverEvents(true);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setPassivePopup(true);

    //setMinimumSize(80, 48);
    (void)graphicsWidget();
    m_emailWidget->m_applet = this;
    setPopupIcon("mail-mark-read");

    if (args.count() > 0) {
        kDebug() << "EmailMessageArgs" << args;
        // FIXME: error handling
        kDebug() << args.at(0).toString();
        QString arg0 = args.at(0).toString();
        if (arg0.toInt()) {
            QString _u = QString("akonadi:?item=%1").arg(arg0);
            m_url = new KUrl(_u);
            m_emailWidget->setUrl(KUrl(m_url->url()));
        } else {
            if (arg0.startsWith("akonadi:")) {
                m_url = new KUrl(args.at(0).toString());
                m_emailWidget->setUrl(KUrl(m_url->url()));
            } else {
                kWarning() << "I don't understand what you mean, try using the akonadi ID as simple number or an akonadi URL in the style of akonadi:?item=<id>";
            }
        }
    }
}

EmailMessage::~EmailMessage()
{
    delete m_url;
}

void EmailMessage::init()
{
    //kDebug() << "init: email";
    KConfigGroup cg = config();

    Plasma::ToolTipManager::self()->registerWidget(this);
    // TODO ...
    appear(true);
    //setPreferredSize(240, 180);
}

QGraphicsWidget* EmailMessage::graphicsWidget()
{
    if (!m_emailWidget) {
        //kDebug() << "new EmailWidget";
        m_emailWidget = new EmailWidget(this);
        m_emailWidget->m_applet = this;
    }
    return m_emailWidget;
}

void EmailMessage::popupEvent(bool show)
{
    if (show) {
        kDebug() << "Showing popup";
    } else {
        kDebug() << "Hiding popup";
    }
}

void EmailMessage::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & (Plasma::FormFactorConstraint | Plasma::SizeConstraint)) {
        int tiny = m_emailWidget->widgetHeight(EmailWidget::Small);
        //setMinimumSize(tiny, m_emailWidget->minimumWidth());
        //kDebug() << contentsRect();
        kDebug() << "emailwidget minheight" << m_emailWidget->minimumHeight() << contentsRect().height();
        int proximity = 8; // How close can we get to the minimumSize before we change appearance?
        //if (m_emailWidget->minimumSize().width()+proximity > m_emailWidget->geometry().width() ) {
        if (contentsRect().width() < 180 ) {
            // not wide enough, only show the icon
            int small = m_emailWidget->widgetHeight(EmailWidget::Small);
            //setMinimumSize(small, small);
            m_emailWidget->setIcon();
        } else {
            //setMinimumSize(m_emailWidget->minimumWidth(), tiny);

            if (contentsRect().height() < m_emailWidget->widgetHeight(EmailWidget::Small)+proximity) {
                kDebug() << "==>setTiny(Medium)" << contentsRect().height() << m_emailWidget->widgetHeight(EmailWidget::Small)+proximity;
                m_emailWidget->setTiny();
            } else if (contentsRect().height() < m_emailWidget->widgetHeight(EmailWidget::Medium)+proximity) { // We can fit date, recipient, attachment and such in
                kDebug() << "==>setSmall(Medium)" << contentsRect().height() << m_emailWidget->widgetHeight(EmailWidget::Medium)+proximity;
                m_emailWidget->setSmall();
            } else if (contentsRect().height() < m_emailWidget->widgetHeight(EmailWidget::Large)+proximity) { // $even_more_info
                kDebug() << "==>setMedium(Large)" << contentsRect().height() << m_emailWidget->widgetHeight(EmailWidget::Large)+proximity;
                m_emailWidget->setMedium();
            } else {  // Enough space to include the body
                kDebug() << "==>setLarge" << contentsRect().height() << m_emailWidget->widgetHeight(EmailWidget::Large)+proximity;;
                m_emailWidget->setLarge();
            }
        }

        update();
    }
}

void EmailMessage::appear(bool show)
{
    if (m_fadeIn == show) {
        return;
    }
    m_fadeIn = show;
    const qreal sec = 2.4;

    const int FadeInDuration = sec * 1000;


    if (m_animId != -1) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }
    m_animId = Plasma::Animator::self()->customAnimation(40 * (sec), FadeInDuration,
                                                      Plasma::Animator::EaseInCurve, this,
                                                      "animationUpdate");
}


void EmailMessage::animationUpdate(qreal progress)
{
    return; // FIXME: no animation for now
    if (progress == 1) {
        m_animId = -1;
    }
    qreal alpha = m_fadeIn ? progress : 1 - progress;

    alpha = qMax(qreal(0.0), alpha);
    kDebug() << progress << alpha;
    //shear(alpha, 0);
    setOpacity(alpha);
    //scale(alpha, alpha);
    //rotate(alpha);
    update();
}


#include "emailmessage.moc"
