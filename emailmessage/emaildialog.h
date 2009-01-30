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

#ifndef EMAILDIALOG_H
#define EMAILDIALOG_H

//Qt
#include <QGraphicsGridLayout>

// KDE
#include <KPushButton>

//own
class EmailMessage;

//desktop view
namespace Plasma
{
    class IconWidget;
    class Dialog;
    class Label;
    class WebView;
}

class EmailDialog : public QObject
{
    Q_OBJECT

    public:
        EmailDialog(EmailMessage* emailmessage, QObject *parent = 0);
        virtual ~EmailDialog();

        QGraphicsWidget* dialog();

        long id;

        void setFrom(const QString& from);
        void setTo(const QStringList& to);
        void setCc(const QStringList& ccList);
        void setBcc(const QStringList& bccList);
        void setFlags(const QStringList& flagList);

        void setSubject(const QString& subject);
        void setBody(const QString& body);
        void setAbstract(const QString& abstract);

        void setDate(const QDate& date);

        enum AppletSize {
            Icon = 0,
            Tiny = 1,
            Small = 2,
            Medium = 4,
            Large = 8
        };
        Q_DECLARE_FLAGS(AppletSizes, AppletSize)

    public Q_SLOTS:
        void setIcon();
        void setTiny();
        void setSmall();
        void setMedium();
        void setLarge();

        void toggleBody();
        void showBody();
        void hideBody();
        void updateColors();

    private :
        bool m_showBody;

        void buildDialog();

        // The widget which displays in the panel
        QGraphicsWidget* m_widget;
        QGraphicsGridLayout* m_layout;

        // The applet attached to this item
        EmailMessage* m_emailMessage;

        // Email data
        QString m_subject;
        QDate m_date;

        QStringList m_to;
        QString m_from;
        QStringList m_cc;
        QStringList m_bcc;
        QStringList m_flags;

        QString m_body;
        QString m_abstract;

        Plasma::IconWidget* m_expandIcon;
        Plasma::IconWidget* m_icon;
        Plasma::Label* m_subjectLabel;
        Plasma::Label* m_toLabel;
        Plasma::Label* m_fromLabel;
        Plasma::Label* m_ccLabel;
        Plasma::Label* m_bccLabel;
        Plasma::Label* m_dateLabel;
        Plasma::WebView* m_bodyView;
        Plasma::Label* m_abstractLabel;

};

#endif

