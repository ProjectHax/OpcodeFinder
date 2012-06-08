#include "opcodefinder.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QInputDialog>

#include <stdint.h>
#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <set>

//Constructor
OpcodeFinder::OpcodeFinder(QWidget *parent, Qt::WFlags flags) : QWidget(parent, flags)
{
	//Initializes the user interface
	ui.setupUi(this);

	//Connect the menu bar items
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(Save()));
	connect(ui.actionSearch, SIGNAL(triggered()), this, SLOT(Search()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
}

//Destructor
OpcodeFinder::~OpcodeFinder()
{
}

//Extracts opcodes out of the client
QStringList OpcodeFinder::ExtractOpcodes(QString path)
{
	QStringList opcodes;
	QFile file(path);

	//Open the file
	if(file.open(QIODevice::ReadOnly))
	{
		//Read the file data and convert it to hex
		std::string data = QString(file.readAll().toHex()).toUpper().toAscii().data();
		file.close();

		if(data.empty())
		{
			//No data read
			QMessageBox::critical(this, "Error", "No data was read from the client.");
		}
		else
		{
			//Regex string for locating opcodes
			const static boost::regex e("C744240C(?<opcode>....)0000C7442410........E8........8D|8D..24108D..2418....8BCEC7442418(?<opcode>....)0000C744241C........E8|8B4E10C7442410(?<opcode>....)000041C74424..........518BCEE8");

			boost::match_results<std::string::const_iterator> what;
			boost::match_flag_type flags = boost::match_default;

			std::string::const_iterator start = data.begin();
			std::string::const_iterator end = data.end();

			try
			{
				//Search
				while(boost::regex_search(start, end, what, e, flags))
				{
					//Get the opcode
					std::string temp = what["opcode"];

					//Add it to the list
					opcodes.append((temp.substr(2, 2) + temp.substr(0, 2)).c_str());

					//Next
					start = what[0].second;
					flags |= boost::match_prev_avail;
					flags |= boost::match_not_bob;
				}
			}
			catch(std::exception & e)
			{
				//Display error message (regular expression is probably outdated for this client)
				QMessageBox::critical(this, "Error", e.what());
			}
		}
	}
	else
	{
		//File open error
		QMessageBox::critical(this, "Error", QString("Unable to open %0 for reading. Could not extract opcodes.").arg(path));
	}

	return opcodes;
}

//Saves a formatted list of opcodes
void OpcodeFinder::Save()
{
	if(ui.lstServer1->count())
	{
		//Ask the user where to save the file
		QString path = QFileDialog::getSaveFileName(this, "Locate sro_client.exe", "", "Text Documents (*.txt)");

		//Open the file
		QFile file(path);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QMessageBox::critical(this, "Error", QString("Unable to open %0 for writing.").arg(path));
			return;
		}

		//Used for writing text to the file
		QTextStream out(&file);

		if(ui.lstServer2->count())
		{
			if(ui.lstServer1->count() == ui.lstServer2->count())
			{
				out << "S->C\n";

				for(int x = 0; x < ui.lstServer1->count(); ++x)
				{
					QString opcode1 = ui.lstServer1->item(x)->text();
					QString opcode2 = ui.lstServer2->item(x)->text();

					out << opcode1 << "    ->    " << opcode2 << "\n";
				}

				out << "\nC->S\n";

				for(int x = 0; x < ui.lstClient1->count(); ++x)
				{	
					QString opcode1 = ui.lstClient1->item(x)->text();
					QString opcode2 = ui.lstClient2->item(x)->text();

					if(!opcode1.isEmpty() && !opcode2.isEmpty())
					{
						out << opcode1 << "    ->    " << opcode2 << "\n";
					}
				}
			}
			else
			{
				//Output path
				out << ui.txtPath1->text() << "\n";

				for(int x = 0; x < ui.lstServer1->count(); ++x)
				{
					//Output server opcode
					out << ui.lstServer1->item(x)->text();

					//See if there is a matching client opcode
					if(!ui.lstClient1->item(x)->text().isEmpty())
						out << "    ->    " << ui.lstClient1->item(x)->text();

					//New line
					out << "\n";
				}

				//Output path
				out << "\n" << ui.txtPath2->text() << "\n";

				for(int x = 0; x < ui.lstServer2->count(); ++x)
				{
					//Output server opcode
					out << ui.lstServer2->item(x)->text();

					//See if there is a matching client opcode
					if(!ui.lstClient2->item(x)->text().isEmpty())
						out << "    ->    " << ui.lstClient2->item(x)->text();

					//New line
					out << "\n";
				}
			}
		}
		//Only need to save one set of opcodes
		else
		{
			//Output path
			out << ui.txtPath1->text() << "\n";

			for(int x = 0; x < ui.lstServer1->count(); ++x)
			{
				//Output server opcode
				out << ui.lstServer1->item(x)->text();

				//See if there is a matching client opcode
				if(!ui.lstClient1->item(x)->text().isEmpty())
					out << "    ->    " << ui.lstClient1->item(x)->text();

				//New line
				out << "\n";
			}
		}

		//Close the file
		file.close();
	}
}

