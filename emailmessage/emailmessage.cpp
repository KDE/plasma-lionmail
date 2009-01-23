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
#include <QTimer>
#include <QClipboard>

//KDE
#include <KIcon>
#include <KConfigDialog>
#include <KStandardDirs>
#include <KToolInvocation>

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
      m_icon(0),
      m_dialog(0)
{
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setAcceptsHoverEvents(true);

    // initialize the widget
    (void)graphicsWidget();
    m_dialog->hideBody();
    //resize(graphicsWidget()->sizeHint());
}

EmailMessage::~EmailMessage()
{
}

void EmailMessage::init()
{
    kDebug() << "init: email";
    KConfigGroup cg = config();


    Plasma::ToolTipManager::self()->registerWidget(this);

    setPopupIcon(QIcon());
    // TODO ...


}

QGraphicsWidget* EmailMessage::graphicsWidget()
{
    kDebug() << "init: email";
    if (!m_dialog) {
        kDebug() << "new EmailDialog";
        m_dialog = new EmailDialog(this);
    }
    return m_dialog->dialog();
}

void EmailMessage::popupEvent(bool show)
{
    if (show) {

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
    m_subject = subject;
    m_dialog->m_subjectLabel->setText(subject);
    // TODO: update widgets...
}

void EmailMessage::setTo(const QStringList& toList)
{
    m_to = toList;
    m_dialog->m_toLabel->setText(toList.join(", "));
}

void EmailMessage::setBody(const QString& body)
{
    m_body = body;
    // TODO: update widgets...
}

void EmailMessage::setAbstract(const QString& abstract)
{
    m_abstract = abstract;
    // TODO: update widgets...
}

void EmailMessage::setDate(const QDate& date)
{
    m_date = date;
    // TODO: update widgets...
}

void EmailMessage::setFrom(const QStringList& fromList)
{
    m_from = fromList;
}

void EmailMessage::setCc(const QStringList& ccList)
{
    m_cc = ccList;
}

void EmailMessage::setBcc(const QStringList& bccList)
{
    m_bcc = bccList;
}

#include "emailmessage.moc"
