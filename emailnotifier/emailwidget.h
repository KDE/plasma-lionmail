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

#ifndef EMAILWIDGET_H
#define EMAILWIDGET_H

//Qt
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

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
#include <Plasma/Animation>
//#include <Plasma/Animation>

namespace Plasma
{
    //class Animation;
    class Dialog;
    class Frame;
    //class Animation;
    class IconWidget;
    class Label;
    class PopupApplet;
    class PushButton;
    class WebView;
}

//own
class EmailMessage;


class EmailWidget : public Plasma::Frame
{
    Q_OBJECT

    public:
        EmailWidget(QGraphicsWidget *parent = 0);
        virtual ~EmailWidget();

        QGraphicsWidget* dialog();

        qlonglong id;

        void setFrom(const QString& from);
        void setTo(const QStringList& to);
        void setCc(const QStringList& ccList);
        void setBcc(const QStringList& bccList);
        void setFlags(const QStringList& flagList);
        KUrl url();
        void setUrl(const KUrl);

        void setSubject(const QString& subject);
        void setBody(MessagePtr msg);
        void setRawBody(const QString& body);
        //void setAbstract(const QString& abstract);

        void setAllowHtml(bool allow);

        void setNew(bool isnew);
        void setImportant(bool important);
        //void setTask(bool task);

        bool isNew();
        void setSize(int appletsize);
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

        Plasma::PopupApplet* m_applet;

        static QString stripTags(QString input);

    public Q_SLOTS:
        //void setIcon();
        //void setTiny();
        void setSmall();
        //void setMedium();
        void setLarge(bool expanded = false);

        void toggleBody();
        //void toggleMeta();
        void expand();
        void collapse();

        void flagNewClicked();
        void deleteClicked();
        void flagImportantClicked();

        void updateColors();

        void fetchDone(KJob* job);
        void itemChanged(const Akonadi::Item& item);

    Q_SIGNALS:
        void geometryChanged(QSizeF newSize);
        void activated(const QUrl);
        void collapsed();

    protected:
        void wheelEvent (QGraphicsSceneWheelEvent * event);

    protected Q_SLOTS:
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    private Q_SLOTS:
        void syncItemResult(KJob* job);
        void showBody(bool show = true);
        void showActions(bool show = true);
        void showExpandIcon(bool show = true);
        void hideLater();
        void resizeLater();
        void itemActivated();
        void linkClicked(const QString &link);

    private :
        void syncItemToAkonadi(Akonadi::Item &item);
        Akonadi::Monitor* m_monitor;

        void updateSize(int h);
        void updateHeader();
        void startDrag();

        bool m_fetching;
        void fetchPayload(bool full = true);
        bool m_expanded;

        void buildDialog();
        void resizeIcon(int iconsize);
        void refreshFlags();
        void refreshFlags(bool show);

        // The applet attached to this item
        MessagePtr m_msg;
        Akonadi::Item m_item;

        int m_appletSize;
        // Email data
        QString m_subject;
        QDateTime m_date;
        KUrl m_url;
        QStringList m_to;
        QString m_from;
        QStringList m_cc;
        QStringList m_bcc;
        QStringList m_flags;

        bool m_isNew;
        bool m_isUnread;
        bool m_isImportant;
        bool m_isDeleted;
        bool m_flagsShown;

        QString m_body;
        //QString m_abstract;
        QString m_stylesheet;
        bool m_allowHtml;
        bool m_showSmilies;

        QGraphicsGridLayout* m_layout;
        QGraphicsWidget* m_actionsWidget;
        QGraphicsLinearLayout* m_actionsLayout;

        Plasma::IconWidget* m_icon;
        Plasma::IconWidget* m_expandIcon;
        Plasma::Label* m_subjectLabel;
        Plasma::Label* m_fromLabel;
        Plasma::Label* m_bodyWidget;

        Plasma::PushButton* m_newIcon;
        Plasma::PushButton* m_importantIcon;
        Plasma::PushButton* m_deleteButton;
        //Plasma::PushButton* m_unreadIcon;
        
        Plasma::Animation* m_expandIconAnimation;
        Plasma::Animation* m_actionsAnimation;
        Plasma::Animation* m_bodyAnimation;

        //Plasma::WebView* m_bodyView;

        QPointF m_startPos;
        int m_fontAdjust; // wheel adjustment of the font size
};

#endif

