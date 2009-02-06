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
#include <QGraphicsGridLayout>
#include <QLabel>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KGlobalSettings>

//plasma
#include <Plasma/Applet>
#include <Plasma/Dialog>
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Extender>
//own
#include "mailextender.h"
#include "lionmail.h"
#include "emailmessage/emailmessage.h"


MailExtender::MailExtender(LionMail * applet, Plasma::Extender *ext)
    : Plasma::ExtenderItem(ext),
      m_info(0),
      m_icon(0),
      m_widget(0),
      m_label(0)
{
    m_applet = applet;
    //setParent(applet);
    setIcon("akonadi");
    setTitle("Lion Mail");
    setName("Lion Mail ExenderItem");
    (void)graphicsWidget();
    setMinimumHeight(200);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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
    | emailmessage ...          |
    +---------------------------+
    | emailmessage ...          |
    +---------------------------+
    | emailmessage ...          |
    +---------------------------+
    */

    int iconsize = 32;

    // Main widget and layout

    m_widget = new QGraphicsWidget(this);

    m_layout = new QGraphicsGridLayout(m_widget);
    m_layout->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_layout->setColumnFixedWidth(0, iconsize);
    m_layout->setColumnMinimumWidth(1, 100);
    m_layout->setHorizontalSpacing(0);

    // larger icon on the left
    m_icon = new Plasma::IconWidget(m_widget);
    //m_icon->setIcon(icon());
    m_icon->setIcon("akonadi");
    m_icon->resize(iconsize, iconsize);
    m_icon->setMinimumHeight(iconsize);
    m_icon->setMaximumHeight(iconsize);
    m_icon->setAcceptHoverEvents(false);
    m_layout->addItem(m_icon, 0, 0, 2, 1);

    // top label
    m_label = new Plasma::Label(m_widget);
    m_layout->addItem(m_label, 0, 1);

    // smaller label
    m_infoLabel = new Plasma::Label(m_widget);
    m_infoLabel->setText("33 emails");
    m_infoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_infoLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_infoLabel, 1, 1);

    m_messageLayout = new QGraphicsLinearLayout(m_layout);
    m_messageLayout->setSpacing(0);
    m_messageLayout->setOrientation(Qt::Vertical);
    m_messageLayout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
/*
    {
        EmailMessage *email = static_cast<EmailMessage*>(Plasma::Applet::load("emailmessage"));
        //email->setParent(this);
        email->setParentItem(m_widget);
        email->setBackgroundHints(Plasma::Applet::NoBackground);
        email->init();
        email->updateConstraints(Plasma::StartupCompletedConstraint);
        email->setMinimumHeight(120);
        email->setMinimumWidth(200);

        addEmail(email);
    }
*/
    m_layout->addItem(m_messageLayout, 3, 0, 1, 3);
    m_messageLayout->addStretch();
    m_messageLayout->addStretch();
    m_widget->setLayout(m_layout);

    setWidget(m_widget);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
    //resize(200, 400);
    return m_widget;
}

void MailExtender::addEmail(EmailMessage* email)
{
    email->setParent(this);
    email->setParentItem(m_widget);
    email->setBackgroundHints(Plasma::Applet::NoBackground);
    email->init();
    email->setPopupIcon(QIcon());
    email->m_emailWidget->setSmall();
    email->updateConstraints(Plasma::StartupCompletedConstraint);

    m_messageLayout->addItem(email->graphicsWidget());
    m_layout->setMinimumSize(m_messageLayout->sizeHint(Qt::MinimumSize));
    m_layout->setPreferredSize(m_messageLayout->sizeHint(Qt::PreferredSize));
    extender()->setMinimumSize(m_layout->sizeHint(Qt::PreferredSize));

    setMinimumSize(m_layout->sizeHint(Qt::MinimumSize));
    setPreferredSize(m_layout->sizeHint(Qt::PreferredSize));
    m_widget->resize(m_layout->sizeHint(Qt::MinimumSize));

    kDebug() << "--------------------------------------------------";
    kDebug() << "MessageLayout:" << m_messageLayout->sizeHint(Qt::MinimumSize)  << m_messageLayout->sizeHint(Qt::PreferredSize);
    kDebug() << "       Layout:" << m_layout->sizeHint(Qt::MinimumSize)         << m_layout->sizeHint(Qt::PreferredSize);
    kDebug() << "         Size:" << size();
    kDebug() << " ExtenderSize:" << extender()->size();
    kDebug() << "EmailWidgSize:" << email->m_emailWidget->geometry() << email->m_emailWidget->minimumSize();

    // Layouting magic, reassess when we depend on Qt 4.5
    //extender()->resize(m_layout->sizeHint(Qt::PreferredSize));
    m_messageLayout->updateGeometry();
    m_layout->updateGeometry();
    updateGeometry();
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
