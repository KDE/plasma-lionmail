/*
    Copyright 2008-2009 by Sebastian Kügler <sebas@kde.org>

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

//Qt
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QGraphicsGridLayout>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KGlobalSettings>

//plasm
#include <Plasma/Dialog>
#include <Plasma/Theme>
#include <Plasma/IconWidget>

//own
#include "mailextender.h"
#include "plasmobiff.h"


MailExtender::MailExtender(PlasmoBiff * applet, Plasma::Extender *ext)
    : Plasma::ExtenderItem(ext),
      m_widget(0),
      m_info(0),
      m_label(0),
      m_icon(0)
{
    m_applet = applet;
    setParent(applet);
    setIcon("akonadi");
    setTitle("jaMailca");
    setName("jaMailca Exenderitem");
    graphicsWidget();
}

MailExtender::~MailExtender()
{
}

QGraphicsWidget* MailExtender::graphicsWidget()
{
    /*
    +-----+---------------------+
    | m_i |  m_label            |
    | con |  m_infoLabel        |
    +-----+---------------------+
    */

    int iconsize = 32;

    // Main widget and layout

    m_widget = new QGraphicsWidget(this);

    QGraphicsGridLayout* layout = new QGraphicsGridLayout(m_widget);
    layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setColumnFixedWidth(0, iconsize);
    layout->setColumnMinimumWidth(1, 160);
    layout->setHorizontalSpacing(0);

    // larger icon on the left
    m_icon = new Plasma::IconWidget(m_widget);
    m_icon->setIcon(icon());
    m_icon->resize(iconsize, iconsize);
    m_icon->setMinimumHeight(iconsize);
    m_icon->setMaximumHeight(iconsize);
    m_icon->setAcceptHoverEvents(false);
    layout->addItem(m_icon, 0, 0, 2, 1);

    // top label
    m_label = new Plasma::Label(m_widget);
    layout->addItem(m_label, 0, 1);

    // smaller label
    m_infoLabel = new Plasma::Label(m_widget);
    m_infoLabel->setText("33 emails");
    m_infoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_infoLabel->nativeWidget()->setWordWrap(false);
    layout->addItem(m_infoLabel, 1, 1);


    m_widget->setLayout(layout);

    setWidget(m_widget);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
    return m_widget;
}

void MailExtender::setDescription(const QString& desc)
{
    if (m_label) {
        QString html = "<b>" + desc + "</b>";
        m_label->setText(html);
    }
    m_description = desc;
}

void MailExtender::setIcon(const QString& icon)
{
    if (m_icon) {
        m_icon->setIcon(icon);
    }
    ExtenderItem::setIcon(icon);
}

QString MailExtender::icon()
{
    return "akonadi"; // TODO: iconstring should be customizable
}

void MailExtender::setInfo(const QString& info)
{
    if (m_infoLabel) {
        m_infoLabel->setText(info);
    }
    m_info = info;
}

void MailExtender::updateColors()
{
    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    //m_widget->setPalette(p);
}

#include "mailextender.moc"
