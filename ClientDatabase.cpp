#include "ClientDatabase.h"

ClientDatabase::ClientDatabase(istream & in) :in(in) {
}
	
void ClientDatabase::open(string filename, int size) 
{
	file.open(filename.data(), ios::in | ios::out | ios::binary);
	dbSize = size;
	if ( !file ) 
	{
		cerr << "File could not be opened." << endl;
		cout << "Would you like to create a new file (y/n)? ";
		char answer;
		in >> answer;
		if (tolower(answer) == 'y') {
			file.clear();
			file.open("credit.dat", ios::out | ios::binary);
			if ( !file )
				throw runtime_error("Error creating file.");
			else {
				if (size < 0)
					dbSize = 100;
				writeBlankRecords();
			}
			file.close();
			file.open("credit.dat", ios::in | ios::out);
			if ( !file ) {
				throw runtime_error("Error opening newly created file.");
			}
		}
	} // end if
	file.seekg(0, ios::end);
	size_t fileSize = file.tellg();
	size_t recordSize = sizeof(ClientData);
	if (size < 0) 
	{
		if (fileSize % recordSize == 0)
			dbSize = fileSize / recordSize;
		else
			throw runtime_error("File does not seem to have integral number of clients.");
	}
	else 
	{
		if (fileSize != (recordSize * dbSize))
			throw invalid_argument("Provided database size is not consistent with existing database");
	}
}

void ClientDatabase::writeBlankRecords() 
{
	ClientData blankClient;
	for (size_t i=0; i<dbSize; i++) 
	{
		file.write( reinterpret_cast<const char * >( &blankClient), sizeof(blankClient));
	}
}

void ClientDatabase::run() {
	int choice = readUserChoice();
	while(choice != END) {
		performChoice(choice);
		choice = readUserChoice();
	}
}

int ClientDatabase::readUserChoice() 
{
   // display available options
   int menuChoice;
   do {
	   // if reading from cin, print out menu
	   if (&in == &cin) {
		   cout << "\nEnter your choice" << endl
			  << "1 - store a formatted text file of accounts" << endl
			  << "    called \"print.txt\" for printing" << endl
			  << "2 - update an account" << endl
			  << "3 - add a new account" << endl
			  << "4 - delete an account" << endl
			  << "5 - find account by last name" << endl
			  << "6 - end program\n? ";
	   }
	   in >> menuChoice; // input menu selection from user
   } while(!in);
   return menuChoice;
}

void ClientDatabase::performChoice(int op) 
{
	switch(op) 
	{
		case PRINT:
			createTextFile();
			break;
		case UPDATE:
			updateRecord();
			break;
		case NEW:
			newRecord();
			break;
		case DELETE:
			deleteRecord();
			break;
		case FIND:
			findRecordByLastName();
			break;
		case END:
			break;
	}
	file.clear(); // reset end-of-file indicator
}

// create formatted text file for printing
void ClientDatabase::createTextFile()
{
	string filename;
	cout << "Enter file name (leave blank for default, \"print.txt\"): ";
	if(in.peek() == '\n')
		in.ignore();
	getline(in, filename);
	if (filename.length() == 0)
		filename = "print.txt";
	// create text file
	ofstream outPrintFile( filename.data(), ios::out );

	// exit program if ofstream cannot create file
	if ( !outPrintFile ) 
		throw runtime_error("File could not be created.");

	// create column headers
	outPrintFile << left << setw( 10 ) << "Account" << setw( 16 )
		<< "Last Name" << setw( 11 ) << "First Name" << right
		<< setw( 10 ) << "Balance" << endl;

	// set file-position pointer to beginning of readFromFile
	file.seekg( 0 );

	// read first record from record file
	ClientData client;
	file.read( reinterpret_cast< char * >( &client ),
				sizeof( ClientData ) );

	// copy all records from record file into text file
	while ( !file.eof() ) 
	{
		// write single record to text file
		if ( client.getAccountNumber() != 0 ) // skip empty records
			client.display(outPrintFile);

		// read next record from record file
		file.read( reinterpret_cast< char * >( &client ), 
					sizeof( ClientData ) );
	} // end while
} // end function createTextFile


