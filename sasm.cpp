/* SICAssembler
 * Developer: nimblecrnch007 
 * Developer GitHub: https://github.com/nimblecrnch007/
 * 
 * 2 Pass Assembler for SIC Architecture 
 * Start date: 20th Nov 2014  
 * End date: 23rd Nov 2014
 *
 * Feel free to employ and distribute this code however you may wish.
 * Both the source code and the binary are public domain.
 *
 * NOTE: This source is written according to C++11 standards.
 *       To compile using g++, use: g++ -std=c++0x sasm.cpp
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

using namespace std;


/* Global variable declarations. */

/* Source asm file. */
ifstream source;

/* Assembler listing1 file. */
fstream listing1;

/* Assembler listing2 file. */
fstream listing2;

/* Assembled code. Object program file. */
fstream objectProgram;

/* Calculated length of the .asm program. */
unsigned int programLength;

/* Error types enumeration. */
enum ErrorType {
	ARGC, FORMAT, NOT_IN_SYMTAB, NOT_IN_OPTAB, DUP_SYM, START, CONST, PGMNAME
};

/* Space padding for formatting in object program. */
enum SpacePadding {
	PGMNAMELEN, ZEROS
};

/* END Global variable declarations. */


/* Realizing the sort of error and displaying message. */
class Error {
	ErrorType Err;
	string inputFile;
	int lineNo;
public:
	/* Constructor for Error objects. 
	 * This itself will call the print error function
	 * and also exit the program after printing.
	 */
	Error (ErrorType E) : Err(E) {
		this -> printErrorMessage ();
		this -> exitProgram ();
	}  
	/* Overloaded constructor for Error objects.
	 * Specific for checking the validity of input file format.
	 */
	Error (ErrorType E, string input) : Err(E), inputFile(input) {
		if (!(this -> isValidFileFormat())){
			this -> printErrorMessage ();
			this -> exitProgram ();
		}
	}  
	/* Overloaded constructor for Error objects.
	 * Specific for symbols and opcodes. 
	 */
	Error (ErrorType E, string input, int line) : Err(E), inputFile(input), lineNo(line) {
		this -> printErrorMessage ();
		this -> exitProgram ();
	}

	/* Print error message based on ErrorType. */
	void printErrorMessage ();
	
	/* Exit out of the program due to fatal error. */
	void exitProgram ();

	/* Check validity of input file format. */
	bool isValidFileFormat ();
};

/* Error member function definitions. */
void Error::printErrorMessage () {
		switch (Err)
		{
			case ARGC : 
				cout << "sasm: fatal error: 1 input file required" << endl << "terminated" << endl;
				break;

			case FORMAT :
				cout << inputFile << ": file format not recognized" << endl << "expected a .asm file" << endl;
				break;

			case NOT_IN_SYMTAB :
				cout << inputFile << ':' << lineNo << ": fatal error: symbol not present in symbol table" << endl << "terminated" << endl;
				break;

			case NOT_IN_OPTAB :
				cout << inputFile << ':' << lineNo << ": fatal error: invalid operation code" << endl << "terminated" << endl;
				break;

		    case DUP_SYM :
		    	cout << inputFile << ':' << lineNo << ": fatal error: multiple symbol definitions" << endl << "terminated" << endl;
		    	break;

		    case START :
		    	cout << "sasm: fatal error: START symbol not found" << endl << "flag NO_BEGIN status" << endl << "terminated" << endl;
				break;

			case CONST :
				cout << inputFile << ':' << lineNo << ": fatal error: unrecognized constant kind" << endl << "terminated" << endl;
				break;

			case PGMNAME :
				cout << "sasm: fatal error: program name too long" << endl << "flag PGM_NAME_INVALID status" << endl << "terminated" << endl;
				break;
		}
	}  // end printErrorMessage.

void Error::exitProgram (){
		exit (1);
	}

bool Error::isValidFileFormat (){
	if (!(inputFile.substr(inputFile.length()-4, string::npos)).compare(".asm"))
		return true;
	return false;
} // end Error definition. 


