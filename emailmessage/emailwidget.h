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

#ifndef EMAILWIDGET_H
#define EMAILWIDGET_H

//Qt
#include <QGraphicsGridLayout>
#include <QGraphicsWidget>

// KDE
#include <KPushButton>
#include <KJob>

// PIM & Akonadi
#include <Akonadi/Item>
#include <Akonadi/Monitor>

#include <kmime/kmime_message.h>

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<KMime::Message> MessagePtr;


// Plasma
#include <Plasma/Frame>
namespace Plasma
{
    class IconWidget;
    class Dialog;
    class Label;
    class WebView;
    class Frame;
}

//own
class EmailMessage;


class EmailWidget : public Plasma::Frame
{
    Q_OBJECT

    public:
        EmailWidget(EmailMessage* emailmessage, QGraphicsWidget *parent = 0);
        virtual ~EmailWidget();

        QGraphicsWidget* dialog();

        qlonglong id;

        void setFrom(const QString& from);
        void setTo(const QStringList& to);
        void setCc(const QStringList& ccList);
        void setBcc(const QStringList& bccList);
        void setFlags(const QStringList& flagList);

        void setSubject(const QString& subject);
        void setBody(MessagePtr msg);
        void setBody(const QString& body);
        void setAbstract(const QString& abstract);

        void setAllowHtml(bool allow);

        void setDate(const QDateTime& date);

        int widgetHeight(int size);
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
        void setLarge(bool expanded = false);

        void toggleBody();
        void toggleMeta();
        void expand();
        void collapse();
        //void sizeChanged();
        void updateColors();

        void fetchDone(KJob* job);
        void itemChanged(const Akonadi::Item* item);

    Q_SIGNALS:
        void geometryChanged(QSizeF newSize);

    private :
        Akonadi::Monitor* m_monitor;

        void updateSize(int h);

        bool m_fetching;
        void fetchPayload();
        bool m_expanded;

        void buildDialog();
        void resizeIcon(int iconsize);

        // The widget which displays in the panel
        //QGraphicsWidget* m_widget;
        QGraphicsGridLayout* m_layout;

        // The applet attached to this item
        EmailMessage* m_emailMessage;
        MessagePtr m_msg;

        int m_appletSize;
        // Email data
        QString m_subject;
        QDateTime m_date;

        QStringList m_to;
        QString m_from;
        QStringList m_cc;
        QStringList m_bcc;
        QStringList m_flags;

        bool m_isNew;
        bool m_isUnread;
        bool m_isImportant;
        bool m_isActionItem;

        QString m_body;
        QString m_abstract;
        QString m_stylesheet;
        bool m_allowHtml;
        bool m_showSmilies;

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