//Opens the first client
void OpcodeFinder::Open1()
{
	QString path = QFileDialog::getOpenFileName(this, "Locate sro_client.exe", "", "sro_client (sro_client.exe)");

	if(!path.isEmpty())
	{
		//Display the client path
		ui.txtPath1->setText(path);

		//Clear current opcodes
		ui.lstServer1->clear();
		ui.lstClient1->clear();

		//Extract opcodes
		QStringList opcodes = ExtractOpcodes(path);

		if(!opcodes.size())
		{
			QMessageBox::critical(this, "Error", "There was a problem extracting opcodes out of the client.");
		}
		else
		{
			for(int x = 0; x < opcodes.size(); ++x)
			{
				QString & opcode = opcodes[x];

				//Add the opcode to the list
				ui.lstServer1->addItem(opcode);

				//If the opcode starts with 'B' then it also has a client opcode that starts with '7'
				if(!opcode.isEmpty() && opcode[0] == 'B')
				{
					ui.lstClient1->addItem("7" + opcode.mid(1));
				}
				else
				{
					//Does not start with 'B' so add something else to fill in the gap
					ui.lstClient1->addItem("");
				}
			}
		}
	}
}

//Opens the second client
void OpcodeFinder::Open2()
{
	//Make sure the user used the first open button before clicking this button
	if(ui.lstServer1->count() == 0)
	{
		QMessageBox::warning(this, "Error", "Please use the first open client button and then use this one.");
		return;
	}

	QString path = QFileDialog::getOpenFileName(this, "Locate sro_client.exe", "", "sro_client (sro_client.exe)");

	if(!path.isEmpty())
	{
		//Display the client path
		ui.txtPath2->setText(path);

		//Clear current opcodes
		ui.lstServer2->clear();
		ui.lstClient2->clear();

		//Extract opcodes
		QStringList opcodes = ExtractOpcodes(path);

		if(!opcodes.size())
		{
			QMessageBox::critical(this, "Error", "There was a problem extracting opcodes out of the client.");
		}
		else
		{
			//Make sure the first client has more opcodes otherwise comparing them would require more code
			if(ui.lstServer1->count() < opcodes.size())
			{
				ui.txtPath2->clear();
				QMessageBox::warning(this, "Error", "The first client you open must be newer than the one you just tried to open (has more opcodes than this client).");
				return;
			}

			for(int x = 0; x < opcodes.size(); ++x)
			{
				QString & opcode = opcodes[x];

				//Add the opcode to the list
				ui.lstServer2->addItem(opcode);

				//If the opcode starts with 'B' then it also has a client opcode that starts with '7'
				if(!opcode.isEmpty() && opcode[0] == 'B')
				{
					ui.lstClient2->addItem("7" + opcode.mid(1));
				}
				else
				{
					//Does not start with 'B' so add something else to fill in the gap
					ui.lstClient2->addItem("");
				}
			}

			if(ui.lstServer1->count() != ui.lstServer2->count())
				QMessageBox::warning(this, "Warning", "The opcode count is not the same for both clients. Comparing opcodes will not work.");
		}
	}
}

//Searches for an opcode
void OpcodeFinder::Search()
{
	QInputDialog input(this);
	input.setLabelText("Enter the opcode you would like to search for:");

	//Make sure the cancel button was not clicked
	if(input.exec() && !input.textValue().isEmpty())
	{
		//Get text and convert it to uppercase
		QString text = input.textValue().toUpper();

		for(int x = 0; x < ui.lstServer1->count(); ++x)
		{
			if(ui.lstServer1->item(x)->text() == text)
			{
				ui.lstServer1->setCurrentRow(x);
				return;
			}
		}

		for(int x = 0; x < ui.lstClient1->count(); ++x)
		{
			if(ui.lstClient1->item(x)->text() == text)
			{
				ui.lstClient1->setCurrentRow(x);
				return;
			}
		}

		for(int x = 0; x < ui.lstServer2->count(); ++x)
		{
			if(ui.lstServer2->item(x)->text() == text)
			{
				ui.lstServer2->setCurrentRow(x);
				return;
			}
		}

		for(int x = 0; x < ui.lstClient2->count(); ++x)
		{
			if(ui.lstClient2->item(x)->text() == text)
			{
				ui.lstClient2->setCurrentRow(x);
				break;
			}
		}
	}
}