#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib> // exit function prototype
#include <string>
#include "ClientDatabase.h" // ClientData class definition
using namespace std;


int main() 
{
	ClientDatabase db;
	try {
		db.open("credit.dat", 100);
		db.run();
	}
	catch (exception & e) {
		cout << "Error opening database: " << e.what() << endl;
	}
	
	return 0;
}
