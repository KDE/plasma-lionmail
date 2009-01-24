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

#include "lionmail.h"

#include <QPainter>
#include <QWidget>
#include <QGraphicsItem>

#include <KConfigDialog>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/Extender>
#include <Plasma/DataEngine>

#include "mailextender.h"
#include "emailmessage/emailmessage.h"

LionMail::LionMail(QObject *parent, const QVariantList &args)
  : Plasma::PopupApplet(parent, args)
{
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/akonadi");
    m_theme->setContainsMultipleImages(false);
    setHasConfigurationInterface(true);

    m_subjectList[0] = QString("Hello CampKDE, hallo Jamaica!"); // ;-)
    setBackgroundHints(StandardBackground);

    m_fontFrom = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    m_fontSubject = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    setPopupIcon("akonadi");

    m_maxEmails = 6;
}

void LionMail::init()
{
    engine = dataEngine("akonadi");
    engine->connectAllSources(this);
    connect(engine, SIGNAL(sourceAdded(QString)), SLOT(newSource(QString)));
    setMinimumSize(600, 300);
    m_theme->resize(600, 300);
    resize(600, 300); // move to constraintsevent
    extender()->setEmptyExtenderMessage(i18n("empty..."));
    initExtenderItem();
    initData();
    updateToolTip("", 0);
}

LionMail::~LionMail()
{
}

void LionMail::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);

    //connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    //connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void LionMail::initExtenderItem()
{
    MailExtender* mailView = new MailExtender(this, extender());
    //mailView->setIcon("view-pim-mail");
    mailView->setDescription("Private Emails"); // FIXME: sample text
    mailView->setInfo("2 unread");

/*
    MailExtender* mailView2 = new MailExtender(this, extender());
    mailView2->setIcon("mail-send");
    mailView2->setDescription("Received Today"); // FIXME: sample text
    mailView2->setInfo("14 emails, 3 unread");

    m_extenders << mailView2 << mailView;
*/
    m_extenders << mailView;
}

void LionMail::updateToolTip(const QString query, const int matches)
{

    m_toolTip = Plasma::ToolTipContent(i18nc("No search has been done yet", "Crystal Desktop Search"),
            i18nc("Tooltip sub text", "Click on the icon to monitor your emails"),
                    KIcon("akonadi").pixmap(IconSize(KIconLoader::Desktop))
                );
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}


/*
// TODO: Maybe reimplement it, showing some number
void LionMail::paintInterface(QPainter * painter, const QStyleOptionGraphicsItem * option, const QRect &contentsRect)
{
    Q_UNUSED( option )
    Q_UNUSED( contentsRect )

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    // draw the main background stuff
    m_theme->paint(painter, boundingRect(), "email_frame");

    // Use layouts?
    QRectF bRect = boundingRect();
    bRect.setX(bRect.x()+93);

    // draw the 4 channels
    bRect.setY(bRect.y()+102);
    drawEmail(0, bRect, painter);

    bRect.setY(bRect.y()-88);
    drawEmail(1, bRect, painter);

    bRect.setY(bRect.y()-88);
    drawEmail(2, bRect, painter);

    bRect.setY(bRect.y()-88);
    drawEmail(3, bRect, painter);
}


void LionMail::drawEmail(int index, const QRectF& rect, QPainter* painter)
{

    QPen _pen = painter->pen();
    QFont _font = painter->font();

    painter->setPen(Qt::white);

    QString from = m_fromList[index];
    // cut if too long
    if(from.size() > 33) {
        from.resize(30);
        from.append("...");
    }
    //  qDebug() << m_fontFrom.family() << m_fontFrom.pixelSize() << m_fontFrom.pointSize();

    QFontMetrics fmFrom(m_fontFrom);
    QFontMetrics fmSubject(m_fontFrom);

    painter->setFont(m_fontFrom);
    painter->drawText((int)(rect.width()/2 - fmFrom.width(from) / 2),
                (int)((rect.height()/2) - fmFrom.xHeight()*3),
                from);

    QString subject = m_subjectList[index];
    // cut
    // how about KSqueezedLabel?
    if (subject.size() > 33) {
        subject.resize(30);
        subject.append("...");
    }

    painter->setFont(m_fontSubject);
    painter->drawText((int)(rect.width()/2 - fmSubject.width(subject) / 2),
                (int)((rect.height()/2) - fmSubject.xHeight()*3 + 15),
                subject);

    // restore
    painter->setFont(_font);
    painter->setPen(_pen);

}
*/
void LionMail::initData()
{
    const QStringList& sources = dataEngine("akonadi")->query("Subject")["sources"].toStringList();
    kDebug() << "Source" << sources;
    foreach (const QString &source, sources) {
        //kDebug() << "BatterySource:" << battery_source;
        dataUpdated(source, dataEngine("akonadi")->query(source));
    }

}

void LionMail::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED( source );
    //  EmailMessage *email = static_cast<EmailMessage*>(Plasma::Applet::load("emailmessage"));

    kDebug() << "Source" << source;
    if (emails.count() < m_maxEmails && !emails.contains(source)) {
        EmailMessage *email = static_cast<EmailMessage*>(Plasma::Applet::load("emailmessage"));

        // Only set email-specific properties here, layouttweaks and the like should go into MailExtender
        email->m_dialog->setSubject(data["Subject"].toString());
        email->m_dialog->setFrom(data["From"].toString());
        email->m_dialog->setTo(data["To"].toString());
        email->m_dialog->setCc(data["Cc"].toString());
        email->m_dialog->setBcc(data["Bcc"].toString());

        m_extenders[0]->addEmail(email); // FIXME: hardcoded, we need to find a way to select the right extender
        emails[source] = email;
    }
    //mailView->addEmail(email);

    m_fromList[0] = data["From"].toString();
    m_subjectList[0] = data["Subject"].toString();

    update();
}

void LionMail::newSource(const QString & source)
{
    kDebug() << "------------- New:" << source;
    engine->connectSource(source, this);
    // We could create MailExtenders here ...

}

K_EXPORT_PLASMA_APPLET(lionmail, LionMail)

#include "lionmail.moc"