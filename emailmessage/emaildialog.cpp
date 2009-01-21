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
#include <KIconLoader>
#include <KMimeType>
#include <KRun>

//plasma
#include <Plasma/Dialog>
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>

// own
#include "emailmessage.h"
#include "emaildialog.h"

using namespace Plasma;

EmailDialog::EmailDialog(EmailMessage* emailmessage, QObject *parent)
    : QObject(parent),
      m_widget(0)
{
    buildDialog();
}

EmailDialog::~EmailDialog()
{
}

QGraphicsWidget* EmailDialog::dialog()
{
    return m_widget;
}

void EmailDialog::buildDialog()
{
    m_widget = new QGraphicsWidget();

    QGraphicsGridLayout *layout = new QGraphicsGridLayout(m_widget);

    m_icon = new Plasma::IconWidget(m_widget);
    m_icon->setIcon("view-pim-mail");
    layout->addItem(m_icon, 0, 0, 2, 1);

    m_subjectLabel = new Plasma::Label(m_widget);
    m_subjectLabel->setText("Re: sell me a beer mon");
    layout->addItem(m_subjectLabel, 0, 1);

    m_toLabel = new Plasma::Label(m_widget);
    m_toLabel->setText("bmarley@kde.org");
    layout->addItem(m_toLabel, 1, 1);


    m_widget->setLayout(layout);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
}

void EmailDialog::updateColors()
{
    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    m_widget->setPalette(p);
}

#include "emaildialog.moc"
