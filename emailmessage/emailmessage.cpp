/***************************************************************************
 *   Copyright 2008-2009 by Sebastian Kügler <sebas@kde.org>               *
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
      m_icon(0)
{
    KGlobal::locale()->insertCatalog("plasma_applet_lionmail");
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setAcceptsHoverEvents(true);
    setMinimumSize(12, 12);
    (void)graphicsWidget();
    setPopupIcon("view-pim-mail");
}

EmailMessage::~EmailMessage()
{
}

void EmailMessage::init()
{
    kDebug() << "init: email";
    KConfigGroup cg = config();

    //m_emailWidget->collapse();
    Plasma::ToolTipManager::self()->registerWidget(this);
    // TODO ...

}

QGraphicsWidget* EmailMessage::graphicsWidget()
{
    if (!m_emailWidget) {
        kDebug() << "new EmailWidget";
        m_emailWidget = new EmailWidget(this);
        connect(m_emailWidget, SIGNAL(geometryChanged(QSizeF)), this, SLOT(updateSize(const QSizeF)));
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

        // TODO: better logic for height and width constraints
        kDebug() << "--------------";
        kDebug() << "Widget g, m:" << m_emailWidget->geometry() << m_emailWidget->minimumSize();
        kDebug() << "Applet c, m:" << contentsRect() << minimumSize();
        int proximity = 8; // How close can we get to the minimumSize before we change appearance?

        if (m_emailWidget->minimumSize().width()+proximity > m_emailWidget->geometry().width() ) {
            // not wide enough, only show the icon
            m_emailWidget->setIcon();
        } else {
            kDebug() << "Basing on height" << m_emailWidget->minimumSize().height()+proximity << contentsRect().height();
            if (m_emailWidget->minimumSize().height()+proximity > contentsRect().height()) {
                m_emailWidget->setTiny();
            } else if (contentsRect().height() < 100) { // We can fit date, recipient, attachment and such in
                m_emailWidget->setSmall();
            } else if (contentsRect().height() < 140) { // $even_more_info
                m_emailWidget->setMedium();
            } else {  // Enough space to include the body
                kDebug() << "set large";
                m_emailWidget->setLarge();
            }
        }
        //m_emailWidget->resize(contentsRect().size());
        //updateGeometry();
        //adjustSize();
        update();
    }
}

void EmailMessage::updateSize(const QSizeF newSize)
{
    kDebug() << "Resizing applet from " << size() << " to:" << m_emailWidget->minimumSize() << newSize;
    setPreferredSize(newSize);
    resize(newSize);
    //setGeometry(QRectF(contentsRect().topLeft(), newSize));
    //emit sizeHintChanged();
    emit geometryChanged();
    updateGeometry();
    update();
}
void EmailMessage::setSubject(const QString& subject)
{
    m_emailWidget->setSubject(subject);
}

void EmailMessage::setTo(const QStringList& toList)
{
    m_emailWidget->setTo(toList);
}

void EmailMessage::setBody(const QString& body)
{
    m_emailWidget->setBody(body);
}

void EmailMessage::setAbstract(const QString& abstract)
{
    m_emailWidget->setAbstract(abstract);
}

void EmailMessage::setDate(const QDate& date)
{
    m_emailWidget->setDate(date);
}

void EmailMessage::setFrom(const QString& from)
{
    m_emailWidget->setFrom(from);
}

void EmailMessage::setCc(const QStringList& ccList)
{
    m_emailWidget->setCc(ccList);
}

void EmailMessage::setBcc(const QStringList& bccList)
{
    m_emailWidget->setBcc(bccList);
}

#include "emailmessage.moc"