/* Global auxiliary function declarations. 
 * See below, definitions for function descriptions.
 */
bool fileLookup (string , string );
void SYMTABInsert (string , int );
unsigned int countNumberOfStrings (string );
unsigned int byteSize (string , string , int ); 
string padding (SpacePadding , string , int );
string itoa0x (int );
string getOpcode (string );
string getAddr (string , string );
string alterAddress (string );
string constToObj (string , string );

/* END Global variable declarations. */


/* Global primary function declarations. 
 * See below, definitions for function descriptions.
 */
void Pass1 (char* );
void Pass2 (char* , string );

/* END Global primary function declarations. */


/* file lookup. */
bool fileLookup (string filename, string key) {
	/* Open either symbol or opcode table file in read mode to lookup. */
	ifstream file;
	file.open (filename.c_str(), ios::in);

	string hold;
	file >> hold;
	while(!file.eof()) {
		if (key == hold){
			file.close();
			return true;
		}
		file >> hold;
		file >> hold;
	}
	file.close();
	return false;
}


/* Insert into SYMTAB. */
void SYMTABInsert (string LABEL, int LOCCTR) {
	/* Open symtab file in append mode. */
	ofstream symtab;
	symtab.open ("symtab.txt", ios::app);
	symtab << LABEL << ' ' << LOCCTR << '\n';
	symtab.close();
}


/* Counts the number of strings in a given string delimited by spaces. */
unsigned int countNumberOfStrings (string input) {
	istringstream splitString (input);
	int counter = 0;
	string hold;
	while (splitString >> hold && hold[0] != '.')
		counter++;
	return counter;
}  // end counterNumberOfStrings.


/* Figure out the kind of BYTE constant 
 * and then find the size of the constant.
 */
unsigned int byteSize (string OPERAND, string inputFile, int lineNumber) {
	
	/* Check the kind of CONSTANT it is. */
	switch (OPERAND[0]){
		/* HEX. */
		case 'X': 
			return (OPERAND.length() - 2) / 2;

		case 'C':
			return (OPERAND.length() - 3);

	    default : Error (ErrorType (CONST), inputFile, lineNumber);
	}
}


/* Padding function to format the object program according to SpacePadding type.*/
string padding (SpacePadding PaddingType, string input, int length = 6) {
	
	switch (PaddingType) {
		
		case PGMNAMELEN:
		{
			string holdspaces = "";
			for (int i = 0; i < length - input.length(); i++)
				holdspaces += ' ';
			return holdspaces;
		}

		case ZEROS:
		{
			string holdzeros = "";
			for (int i = 0; i < length - input.length(); i++)
				holdzeros += '0';
			return holdzeros;
		}
	}
}



