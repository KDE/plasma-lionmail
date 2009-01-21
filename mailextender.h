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
#include <QLabel>
#include <QStringList>

// Plasma
#include <Plasma/Label>
#include <Plasma/ExtenderItem>

// own
#include "plasmobiff.h"

class MailExtender : public Plasma::ExtenderItem
{
    Q_OBJECT

    public:
        MailExtender(PlasmoBiff * applet, Plasma::Extender *ext = 0);
        virtual ~MailExtender();

        QGraphicsWidget* graphicsWidget();
        void setIcon(const QString& icon);

        void setDescription(const QString& desc);
        void setInfo(const QString& info);

    private slots:
        void updateColors();

    private:
        void buildDialog();

        QString m_description;
        QString m_info;

        PlasmoBiff* m_applet;

        Plasma::IconWidget* m_icon;
        QGraphicsWidget* m_widget;
        Plasma::Label* m_label;
        Plasma::Label* m_infoLabel;
};

#endif

