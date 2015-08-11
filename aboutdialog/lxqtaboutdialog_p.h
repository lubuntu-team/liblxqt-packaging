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

#ifndef LXQT_ABOUTDIALOG_P_H
#define LXQT_ABOUTDIALOG_P_H

#include "ui_lxqtaboutdialog.h"
#include <QDialog>

namespace LxQt
{

/**
 * @brief prepares the data to show and fills the form, then shows.
 */
class AboutDialogPrivate: public QDialog, public Ui_about
{
    Q_OBJECT

public:
    AboutDialogPrivate();
    QString titleText() const;
    QString aboutText() const;
    QString authorsText() const;
    QString thanksText() const;
    QString translationsText() const;

public slots:
    void copyToCliboardTechInfo();
};

} // namespace LxQt
#endif // LXQT_ABOUTDIALOG_P_H
