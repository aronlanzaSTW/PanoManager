//    PanoManager - Interactive panorama tour manager program
//    Copyright (C) 2018  Steve M Clarke
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

//
// Tour Properties Dialog
//

#ifndef TOURPROPERTIESDIALOG_H
#define TOURPROPERTIESDIALOG_H

#include <QDialog>
#include "../../project/project.h"

namespace Ui {
class TourPropertiesDialog;
}

class TourPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TourPropertiesDialog(QWidget *parent = 0);
    ~TourPropertiesDialog();
    void save(Project *proj) ;
    void load(Project *proj) ;

private:
    Ui::TourPropertiesDialog *ui;
};

#endif // TOURPROPERTIESDIALOG_H
