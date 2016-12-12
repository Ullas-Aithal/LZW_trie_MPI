#include <iostream>
#include <sstream>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
#include <map>

//
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <vector>
//#include <conio.h>
#include <sstream>
#include <string>
#include <map>
#include <sys/stat.h>
#include <cstring>
//
#define ARRAYSIZE 26  // maximum characters supported currently for compression and decompression

using namespace std;

string inputFile = "serialInput.txt";  // input value with default value
string outputFile = "serialOutput.txt";  // output value with default value
string hasFileToStoreIntermediateValue = "hash.txt"; // to store intermediate value
string encodedFile = "encodedFile.txt";
string stringBuffer;  // streing buffer to get input value
int mEncodingCode = 0;  // generate the code for every new string found
typedef std::map<int, string> DictionaryHash;
DictionaryHash hashToFile; // to write hash values to file
DictionaryHash hashFromFile; // to get hash values from file
ofstream output_file;  // outputfile stream to write output
ifstream input_file; // inputfile stream to get input


/* all single characters */
char arrayChar[] = { 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };


/*node construct the trie data structure*/
typedef struct Node {

	int mValue = -1;
	Node *mChild[ARRAYSIZE] = { NULL };

}NODE;




/*allocate new node required by the */
Node* allocateNode(int value) {

	NODE *newNode = (NODE*)calloc(1, sizeof(NODE));

	if (newNode == NULL) {
		cout << "failed to allocate memory: Should not be here\n";
		exit(0);

	}
	newNode->mValue = value;

	return newNode;
}


/* initialize the all single character with encoding code*/
void initializeSingleCharValue(NODE *trie) {

	for (int i = 0; i < ARRAYSIZE; i++)
		if (trie->mChild[i] == NULL) {
			trie->mChild[i] = allocateNode(i);
			hashToFile[i] = arrayChar[i];

		}
		else {
			cout << "should not be here!!!!" << endl;
		}

		mEncodingCode = ARRAYSIZE;
}

/*insert the given string to the trie and generate compression code*/
string insertWord(string input_string, NODE *trie) {

	
	vector<int> encodedWord;
	vector<int> code;
	string finalCompressedContentAsString;
	ostringstream convert;
	NODE *temp_trie = trie;
	bool last_search_status = true;
	//string input_string(input);
	string  current_string, prefix_string, lastString;
	int prev_code, current_code, current_string_length, encoded_value, valueToAddInEnd, newValueForNewNode;
	int i = 0;

	int word_size = input_string.length();
	while (i < word_size) {

		current_string = prefix_string + string(1, input_string[i]);  // prepare new string by adding current new character
		current_string_length = current_string.length();
		lastString = current_string; // to add value in end

		{

			NODE *temp_trie_to_add = trie;  // search from root and add new item to dictionary
			NODE* trie_node_to_getValue = NULL;

			int j = 0;
			while (temp_trie_to_add->mChild[current_string[j] - 'a'] != NULL && j < current_string_length) {

				temp_trie_to_add = temp_trie_to_add->mChild[current_string[j] - 'a'];
				j++;
			}

			if (j == current_string_length) {   // found
												// patter found
												// its present in the tree just copy the encoding code for this found string
				trie_node_to_getValue = temp_trie_to_add;

				prefix_string = current_string;  // take full string to next itteration
				last_search_status = true;
			}
			else if (j < current_string_length) {  // not found

				if (temp_trie_to_add->mChild[current_string[current_string_length - 1] - 'a'] == NULL) {

					encodedWord.push_back(temp_trie_to_add->mValue);// add value to encoded value
					newValueForNewNode = mEncodingCode++;
					hashToFile[newValueForNewNode] = current_string.c_str();

					temp_trie_to_add->mChild[current_string[current_string_length - 1] - 'a'] = allocateNode(newValueForNewNode);  // creating value and putting value
					trie_node_to_getValue = temp_trie_to_add->mChild[current_string[current_string_length - 1] - 'a']; // taking previous node
					prefix_string = current_string[current_string_length - 1]; // take last character to next itteration
																			   //encodedWord.push_back(trie_node_to_getValue->mValue);
					last_search_status = false;

				}
				else { // should not be here
					cout << "should not be here" << endl;
				}
			}
		}

		i++;
	}


	if (last_search_status == true) {
		current_string = lastString;
	}
	else {
		current_string = string(1, input_string[word_size - 1]);
	}

	current_string_length = current_string.length();

	temp_trie = trie;
	NODE* prev_trie = NULL;
	int j = 0;
	while (temp_trie->mChild[current_string[j] - 'a'] != NULL && j < current_string_length) {
		prev_trie = temp_trie;
		temp_trie = temp_trie->mChild[current_string[j] - 'a'];
		j++;
	}

	if (temp_trie != NULL)
		encodedWord.push_back(temp_trie->mValue);
	else
		cout << "should not be here\n";


	i = 0;
	while (encodedWord.size() != 0) {
		if (i == 0) {
			convert << encodedWord[0];
			i++;
		}
		else
			convert << "," << encodedWord[0];
		encodedWord.erase(encodedWord.begin());
	}

	finalCompressedContentAsString = convert.str();

	return finalCompressedContentAsString;
}



