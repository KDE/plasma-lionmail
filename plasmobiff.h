/***************************************************************************
 *   Copyright (C) 2007 by Thomas Moenicke                                 *
 *   thomas.moenicke@kdemail.net                                           *
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

#ifndef PLASMOBIFF_H
#define PLASMOBIFF_H

#include <QImage>
#include <QPaintDevice>
#include <QLabel>
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>
#include <QX11Info>
#include <QWidget>
#include <QGraphicsItem>
#include <QColor>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include "ui_plasmobiffConfig.h"

class KDialog;

namespace Plasma
{
  class Svg;
}

class PlasmoBiff : public Plasma::Applet
{
  Q_OBJECT

 public:
    PlasmoBiff(QObject* parent, const QStringList &args);
    ~PlasmoBiff();

    void constraintsUpdated();
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect );
    QRectF boundingRect() const;

  public slots:
    void configureDialog();
    void updated(const QString &source, const Plasma::DataEngine::Data &data);

  private:
    void drawEmail(int index, const QRectF& rect, QPainter* painter) ;

  private slots:
    void newSource( const QString &source );

  private:
    Plasma::Svg* m_theme;
    Plasma::DataEngine *engine;
    QRectF m_bounds;
    int m_xPixelSize;
    int m_yPixelSize;

    KDialog *m_dialog;
    Ui::plasmobiffConfig ui;

    QFontMetrics m_fmFrom;
    QFontMetrics m_fmSubject;

    QFont m_fontFrom;
    QFont m_fontSubject;

    QMap<int,QString> m_fromList;
    QMap<int,QString> m_subjectList;
};

K_EXPORT_PLASMA_APPLET(plasmobiff, PlasmoBiff)

#endif
