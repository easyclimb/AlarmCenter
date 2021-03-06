// TestProtobuf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>


#ifdef _DEBUG
#pragma comment(lib, "D:/dev_libs/google/protobuf-3.0.0-beta-3.1/vsprojects/Debug/libprotobuf.lib")
#else
#pragma comment(lib, "D:/dev_libs/google/protobuf-3.0.0-beta-3.1/vsprojects/Release/libprotobuf.lib")
#endif

#include "addressbook.pb.h"


using namespace std;

// This function fills in a Person message based on user input.
void PromptForAddress(tutorial::Person* person) {
	cout << "Enter person ID number: ";
	int id;
	cin >> id;
	person->set_id(id);
	cin.ignore(256, '\n');

	cout << "Enter name: ";
	getline(cin, *person->mutable_name());

	cout << "Enter email address (blank for none): ";
	string email;
	getline(cin, email);
	if (!email.empty()) {
		person->set_email(email);
	}

	while (true) {
		cout << "Enter a phone number (or leave blank to finish): ";
		string number;
		getline(cin, number);
		if (number.empty()) {
			break;
		}

		tutorial::Person::PhoneNumber* phone_number = person->add_phone();
		phone_number->set_number(number);

		cout << "Is this a mobile, home, or work phone? ";
		string type;
		getline(cin, type);
		if (type == "mobile") {
			phone_number->set_type(tutorial::Person::MOBILE);
		} else if (type == "home") {
			phone_number->set_type(tutorial::Person::HOME);
		} else if (type == "work") {
			phone_number->set_type(tutorial::Person::WORK);
		} else {
			cout << "Unknown phone type.  Using default." << endl;
		}
	}
}


// Iterates though all people in the AddressBook and prints info about them.
void ListPeople(const tutorial::AddressBook& address_book) {
	for (int i = 0; i < address_book.person_size(); i++) {
		const tutorial::Person& person = address_book.person(i);

		cout << "Person ID: " << person.id() << endl;
		cout << "  Name: " << person.name() << endl;
		if (!person.email().empty()) {
			cout << "  E-mail address: " << person.email() << endl;
		}

		for (int j = 0; j < person.phone_size(); j++) {
			const tutorial::Person::PhoneNumber& phone_number = person.phone(j);

			switch (phone_number.type()) {
			case tutorial::Person::MOBILE:
				cout << "  Mobile phone #: ";
				break;
			case tutorial::Person::HOME:
				cout << "  Home phone #: ";
				break;
			case tutorial::Person::WORK:
				cout << "  Work phone #: ";
				break;
			}
			cout << phone_number.number() << endl;
		}
	}
}


int main(int argc, char** argv)
{
	// Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	if (argc != 2) {
		cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
		return -1;
	}

	// write
	{
		tutorial::AddressBook address_book;

		{
			// Read the existing address book.
			fstream input(argv[1], ios::in | ios::binary);
			if (!input) {
				cout << argv[1] << ": File not found.  Creating a new file." << endl;
			} else if (!address_book.ParseFromIstream(&input)) {
				cerr << "Failed to parse address book." << endl;
				return -1;
			}
		}

		// Add an address.
		PromptForAddress(address_book.add_person());

		{
			// Write the new address book back to disk.
			fstream output(argv[1], ios::out | ios::trunc | ios::binary);
			if (!address_book.SerializeToOstream(&output)) {
				cerr << "Failed to write address book." << endl;
				return -1;
			}
		}
	}

	// read
	{
		tutorial::AddressBook address_book;

		{
			// Read the existing address book.
			fstream input(argv[1], ios::in | ios::binary);
			if (!address_book.ParseFromIstream(&input)) {
				cerr << "Failed to parse address book." << endl;
				return -1;
			}
		}

		ListPeople(address_book);
	}

	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();
	std::system("pause");

    return 0;
}

