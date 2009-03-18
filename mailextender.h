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

#ifndef MAILEXTENDER_H
#define MAILEXTENDER_H

//Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

// Plasma
#include <Plasma/Label>
#include <Plasma/ExtenderItem>
#include <Plasma/ScrollWidget>
// own
#include "lionmail.h"
#include "emailmessage/emailwidget.h"

class EmailWidget;

class MailExtender : public Plasma::ExtenderItem
{
    Q_OBJECT

    public:
        MailExtender(LionMail * applet, const QString collectionId, Plasma::Extender *ext = 0);
        virtual ~MailExtender();
        void load();

        void setCollection(const QString id);
        QGraphicsWidget* graphicsWidget();
        void setIcon(const QString& icon);
        void setEmailSize(int appletsize);

        QString id();
        QString icon();

        void setDescription(const QString& desc);
        QString description();
        void setInfo(const QString& info);


        void addEmail(EmailWidget* email);
        void setName(const QString name);
        void setShowUnreadOnly(bool show);
        bool showUnreadOnly();
        void setMaxEmails(int max);
        int maxEmails();
        int unreadEmails();

    public Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

    private Q_SLOTS:
        void updateColors();
        void newSource( const QString &source );

    private:
        void buildDialog();

        void connectCollection(QString cid);
        void disconnectCollection(QString cid);

        QString m_id;
        QString m_collection;
        QString m_description;
        QString m_info;
        QString m_iconName;
        QHash<QString, EmailWidget*> emails;

        int m_maxEmails;
        bool m_showUnreadOnly;

        LionMail* m_applet;
        Plasma::DataEngine* engine;
        QGraphicsLinearLayout* m_messageLayout;
        QGraphicsGridLayout* m_layout;
        Plasma::IconWidget* m_icon;
        QGraphicsWidget* m_widget;
        Plasma::Label* m_label;
        Plasma::Label* m_infoLabel;
        Plasma::ScrollWidget* m_emailScroll;
        QGraphicsWidget* m_emailsWidget;
};

#endif

