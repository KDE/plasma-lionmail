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
#include "emaildialog.h"

//Qt

//KDE

//plasma
#include <Plasma/Dialog>
#include <Plasma/Label>
#include <Plasma/IconWidget>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>



K_EXPORT_PLASMA_APPLET(emailmessage, EmailMessage)

using namespace Plasma;

EmailMessage::EmailMessage(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_dialog(0),
      m_icon(0)
{
    KGlobal::locale()->insertCatalog("plasma_applet_lionmail");
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setAcceptsHoverEvents(true);

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

    //m_dialog->hideBody();

    Plasma::ToolTipManager::self()->registerWidget(this);
    // TODO ...

}

QGraphicsWidget* EmailMessage::graphicsWidget()
{
    if (!m_dialog) {
        kDebug() << "new EmailDialog";
        m_dialog = new EmailDialog(this);
    }
    return m_dialog->dialog();
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

        if (contentsRect().height() > 100) {
            m_dialog->showBody();
        } else {
            m_dialog->hideBody();
        }
        update();
    }
}
/*
void EmailMessage::setSmall
        if (contentsRect().height() > 100) {
            m_dialog->hideBody();
        } else {
            m_dialog->showBody();
        }

*/

void EmailMessage::setSubject(const QString& subject)
{
    m_dialog->setSubject(subject);
}

void EmailMessage::setTo(const QStringList& toList)
{
    m_dialog->setTo(toList.join(", "));
}

void EmailMessage::setBody(const QString& body)
{
    m_dialog->setBody(body);
}

void EmailMessage::setAbstract(const QString& abstract)
{
    m_dialog->setAbstract(abstract);
}

void EmailMessage::setDate(const QDate& date)
{
    m_dialog->setDate(date);
}

void EmailMessage::setFrom(const QStringList& fromList)
{
    m_dialog->setFrom(fromList.join(", "));
}

void EmailMessage::setCc(const QStringList& ccList)
{
    m_dialog->setCc(ccList.join(", "));
}

void EmailMessage::setBcc(const QStringList& bccList)
{
    m_dialog->setBcc(bccList.join(", "));
}

#include "emailmessage.moc"
