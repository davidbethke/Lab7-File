#ifndef CLIENT_DATABASE_H
#define CLIENT_DATABASE_H

#include "ClientData.h"
#include <stdexcept>
#include <fstream>
using namespace std;

enum DBOperation { PRINT = 1, UPDATE, NEW, DELETE, FIND, END };

class ClientDatabase {
public:
	ClientDatabase(istream & in = cin); // specifies input stream, so interactions can be run from a file instead
	void open(string filename="credit.dat", int size = -1);
	void run(); 
private:
	void createTextFile();
	void updateRecord();
	void newRecord();
	void deleteRecord();
	void findRecordByLastName();
	int readUserChoice();
	void performChoice(int op);
	int getAccountNum(const string & prompt);
	void writeBlankRecords();
	istream & in;
	fstream file;
	size_t dbSize;
	bool failbit;
};

#endif