void ClientDatabase::updateRecord()
{
	// obtain number of account to update
	int accountNumber = getAccountNum( "Enter account to update" );

	// move file-position pointer to correct record in file
	file.seekg( ( accountNumber - 1 ) * sizeof( ClientData ) );

	// read first record from file
	ClientData client;
	file.read( reinterpret_cast< char * >( &client ), 
				sizeof( ClientData ) );

	// update record
	if ( client.getAccountNumber() != 0 ) 
	{
		client.display(cout);

		// request user to specify transaction
		cout << "\nEnter charge (+) or payment (-): ";
		double transaction; // charge or payment
		in >> transaction;
		while(!in) {
			in.clear();
			in.ignore(128, '\n');	// ignore offending input
			cout << "I'm sorry, I didn't understand that amount." << endl;
			cout << "\nEnter charge (+) or payment (-): ";
			in >> transaction;
		}

		// update record balance
		double oldBalance = client.getBalance();
		client.setBalance( oldBalance + transaction );
		client.display(cout);

		// move file-position pointer to correct record in file
		file.seekp( ( accountNumber - 1 ) * sizeof( ClientData ) );

		// write updated record over old record in file
		file.write( reinterpret_cast< const char * >( &client ), 
					sizeof( ClientData ) );
	} // end if
	else // display error if account does not exist
		cerr << "Account #" << accountNumber 
		<< " has no information." << endl;
} // end function updateRecord


// create and insert record
void ClientDatabase::newRecord()
{
	// obtain number of account to create
	int accountNumber = getAccountNum( "Enter new account number" );

	// move file-position pointer to correct record in file
	file.seekg( ( accountNumber - 1 ) * sizeof( ClientData ) );

	// read record from file
	ClientData client;
	file.read( reinterpret_cast< char * >( &client ), 
				sizeof( ClientData ) );

	// create record, if record does not previously exist
	if ( client.getAccountNumber() == 0 ) 
	{
		string lastName;
		string firstName;
		double balance;

		// user enters last name, first name and balance
		cout << "Enter lastname, firstname, balance\n? ";
		in >> lastName;
		in >> firstName;
		in >> balance;
		// remove commas from names (not terribly robust)
		if (lastName[lastName.length()-1] == ',')
			lastName.erase(lastName.length()-1,1);
		if (firstName[firstName.length()-1] == ',')
			firstName.erase(firstName.length()-1,1);

		// use values to populate account values
		client.setLastName( lastName );
		client.setFirstName( firstName );
		client.setBalance( balance );
		client.setAccountNumber( accountNumber );

		// move file-position pointer to correct record in file
		file.seekp( ( accountNumber - 1 ) * sizeof( ClientData ) );

		// insert record in file                       
		file.write( reinterpret_cast< const char * >( &client ),
					sizeof( ClientData ) );                     
	} // end if
	else // display error if account already exists
		cerr << "Account #" << accountNumber
			<< " already contains information." << endl;
} // end function newRecord

// delete an existing record
void ClientDatabase::deleteRecord( )
{
	// obtain number of account to delete
	int accountNumber = getAccountNum( "Enter account to delete" );

	// move file-position pointer to correct record in file
	file.seekg( ( accountNumber - 1 ) * sizeof( ClientData ) );

	// read record from file
	ClientData client;
	file.read( reinterpret_cast< char * >( &client ), 
							sizeof( ClientData ) );

	// delete record, if record exists in file
	if ( client.getAccountNumber() != 0 ) 
	{
		ClientData blankClient; // create blank record

		// move file-position pointer to correct record in file
		file.seekp( ( accountNumber - 1 ) * 
					sizeof( ClientData ) );

		// replace existing record with blank record
		file.write( reinterpret_cast< const char * >( &blankClient ), 
					sizeof( ClientData ) );

		cout << "Account #" << accountNumber << " deleted.\n";
	} // end if
	else // display error if record does not exist
		cerr << "Account #" << accountNumber << " is empty.\n";
} // end deleteRecord

void ClientDatabase::findRecordByLastName()
{
	// TODO: FILL IN THIS FUNCTION
	// obtain number of account to update
	//int accountNumber = getAccountNum( "Enter account to update" );
	cout << "Enter the Last Name to find:";
	string lastName;
	in >> lastName;
	// move file-position pointer to correct record in file
	for(size_t i=1; i<=dbSize-1; ++i)
	{
		file.seekg( ( i - 1 ) * sizeof( ClientData ) );

	// read first record from file
		ClientData client;
		file.read( reinterpret_cast< char * >( &client ), sizeof( ClientData ) );
		if(client.getLastName() == lastName)
		{
			cout << "Found account:";
			client.display(cout);
		}

	}

	
}

// obtain account-number value from user
int ClientDatabase::getAccountNum( const string & prompt )
{
	int accountNumber;
	// obtain account-number value
	do 
	{
		cout << prompt << " (1 - 100): ";
		in >> accountNumber;
	} while ( accountNumber < 1 || accountNumber > static_cast<int>(dbSize) );

   return accountNumber;
} // end function getAccount