/* Principle function of the assembler. */
void Pass1 (char* argv) {

	int startAddr;
	int LOCCTR = 0;
	int lineNumber = 0;
	string currentLine;
	string OPCODE;
	string LABEL;
	string OPERAND;


	/* Store the words in a line into an array of strings. */
	string* words = new string[3] ();

	/* Start to loop over the lines and assign addresses. */
	while (1) {
		/* Increment lineNumber. */
		lineNumber++;

		/* Read first input line. */
		getline (source, currentLine);
		//cout << lineNumber << endl;

		int counter = countNumberOfStrings (currentLine);

		if (counter == 1)
			words[1] = "";
		/* currentLine string is converted into a string stream of strings separated by whitespaces,
		 * which is stored in splitLine.
		 */
		istringstream splitLine (currentLine);

		words[2] = "";
		/* Words[i] contains the i'th word in a line of asm code. */
		int i = 0;
		while(splitLine >> words[i++] && i < counter);

		/* If END is found then break out of the loop, end pass1. */
		if (words[0] == "END"){
			listing1 << words[0] << "\t\t" << words[1];
			break;
		}

		if (lineNumber == 1) {
			if (words[1] == "START") {
				if (!words[2].empty())
					startAddr = stoi (words[2]);
				
				else 	
					startAddr = 0;
				
				LOCCTR = startAddr;

				/* Write line1 into listing1. */
				listing1 << padding (SpacePadding (ZEROS), to_string (LOCCTR), 4); 
				listing1 << hex << LOCCTR << "\t" << words[0] << padding (SpacePadding (PGMNAMELEN), words[0], 10); 
				listing1 << "\t" << words[1] << padding (SpacePadding (PGMNAMELEN), words[1], 15) << words[2] << '\n';

			}  // end if
			/* if START isn't found in line number 1, create Error instance. */ 
			else 
				Error (ErrorType (START));
		} // end if
		else{
			/* Check if it is a comment line. 
			 * If yes, then continue to the next line.
			 */
			if (words[0][0] == '.')
				continue;

			/* Index to OPCODE in words* */
			int opcodeIndex = 0;

			/* Index to OPERAND in words* */
			int operandIndex = 1;

			/* If it isn't a comment line. 
			 * If there is symbol in the LABEL field words[0], 
			 * lookup symbol in the symbol table and if dup then exit.
			 * Otherwise insert it into SYMTAB. */
			if (counter == 3) {
				
				LABEL = words[0];
				opcodeIndex  = 1;
				operandIndex = 2;

				if (fileLookup (string ("symtab.txt"), LABEL))
					Error (ErrorType (DUP_SYM), string (argv), lineNumber);

				else
					SYMTABInsert (LABEL, LOCCTR);
			} // end if

			OPERAND = words[operandIndex];
			OPCODE  = words[opcodeIndex];

			/* Write assembler listing1 to listing1 file. (unforamtted) */
			listing1 << padding (SpacePadding (ZEROS), to_string (LOCCTR), 4); 
			listing1 << hex << LOCCTR << "\t" << words[0] << padding (SpacePadding (PGMNAMELEN), words[0], 10); 
			listing1 << "\t" << words[1] << padding (SpacePadding (PGMNAMELEN), words[1], 15)  << words[2] << '\n';

			/* Search OPTAB for OPCODE. 
			 * If found then modify LOCCTR accordingly.
			 * Else check if it is a directive.
			 * If none, throw a NOT_IN_OPTAB and exit.
			 */
			if ((fileLookup (string ("optab.txt"), OPCODE)) || OPCODE == "WORD")
				LOCCTR += 3;

			else if (OPCODE == "RESW")
				LOCCTR += 3 * stoi (OPERAND);

			else if (OPCODE == "RESB")
				LOCCTR += stoi (OPERAND);

			else if (OPCODE == "BYTE") {
				LOCCTR += byteSize (OPERAND, string(argv), lineNumber);	// filename and lineNumber for error handling. 
			}
			
			else
				Error (ErrorType (NOT_IN_OPTAB), string (argv), lineNumber);
		}
		
	}

	programLength = LOCCTR - startAddr;
} // END pass1.


/* Function to convert a given decimal integer into hex string. */
string itoa0x (int number) {
	stringstream ss;
    ss << hex << number;
	return ss.str();
}


/* Given MNEMONIC, get OPCODE. */
string getOpcode (string MNEMONIC) {
	ifstream optabfile;
	optabfile.open ("optab.txt", ios::in);

	//cout << "MNEMONIC = " << MNEMONIC << endl;
	string hold;
	string OPCODE;
	while (1) {
		optabfile >> hold;
		//cout << "hold = " << hold << endl;
		optabfile >> OPCODE;
		//cout << "opcode = " << OPCODE << endl;

		if (hold == MNEMONIC) {
			optabfile.close ();
			//cout << "Returning OPCODE = " << OPCODE << endl;
			return OPCODE;
		}
	}
}


/* Get ADDRESS of given OPERAND from listing1. */
string getAddr (string inputFilename, string OPERAND) {
	
	/* Open listing1 using a temporary file. */
	string filename = string (inputFilename).substr(0, (string (inputFilename)).length() - 4) + ".listing1";
	ifstream file;
	file.open (filename, ios::in);

	string currentline, hold0, hold1, hold2;

	while (1) {
		getline (file, currentline);
		istringstream splitString (currentline);
		if (countNumberOfStrings (currentline) == 4) {
			splitString >> hold0;
			splitString >> hold1;
			if (hold1 == OPERAND) {
				file.close();
				return hold0;
			}
		}
	}
}