/* Set the program parameters from the command-line arguments */
void parameters(int argc, char **argv) {

	if (argc == 3) {

		inputFile = argv[1];
		outputFile = argv[2];
		cout << "InputFile : " << inputFile << "OutputFile : " << outputFile << endl;

	}
	else {

		cout << "Enter input and output file with format ./a.out <inputfile> <outputfile>\n";
		cout << "Enter ./a.out default to consider default file to execute program \n";
		//exit(0);
	}

	
}



/*read input file to get the string present int he file*/
void readInputFile(string input_file_name)
{
	
	
	input_file.open(input_file_name.c_str(), ios::in);

	if (!input_file.is_open())
	{
		cout << "File not found :" << input_file_name << endl;
		
		exit(1);
	}


	std::getline(input_file, stringBuffer);  // reading character data from the file

	input_file.close();  // closing file opened for reading

	//cout << stringBuffer << endl;

	//return stringBuffer;

}


/*compression request*/
void compress(string sstr)
{
	
	NODE *mRootNode = allocateNode(-1); // first root node with value -1 for the trie structure
	initializeSingleCharValue(mRootNode);  // initialize values for single character

	string code;  // encoded string
	string file_name; // file 
	stringstream ss;

	code = insertWord(sstr, mRootNode);


//	cout << "Output file name \n";
	//cout << "compress code: " << code << endl;

	output_file.open(encodedFile.c_str(), ios::out);
	output_file << code;
	output_file.close();
}


void putHashToFile()
{


	output_file.open(hasFileToStoreIntermediateValue.c_str(), ios::out);

	if (!output_file)
	{
		cout << "File not found";
		exit(1);
	}


	for (DictionaryHash::const_iterator iterator = hashToFile.begin(); iterator != hashToFile.end(); iterator++)
	{
		output_file << iterator->first << "|" << iterator->second;
		output_file << "\n";
	}
	output_file.close();
}



void getHashFromFile()
{


	string line;

	input_file.open(hasFileToStoreIntermediateValue.c_str(), ios::in);



	if (!input_file)
	{
		cout << "File not found";
		exit(1);
	}


	while (std::getline(input_file, line))
	{

		istringstream is_line(line);
		string key;

		if (getline(is_line, key, '|'))
		{
			std::string value;
			if (key[0] == '\n')
				continue;

			if (std::getline(is_line, value))
			{
				hashFromFile[atoi(key.c_str())] = value;

			}
		}
	}
	input_file.close();

}



void decompress()
{

	char const* output_file_name = encodedFile.c_str();
	ostringstream op;


	string  line;


	input_file.open(output_file_name, ios::in);

	if (!input_file.is_open()) {

		cout << "Output partial files failed to open. Exiting...........\n";
		exit(1);
	}

	string str;

	while (std::getline(input_file, str))
	{
	
	
		stringstream ss(str);

		while (ss.good())
		{
			string substr;
			getline(ss, substr, ',');
			op << hashFromFile[atoi(substr.c_str())];
		}
	}

	//cout << "op" << op.str() << endl;
	string op_string = op.str();
	const char *op_char = op_string.c_str();

	//input_file<<op_char;  // output to check

	input_file.close(); // closing input


	output_file.open(outputFile.c_str(), ios::out);
	if (!output_file.is_open()) {
		cout << "failed to open file : " << outputFile << endl;
		//getchar();
		exit(0);
	}

	
	output_file << op_char;
	output_file.close();

}



int main(int argc, char **argv) {

	/* Start Clock */
	clock_t mCompressionStart , mDecompressionStart;
	clock_t mCompressionEnd, mDecompressionEnd;
	double elapsed_secs;

	parameters(argc, argv);



	readInputFile(inputFile);
    //cout << readInputFile(inputFile) << endl;  // to cross verify buffer is read or not


	mCompressionStart = clock();  // start compression time
	compress(stringBuffer);
	mCompressionEnd = clock();  // end compression time

	elapsed_secs = double(mCompressionEnd - mCompressionStart) / CLOCKS_PER_SEC;

	cout << "elapsed_secs for compression: " << elapsed_secs << endl;

	putHashToFile();  // put all encoded value to hashtable
	getHashFromFile(); // get hash value back for decompression

	mDecompressionStart = clock();  // start decompression time
	decompress();					// calling decompression 
	mDecompressionEnd = clock();  // end decompression time

	elapsed_secs = double(mDecompressionEnd - mDecompressionStart) / CLOCKS_PER_SEC;  // decompressoin time

	cout << "elapsed_secs for decompression: " << elapsed_secs << endl;

	return 0;

}