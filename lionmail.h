/***************************************************************************
 *   Copyright 2007 by Thomas Moenicke <thomas.moenicke@kdemail.net>       *
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
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

#ifndef LIONMAIL_H
#define LIONMAIL_H

#include <QPainter>
#include <QGraphicsItem>

#include <plasma/popupapplet.h>
#include <plasma/dataengine.h>
#include <Plasma/ToolTipManager>
#include "ui_lionmailConfig.h"

class MailExtender;
class EmailMessage;
namespace Plasma
{
    class Svg;
}

class LionMail : public Plasma::PopupApplet
{
  Q_OBJECT

    public:
        LionMail(QObject* parent, const QVariantList &args);
        ~LionMail();

        void init();
        //void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect );

        void initExtenderItem();
        void updateToolTip(const QString query, const int matches);

        //void initData();


    public Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

    protected Q_SLOTS:
        void configAccepted();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private:
        //void drawEmail(int index, const QRectF& rect, QPainter* painter) ;

    private Q_SLOTS:
        void newSource( const QString &source );

    private:
        QStringList m_collections;
        QString m_activeCollection;
        QHash<QString, EmailMessage*> emails;
        int m_maxEmails;
        QList<MailExtender*> m_extenders;
        Plasma::Svg* m_theme;
        Plasma::DataEngine *engine;
        Plasma::ToolTipContent m_toolTip;

        Ui::lionmailConfig ui;

        QFont m_fontFrom;
        QFont m_fontSubject;

        QMap<int,QString> m_fromList;
        QMap<int,QString> m_subjectList;
};


#endif
