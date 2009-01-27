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
#include <QWebPage>
#include <QLabel>
//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KGlobalSettings>

//plasma
#include <Plasma/Dialog>
#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/WebView>

// own
#include "emailmessage.h"
#include "emaildialog.h"

using namespace Plasma;

EmailDialog::EmailDialog(EmailMessage* emailmessage, QObject *parent)
    : QObject(parent),
      m_widget(0)
{
    m_showBody = false;
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
    int iconsize = 32;

    m_widget = new QGraphicsWidget();
    m_widget->setParent(this);

    m_layout = new QGraphicsGridLayout(m_widget);
    m_layout->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_layout->setColumnFixedWidth(0, iconsize);
    m_layout->setColumnMinimumWidth(1, 180);
    m_layout->setColumnFixedWidth(2, 22);
    m_layout->setHorizontalSpacing(4);


    m_icon = new Plasma::IconWidget(m_widget);
    m_icon->setIcon("view-pim-mail");
    m_icon->resize(iconsize, iconsize);
    m_icon->setMinimumHeight(iconsize);
    m_icon->setMaximumHeight(iconsize);
    m_icon->setAcceptHoverEvents(false);

    m_layout->addItem(m_icon, 0, 0, 2, 1);

    m_subjectLabel = new Plasma::Label(m_widget);
    m_subjectLabel->setText("<b>Re: sell me a beer, mon</b>");
    m_subjectLabel->setMaximumHeight(iconsize/2);
    m_layout->addItem(m_subjectLabel, 0, 1);

    m_toLabel = new Plasma::Label(m_widget);
    m_toLabel->setText("<b>Recipient:</b> Bob Marley <bmarley@kde.org>");
    m_toLabel->setMaximumHeight(iconsize/2);
    m_toLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_toLabel->nativeWidget()->setWordWrap(false);

    m_layout->addItem(m_toLabel, 1, 1);

    m_body = new Plasma::WebView(m_widget);
    QString html("<font color=white>Hi everybody<br /><br />I hope you're all having a great time on Jamaica, my home country (which you might have noticed after yesterday's bob-marley-on-repeat-for-the-whole-night. I wish I could be with you, but it wasn't meant to be. I'll go out with my friends Kurt and Elvis tonight instead and wish you a happy CampKDE.<br /><br />-- Bob</font>");

    m_body->setHtml(html);
    m_layout->addItem(m_body, 3, 0, 1, 3);

    m_expandIcon = new Plasma::IconWidget(m_widget);
    m_expandIcon->setIcon("arrow-down-double");
    m_expandIcon->setMinimumSize(12, 12);
    connect(m_expandIcon, SIGNAL(clicked()), this, SLOT(toggleBody()));
    m_layout->addItem(m_expandIcon, 1, 2, Qt::AlignRight);

    m_widget->setLayout(m_layout);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
    kDebug() << "EmailDialog built";
}

void EmailDialog::toggleBody()
{
    if (!m_showBody) {
        showBody();
    } else {
        hideBody();
    }
}

void EmailDialog::hideBody()
{
    m_body->hide();
    m_expandIcon->setIcon("arrow-down-double");
    m_showBody = false;
    kDebug() << "hiding body";
    m_layout->invalidate();
}

void EmailDialog::showBody()
{
    m_widget->setMinimumHeight(250);
    m_body->show();
    m_expandIcon->setIcon("arrow-up-double");
    m_showBody = true;
    kDebug() << "showing body";
    m_layout->invalidate();
}

void EmailDialog::updateColors()
{
    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    //p.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    p.setColor(QPalette::Text, QColor("white"));


    m_widget->setPalette(p);
    m_body->page()->setPalette(p);
}

void EmailDialog::setSubject(const QString& subject)
{
    //m_subject = subject;
    m_subjectLabel->setText(subject);
    // TODO: update widgets...
}

void EmailDialog::setTo(const QString& to)
{
    kDebug() << "Setting recipient" << to;
    if (!to.isEmpty()) {
        kDebug() << "TO IS NONEMPTY!!!!!!!!!!!!!!!!!!!!!!!!!!";
        m_toLabel->setText(to);
    }
}

void EmailDialog::setBody(const QString& body)
{
    // TODO
}

void EmailDialog::setAbstract(const QString& abstract)
{
    // TODO
}

void EmailDialog::setDate(const QDate& date)
{
    // TODO
}

void EmailDialog::setFrom(const QString& fromList)
{
    // TODO
}

void EmailDialog::setCc(const QString& ccList)
{
    // TODO
}

void EmailDialog::setBcc(const QString& bccList)
{
    // TODO
}


#include "emaildialog.moc"
