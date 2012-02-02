/*
QHardwareKontrol is part of the openGreenHouse package of software sources

Written by: Travis McCann
Copywrite GPL 2010-2012

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "target.h"
#include "ui_target.h"

Target::Target(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Target)
{
    ui->setupUi(this);
    relayID = 0; //init to invalid
}

Target::Target(QWidget *parent, int id, int md) :
    QWidget(parent),
    ui(new Ui::Target)
{
    ui->setupUi(this);
    relayID = id;
    mode = md;   //mode 0 = temps; 1 = humidity; 2 = c02

    this->on_revertBtn_clicked();  //read values from database
}

Target::~Target()
{
    delete ui;
}

void Target::setTitle(QString label) {
    ui->groupBox->setTitle(label);
}

void Target::setLabel(QString label) {
    ui->label->setText(label);
}

QString Target::check() {

    QString cmd = relayID.toString();

    float target, val;
    QSqlQuery q;
    q.prepare("SELECT target FROM Target WHERE relayID = :relayID");
    q.bindValue(":relayID",relayID);
    q.exec();
    while(q.next()) {
        target = q.value(0).toFloat();
    }

    switch(mode.toInt()){
    case 0:
        q.prepare("SELECT temp FROM Temps ORDER BY time DESC LIMIT 1");
        q.exec();
        while (q.next()) {
            val = q.value(0).toFloat();     //average the two sensors in the room
            val += q.value(1).toFloat();
            val = val/2.0;
        }
        if( ((val + pval)/2.0) < target) {   //average this reading with the last
            cmd.append(":off.");
        } else {
            cmd.append(":on.");
        }
        break;

    case 1:
        q.prepare("SELECT humid FROM Humid ORDER BY time DESC LIMIT 1");
        q.exec();
        while(q.next()) {
            val = q.value(0).toFloat();
        }
        if (((val+pval)/2) < target) {
            cmd.append(":off.");
        } else {
            cmd.append(":on.");
        }
        break;

    case 2:
        break;
    }
    pval = val;  //set previous value for next run
    return cmd;
}

bool Target::confirmChange() {
    bool ret;
    QMessageBox msgBox;
    QString txt = "Relay " + relayID.toString();
    txt.append(" has been modified.");
    msgBox.setText(txt);
    msgBox.setInformativeText("Are you sure you want to change relay " + relayID.toString() + " ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    if( msgBox.exec() == QMessageBox::Yes) {
        ret = true;
    } else {
        ret = false;
    }
    return ret;
}


void Target::on_updateBtn_clicked()
{
    if(confirmChange()) {
        QSqlQuery q;
        q.prepare("UPDATE Target SET target = :target WHERE relayID = :relayID");
        q.bindValue(":target",ui->spinBox->value());
        q.bindValue(":relayID", relayID);
        if(!q.exec()) {
            //query failed print error message here...
        }
    }
}

void Target::on_revertBtn_clicked()
{
    QSqlQuery q;
    q.prepare("SELECT target FROM Target WHERE relayID = :relayID");
    q.bindValue(":relayID",relayID);
    q.exec();
    while(q.next()){
        ui->spinBox->setValue(q.value(0).toInt());
    }
}
