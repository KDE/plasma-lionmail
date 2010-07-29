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

#ifndef EMAILNOTIFIER_H
#define EMAILNOTIFIER_H

#include <QGraphicsItem>

#include <Plasma/PopupApplet>
//#include <plasma/dataengine.h>
#include <Plasma/ToolTipManager>
#include "ui_emailnotifierConfig.h"
#include "dialog.h"

namespace Plasma
{
    class Svg;
}

namespace Akonadi
{
    class EntityModelStateSaver;
}

class EmailNotifier : public Plasma::PopupApplet
{
  Q_OBJECT

    public:
        EmailNotifier(QObject* parent, const QVariantList &args);
        ~EmailNotifier();

        QGraphicsWidget* graphicsWidget();

        void init();
        void updateToolTip(const QString& statusText, const QString& icon);
        bool allowHtml();

    protected Q_SLOTS:
        void configAccepted();
        void configChanged();
        void statusChanged(int count, const QString& = QString());
        //void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private:
        Plasma::Svg* m_theme;
        Plasma::ToolTipContent m_toolTip;

        bool m_allowHtml;
        QHash<quint64, QString> m_allCollections;

        Ui::emailnotifierConfig* ui;
        QItemSelectionModel *m_checkSelection;
        Akonadi::EntityModelStateSaver *m_modelState;

        Dialog* m_dialog;

        quint64 m_collectionId;
        QList<quint64> m_newCollectionIds;
        QList<quint64> m_collectionIds;
};


#endif
