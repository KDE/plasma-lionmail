/***************************************************************************
 *   Copyright 2010 by Sebastian Kügler <sebas@kde.org>                    *
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

#include "emailnotifier.h"


#include <KConfigDialog>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/Extender>
#include <Plasma/DataEngine>

#include <Akonadi/ServerManager>
//#include "mailextender.h"
#include "../emailmessage/emailmessage.h"
#include "dialog.h"

EmailNotifier::EmailNotifier(QObject *parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args),
    m_theme(0),
    ui(0),
    m_dialog(0),
    m_collectionId(0)
{
    setPopupIcon("mail-unread-new");
    setHasConfigurationInterface(true);
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPassivePopup(true);
    setStatus(Plasma::ActiveStatus);

    kDebug() << "LionmailArgs" << args;
    foreach (QVariant a, args) {
        kDebug() << args.at(0).toString() << a.toString();
        m_collectionId = args.at(0).toString().toInt();
        /*
        QString firstarg = a.toString();
        if (firstarg.startsWith("EmailCollection-")) {
            //m_collections << firstarg;
            kDebug() << "Loading EmailCollection from commandline argument (" << firstarg << ").";
        } else {
            kWarning() << "argument has to be in the form \"EmailCollection-<id>\", but it isn't (" << firstarg << ")";
        }
        */
    }
    if (!m_collectionId == 0) {
        kDebug() << "No valid collection ID given";
    }
}

EmailNotifier::~EmailNotifier()
{
    delete ui;
}

void EmailNotifier::configChanged()
{
    KConfigGroup cg = config();
    if (!m_collectionId) {
        m_collectionId = cg.readEntry("unreadCollectionId", 0);
        kDebug() << "using config" << m_collectionId;
    } else {
        cg.writeEntry("unreadCollectionId", m_collectionId);
        kDebug() << "writing config" << m_collectionId;
    }
    kDebug() << "Using collection ID" << m_collectionId;
    m_allowHtml = cg.readEntry("allowHtml", false);
}

QGraphicsWidget* EmailNotifier::graphicsWidget()
{
    if (!m_dialog) {
        kDebug() << "============================================== NEW";
        m_dialog = new Dialog(m_collectionId, this);
        connect(m_dialog, SIGNAL(statusChanged(int, const QString&)), this, SLOT(statusChanged(int, const QString&)));
    }

    return m_dialog;
}

bool EmailNotifier::allowHtml()
{
    return m_allowHtml;
}

void EmailNotifier::init()
{
    //Akonadi::ServerManager::start();
    //dataEngine("akonadi")->connectSource("EmailCollections", this);
    setStatus(Plasma::PassiveStatus);

    configChanged();

    updateToolTip("", 0);
}


void EmailNotifier::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui = new Ui::emailnotifierConfig();
    ui->setupUi(widget);
    parent->addPage(widget, i18n("Collections"), Applet::icon());

}


void EmailNotifier::configAccepted()
{
    KConfigGroup cg = config();

    if (ui->allowHtml->isChecked() != m_allowHtml) {
        m_allowHtml = !m_allowHtml;
        cg.writeEntry("allowHtml", m_allowHtml);
    }

}

void EmailNotifier::statusChanged(int emailsCount, const QString& statusText)
{
    QString icon = "mail-mark-unread";
    if (emailsCount) {
        icon = "mail-mark-unread-new";
        if (!statusText.isEmpty()) {
            updateToolTip(statusText, icon);
        } else {
            const QString _t = i18np("%1 new email", "%1 new emails", emailsCount);
            updateToolTip(_t, icon);
        }
        setStatus(Plasma::ActiveStatus);
    } else {
        updateToolTip(i18nc("tooltip: no new emails", "No new email"), icon);
        setStatus(Plasma::PassiveStatus);
    }
    setPopupIcon(icon);
}

void EmailNotifier::updateToolTip(const QString& statusText, const QString& icon)
{
    m_toolTip = Plasma::ToolTipContent(statusText,
            i18nc("Tooltip sub text", "Click on the icon to view your emails"),
                    KIcon(icon).pixmap(IconSize(KIconLoader::Desktop))
                );
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}


K_EXPORT_PLASMA_APPLET(emailnotifier, EmailNotifier)

#include "emailnotifier.moc"