/* Alter ADDRESS for INDEXED MODE */
string alterAddress (string ADDRESS) {
	stringstream ss;
	ss << dec << ADDRESS;
	int tempAddr = stoi (ss.str ());
	tempAddr += 32768;	// Add 0x8000
	return itoa0x (tempAddr);
}


/* Convert constant into object code. */
string constToObj (string constant, string type) {
	
	string temp;
	string result;
	
	if (type == "BYTE") {
		if (constant[0] == 'C') {
			temp = constant.substr (2, constant.length() - 3);
			
			for (int i = 0; temp.c_str()[i] != '\0'; i++) {
				result += itoa0x ((int) temp.c_str()[i]);
			}
			return result;
		}
		else 
			return constant.substr (2, constant.length() - 3);
	}

	else if (type == "WORD") {
		string result;
		for (int i = 0; i < 6 - constant.length (); i++) 
			result += '0';
		result += constant;
		return result;
	}
}


/* Pass2 of assembler. */
void Pass2 (char* inputFilename, string listing1Filename) {

	string startAddr;
	string programName;
	int lineNumber = 0;
	string currentLine;
	string currentObjLine;
	string operandAddress;
	string LOCCTR;
	string LABEL;
	string OPERAND;
	string MNEMONIC;
	string OPCODE;
	string OBJCODE;
	string ADDRESS;
	string textString = "";
	string textStart;
	int textLength = 0;
	string* words = new string[4] ();

	while (1) {

		lineNumber++;

		/* Read line and store into currentLine */
		getline (listing1, currentLine);

		/* Count the number of words in listing1 line. */
		int counter = countNumberOfStrings (currentLine);

		/* Initialize a stream of strings from currentLine. */
		istringstream splitString (currentLine);

		/* Store strings in splitStream into words. */
		int i = 0;
		while (splitString >> words[i++] && i < counter);

		/* If MNEMONIC == START that is in line 1, write line verbatim to listing2. */
		if (lineNumber == 1) {
			startAddr   = words[0];
			textStart   = startAddr;
			programName = words[1];
			
			/* If programName is more than 6 chars long, throw fatal error. */
			if (programName.length() > 6)
				Error (ErrorType (PGMNAME));

			for (int i = 0; i < counter - 1; i++)
				listing2 << words[i] << "\t\t";
			listing2 << endl; 

			/* Write HEADER RECORD. */
			objectProgram << 'H' << programName; 
			objectProgram << padding (SpacePadding (PGMNAMELEN), programName) << padding(SpacePadding (ZEROS), startAddr); 
			objectProgram << startAddr;
			objectProgram << padding (SpacePadding (ZEROS), itoa0x (programLength));
			objectProgram << itoa0x (programLength) << '\n';
			continue;
		}

		/* If directive is END then writing to listing2. 
		 * Write END record to object program.
		 * Break out of current loop and end Pass2. 
		 */
		if (words[0] == "END") {
			/* listing2 entry. */
			for (int i = 0; i < counter; i++)
				listing2 << words[i] << padding (SpacePadding (PGMNAMELEN), words[i], 12);

			/* END record. */
			objectProgram << 'E' << padding (SpacePadding (ZEROS), startAddr) << startAddr;
			break;
		}

		/* TEXT record related listing now. */

		/* If there is SYMBOL in OPERAND field.
		 * Search SYMTAB for OPERAND.
		 * Store symbol value as OPERAND ADDRESS. 
		 */

		LOCCTR = words[0];

		if (counter == 4) {
			LABEL    = words[1];
			MNEMONIC = words[2];
			OPERAND  = words[3];
		}

		else if (counter == 3) {
			LABEL    = "";
			MNEMONIC = words[1];
			OPERAND  = words[2];
		}

		else if (counter == 2) {
			LABEL    = "";
			MNEMONIC = words[1];
			OPERAND  = "";
		}

		/* SYMBOL in OPERAND field exists. */
		if (counter == 4 || counter == 3) {
			
			if (fileLookup ((string ("optab.txt")), MNEMONIC)) {

				/* Lookup SYMBOL in symtab.txt. */
				if (fileLookup (string ("symtab.txt"), OPERAND)) {

					OPCODE = getOpcode (MNEMONIC);
					
					/* Start init of OBJCODE. */
					OBJCODE = OPCODE;

					/* Find address of the OPERAND from listing1. */
					ADDRESS = getAddr (inputFilename, OPERAND);

					/* Figure out if X is set/reset */
					if (OPERAND.find (",X") != string::npos) 
						ADDRESS = alterAddress (ADDRESS);

					/* Final OBJCODE. */
					OBJCODE += ADDRESS;
					//cout << "OBJCODE = " << OBJCODE << endl;
					
				}
			}

			else if (MNEMONIC == "BYTE") {
				OBJCODE = constToObj (OPERAND, "BYTE");
			}

			else if (MNEMONIC == "WORD") {
				OBJCODE = constToObj (OPERAND, "WORD");
			}

			else if (MNEMONIC == "RESB" || MNEMONIC == "RESW") {
				OBJCODE = "";
		    }

			/* If lookup fails, create Error instance and exit. */
			else {
				Error (ErrorType (NOT_IN_SYMTAB), listing1Filename, lineNumber);
			}
	
		}
		
		/* Write listing2 entry. */
		for (int i = 0; i < 4; i++)
			listing2 << words[i] << padding (SpacePadding (PGMNAMELEN), words[i], 10) << "\t";
		listing2 << "\t" << OBJCODE << endl;

		for (int i = 0; i < 4; i++) {
			words [i] = "";
		}

		/* TEXT record string. */
		if ((textLength + OBJCODE.length () / 2) > 30) {
				objectProgram << 'T' << padding (SpacePadding (ZEROS), textStart); 
				objectProgram << textStart << padding (SpacePadding (ZEROS), itoa0x (textLength), 2);
				objectProgram << itoa0x (textLength) << textString << endl;
				textLength = 0;
				textString = "";
				textStart  = LOCCTR;  
		}
		textLength += OBJCODE.length () / 2;	
		textString += OBJCODE;
	}
}


