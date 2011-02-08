/*
    Copyright 2008-2010 by Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef EMAILWIDGET_H
#define EMAILWIDGET_H

//Qt
class QGraphicsAnchorLayout;
class QGraphicsGridLayout;
class QGraphicsLinearLayout;
class QGraphicsWidget;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

// KDE
class KPushButton;
class KJob;

// PIM & Akonadi
namespace Akonadi
{
    //class Item;
    class Monitor;
    class Session;
    //class MessageStatus;
    //class MessagePart;
}

#include <Akonadi/Item>
#include <kmime/kmime_message.h>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/kmime/messagestatus.h>

#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<KMime::Message> MessagePtr;

// Plasma
#include <Plasma/Frame>

namespace Plasma
{
    class Animation;
    class Dialog;
    class IconWidget;
    class Label;
    class PopupApplet;
    class PushButton;
    class WebView;
}

class EmailWidget : public Plasma::Frame
{
    Q_OBJECT

    public:
        EmailWidget(QGraphicsWidget *parent = 0);
        virtual ~EmailWidget();

        QGraphicsWidget* dialog();
        Akonadi::Item item();
        Akonadi::MessageStatus status();

        Akonadi::Entity::Id id;

        void setFrom(const QString& from);
        void setTo(const QStringList& to);
        void setFlags(const QStringList& flagList);
        KUrl url();
        void setUrl(const KUrl);

        void setSubject(const QString& subject);
        void setBody(MessagePtr msg);
        void setRawBody(const QString& body);
        //void setAbstract(const QString& abstract);

        void setAllowHtml(bool allow);

        //void setDeleted(bool deleted = true);

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

        static QString stripTags(QString input);
        static QString abstract(QString body);

    public Q_SLOTS:
        void setSmall();
        void setLarge(bool expanded = false);

        void toggleBody();
        void expand();
        void collapse();

        void setDeleted(bool deleted = true, bool noSync = false);
        void setSpam(bool spam);
        void setTask(bool task);

        void flagNewClicked();
        void deleteClicked();
        void setSpamClicked(bool checked);
        void setTaskClicked(bool checked);
        void flagImportantClicked();

        void updateColors();

        void fetchDone(KJob* job);
        void itemChanged(const Akonadi::Item& item);

    Q_SIGNALS:
        void geometryChanged(QSizeF newSize);
        void activated(const QUrl);
        void collapsed();
        void deleteMe(); /**< emitted once the disappearAnimation is done and the item can go away **/

    protected:
        void wheelEvent (QGraphicsSceneWheelEvent * event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

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
        void disappearAnimationFinished();
        void spamAnimationFinished();

    private :
        void syncItemToAkonadi();
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

        bool m_isDeleted;

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

        Akonadi::MessageStatus m_status;

        bool m_flagsShown;

        QString m_body;
        QString m_stylesheet;
        bool m_allowHtml;
        bool m_showSmilies;

        QGraphicsAnchorLayout* m_anchorLayout;
        QGraphicsGridLayout* m_layout;
        QGraphicsWidget* m_actionsWidget;
        QGraphicsLinearLayout* m_actionsLayout;

        QGraphicsWidget* m_emailWidget;

        Plasma::IconWidget* m_icon;
        Plasma::IconWidget* m_expandIcon;
        Plasma::Label* m_subjectLabel;
        Plasma::Label* m_fromLabel;
        Plasma::Label* m_bodyWidget;

        Plasma::PushButton* m_newIcon;
        Plasma::PushButton* m_importantIcon;
        Plasma::PushButton* m_deleteButton;

        Plasma::Animation* m_expandIconAnimation;
        QPropertyAnimation* m_actionsAnimation;
        Plasma::Animation* m_bodyAnimation;
        Plasma::Animation* m_disappearAnimation;
        QPropertyAnimation* m_spamAnimation;

        QPointF m_startPos;
        int m_fontAdjust; // wheel adjustment of the font size
        bool m_hasFullPayload;
        bool m_noSync;
};

#endif

