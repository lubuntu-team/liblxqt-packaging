/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QProcess>
#include "lxqtscreensaver.h"
#include "lxqttranslator.h"

#include <XdgIcon>
#include <QMessageBox>
#include <QAction>

using namespace LxQt;

ScreenSaver::ScreenSaver(QObject * parent)
    : QObject(parent)
{
    m_xdgProcess = new QProcess(this);
    connect(m_xdgProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(xdgProcess_finished(int,QProcess::ExitStatus)));
}

QList<QAction*> ScreenSaver::availableActions()
{
    QList<QAction*> ret;
    QAction * act;

    act = new QAction(XdgIcon::fromTheme("system-lock-screen", "lock"), tr("Lock Screen"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(lockScreen()));
    ret.append(act);

    return ret;
}

void ScreenSaver::lockScreen()
{
    m_xdgProcess->start("xdg-screensaver", QStringList() << "lock");
}

void ScreenSaver::xdgProcess_finished(int err, QProcess::ExitStatus status)
{
    QWidget *p = qobject_cast<QWidget*>(parent());

    // http://portland.freedesktop.org/xdg-utils-1.1.0-rc1/scripts/xdg-screensaver

    if (err == QProcess::NormalExit) // QProcess::NormalExit = 0
    {
        emit activated();
    }
    else if (err == 1)
    {
        QMessageBox::warning(p,
                             tr("Screen Saver Error"),
                             tr("An error occurred starting screensaver. "
                                "Syntax error in xdg-screensaver arguments.")
                            );
    }
    else if (err == 3)
    {
        QMessageBox::warning(p,
                             tr("Screen Saver Activation Error"),
                             tr("An error occurred starting screensaver. "
                                "Ensure you have xscreensaver installed and running.")
                            );
    }
    else if (err == 4)
    {
        QMessageBox::warning(p,
                             tr("Screen Saver Activation Error"),
                             tr("An error occurred starting screensaver. "
                                "Action 'activate' failed. "
                                "Ensure you have xscreensaver installed and running.")
                            );
    }
    else
    {
        QMessageBox::warning(p,
                             tr("Screen Saver Activation Error"),
                             tr("An error occurred starting screensaver. "
                                "Unknown error - undocumented return value from xdg-screensaver: %1.").arg(err)
                            );
    }
    emit done();
}

