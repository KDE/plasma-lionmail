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

#ifndef EMAILMESSAGE_H
#define EMAILMESSAGE_H

//Plasma
#include <Plasma/PopupApplet>
#include <Plasma/ToolTipManager>


#include "emailwidget.h"

//desktop view
namespace Plasma
{
    class IconWidget;
}

class EmailMessage : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        EmailMessage(QObject *parent, const QVariantList &args);
        ~EmailMessage();
        void init();
        QGraphicsWidget* graphicsWidget();
        void constraintsEvent(Plasma::Constraints constraints);

        // The dialog displaying the email
        EmailWidget* m_emailWidget;

        void setTo(const QStringList& toList);
        void setFrom(const QString& fromList);
        void setCc(const QStringList& ccList);
        void setBcc(const QStringList& bccList);
        void setFlags(const QStringList& flagList);

        void setSubject(const QString& subject);
        void setBody(const QString& body);
        void setAbstract(const QString& abstract);
        void setDate(const QDateTime& date);

    protected:
        void popupEvent(bool show);

    private Q_SLOTS:
        void animationUpdate(qreal progress);

    private:
        int m_appletSize;
        ///the icon used when the applet is in the taskbar
        Plasma::IconWidget *m_icon;
        
        void appear(bool show);
        int m_animId;
        bool m_fadeIn;
};

#endif