int main (int argc, char** argv){

	/* Clear terminal screen. */
	system ("clear");

	/* Check if there is one file as input. */
	if (argc != 2)
		Error (ErrorType (ARGC));

	/* Check the validity of the input file format. */
	Error (ErrorType (FORMAT), string (argv[1]));

	/* Open input asm file in read mode. */ 
	source.open (argv[1], ios::in);

	/* Open assembler listing1 file in write mode. */
	string listing1Filename = string (argv[1]).substr(0, (string (argv[1])).length() - 4) + ".listing1";
	listing1.open (listing1Filename.c_str(), ios::out);
	
	/* Open and close symtab.txt to clear previous output. */
	ofstream symboltable;
	symboltable.open ("symtab.txt", ios::out);
	symboltable.close ();

	Pass1 (argv[1]);

	/* Close listing1 file after pass1 and reopen in read mode for pass2. */
	listing1.close();
	listing1.open (listing1Filename.c_str(), ios::in);

	/* Open listing2 file for pass2 in write mode. */
	string listing2Filename = string (argv[1]).substr(0, (string (argv[1])).length() - 4) + ".listing2";
	listing2.open (listing2Filename.c_str(), ios::out);

	/* Open Object program file for pass2 in write mode. */
	string objPgmFilename = string (argv[1]).substr(0, (string (argv[1])).length() - 4) + ".obj";
	objectProgram.open (objPgmFilename.c_str(), ios::out);
	objectProgram.close ();
	objectProgram.open (objPgmFilename.c_str(), ios::out | ios::in);
	
	Pass2 (argv[1], listing1Filename);
}
