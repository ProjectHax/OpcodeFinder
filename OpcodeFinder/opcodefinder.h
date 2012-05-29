#pragma once

#ifndef OPCODEFINDER_H
#define OPCODEFINDER_H

#include <QtGui/QWidget>
#include "ui_opcodefinder.h"

class OpcodeFinder : public QWidget
{
	Q_OBJECT

private:

	//User interface
	Ui::OpcodeFinderClass ui;

public:

	//Constructor
	OpcodeFinder(QWidget *parent = 0, Qt::WFlags flags = 0);

	//Destructor
	~OpcodeFinder();


};

#endif