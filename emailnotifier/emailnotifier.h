/*
    Copyright 2010 by Sebastian Kügler <sebas@kde.org>

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

#ifndef EMAILNOTIFIER_H
#define EMAILNOTIFIER_H

#include <Plasma/PopupApplet>

#include "ui_emailnotifierConfig.h"

class Dialog;
class KJob;

namespace Plasma
{
    class Svg;
    class ToolTipContent;
}

class EmailNotifier : public Plasma::PopupApplet
{
  Q_OBJECT

    enum ImportantDisplay { None = 0, ShowMerged = 1, ShowSeparately = 2 };
    Q_DECLARE_FLAGS(ImportantDisplays, ImportantDisplay)

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


    private Q_SLOTS:
        void findDefaultCollectionsDone(KJob* job);

    private:
        void findDefaultCollections();

        Plasma::Svg* m_theme;
        Plasma::ToolTipContent m_toolTip;

        bool m_allowHtml;
        ImportantDisplay m_showImportant;
        QHash<Akonadi::Entity::Id, QString> m_allCollections;

        Ui::emailnotifierConfig* ui;
        QItemSelectionModel *m_checkSelection;
        KCheckableProxyModel *m_checkable;

        Dialog* m_dialog;

        QList<Akonadi::Entity::Id> m_newCollectionIds;
        QList<Akonadi::Entity::Id> m_collectionIds;

        bool m_hasFullPayload;
};


#endif
