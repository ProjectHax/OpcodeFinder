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

	//Extracts opcodes out of the client
	QStringList ExtractOpcodes(QString path);

private slots:

	//Saves a formatted list of opcodes
	void Save();

	//Searches for an opcode
	void Search();

	//Opens the first client
	void Open1();

	//Opens the second client
	void Open2();

	//Opcode row changed
	void RowChanged(int index) { ui.lstServer1->setCurrentRow(index); ui.lstClient1->setCurrentRow(index); ui.lstServer2->setCurrentRow(index); ui.lstClient2->setCurrentRow(index); }

public:

	//Constructor
	OpcodeFinder(QWidget *parent = 0, Qt::WFlags flags = 0);

	//Destructor
	~OpcodeFinder();

};

#endif