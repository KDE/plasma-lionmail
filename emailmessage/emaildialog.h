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

        Plasma::IconWidget* m_expandIcon;
        Plasma::IconWidget* m_icon;
        Plasma::Label* m_subjectLabel;
        Plasma::Label* m_toLabel;
        Plasma::WebView* m_body;

        long id;

        void setTo(const QString& to);
        void setFrom(const QString& fromList);
        void setCc(const QString& ccList);
        void setBcc(const QString& bccList);
        void setFlags(const QString& flagList);

        void setSubject(const QString& subject);
        void setBody(const QString& body);
        void setAbstract(const QString& abstract);

        void setDate(const QDate& date);


    public Q_SLOTS:

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
        EmailMessage* m_emailmessage;

};

#endif

