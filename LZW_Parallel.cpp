#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include <sstream>
#include <string>
#include <map>
#include <mpi.h>
#include <sys/stat.h>
#include <cstring>
#include <sys/times.h>
#include <sys/time.h>
#define ARRAYSIZE 26

using namespace std;
int mEncodingCode = 0;
typedef std::map<int,string> DictionaryHash;
DictionaryHash hashToFile;
DictionaryHash hashFromFile;

ofstream output_file;
ifstream input_file;

int rank, i,nprocs,file_open_error; 
FILE *fp;
MPI_File file_handle ; 
MPI_Status status ; 

MPI_Offset file_content_size_mpi,buf_size_proc_mpi,proc_offset_mpi;
int file_content_size,buf_size_proc,proc_offset,f_error;
int file_size,ret;
char *rank_file_name,*file_name;
char *file;
int type_of_data = sizeof(char);
char *meta = "meta.txt";


char arrayChar[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};

typedef struct Node {
	
	int mValue = -1;
	Node *mChild[ARRAYSIZE] = { NULL };
	
}NODE;


Node* allocateNode(int value) {
	NODE *newNode = (NODE*)calloc(1, sizeof(NODE));
	newNode->mValue = value;
	
	return newNode;
}

/*------------------------------checkFileExists--------------------------------------------------
Checks if a file exists before performing read or write operations
-------------------------------------------------------------------------------------------*/

bool checkFileExists(const std::string& name) 
{
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

/*------------------------------insertWord--------------------------------------------------
This function takes the input to be processed, searches for the word, and inserts the word to 
the trie structure if necessary and returns the encoded input.
-------------------------------------------------------------------------------------------*/

string insertWord(char input[], NODE *trie) {

	
	vector<int> encodedWord;
	vector<int> code;
	string finalCompressedContentAsString;
	ostringstream convert;
	NODE *temp_trie = trie;
	bool last_search_status = true;	
	string input_string(input);
	string  current_string, prefix_string, lastString;
	int prev_code, current_code, current_string_length, encoded_value, valueToAddInEnd, newValueForNewNode;
	int i = 0;

	int word_size = strlen(input);
	while (i < word_size) {	

		if(((input_string[i] - 'a') < 0) || ((input_string[i] - 'a') > ARRAYSIZE)){ // to check limits
			cout <<"unsupported character" <<  "'" << input_string[i] << "'"  << "ASCII value :" << (int)input_string[i] << " supported character [a-z]" << endl;
			MPI_Abort(MPI_COMM_WORLD, 0);
			exit(0);
		}
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
	
	
	i=0;
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

NODE* search() {
	return NULL;
}



/*------------------------------initializeSingleCharValue------------------------------------
As the program starts, the dictionary with characters a-z will be intitalized into the trie
-------------------------------------------------------------------------------------------*/


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

/*------------------------------readInputFile-------------------------------------------------
Reads the input file, splits the file based on the number of processors and returns the string 
that needs to be compressed by each processor
-------------------------------------------------------------------------------------------*/

char* readInputFile(char* input_file_name)
{
	ofstream meta_file;
	meta_file.open(meta,ios::out);
	
	
	MPI_File_open(MPI_COMM_WORLD, input_file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &file_handle); 
	if (file_open_error != MPI_SUCCESS)
	{
		char error_string[BUFSIZ];
		int length_of_error_string, error_class;
	 
		MPI_Error_class(file_open_error, &error_class);
		MPI_Error_string(error_class, error_string, &length_of_error_string);
		printf("%3d: %s\n", rank, error_string);
	 
		MPI_Error_string(file_open_error, error_string, &length_of_error_string);
		printf("%3d: %s\n", rank, error_string);
	 
		MPI_Abort(MPI_COMM_WORLD, file_open_error);
	} 
	MPI_File_get_size(file_handle, &file_content_size_mpi);
	
	
	 
	buf_size_proc_mpi = file_content_size_mpi / nprocs;
	
	
	

	buf_size_proc = (int)buf_size_proc_mpi;
	meta_file<<buf_size_proc;

	proc_offset_mpi = (MPI_Offset)buf_size_proc_mpi * rank;

	if(rank == nprocs-1)
	{
		buf_size_proc = file_content_size_mpi - proc_offset_mpi;
		
		
	}
	
	file = (char*) malloc (buf_size_proc + 1 );

	
	MPI_File_read_at(file_handle, proc_offset_mpi, file, buf_size_proc, MPI_CHAR, &status); 	
	
	
	
	file[buf_size_proc] = '\0';
	
	MPI_File_close(&file_handle) ;  
	meta_file.close();
	
	return file;
	
}

/*------------------------------putHashToFile-------------------------------------------------
The hash map generated during compression is written to multiple files based on the number of 
processors
-------------------------------------------------------------------------------------------*/

void putHashToFile()
{
	

	string file_name;	
	stringstream ss;
	ss << rank;
	file_name = ss.str();
	
	
	file_name = "hash_" + ss.str() ;
	file_name = file_name + ".txt";
	
	output_file.open(file_name.c_str(), ios::out);
	
	
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

/*------------------------------getHashFromFile-------------------------------------------------
The hash files generated during compression is read and filled into a hash map
-------------------------------------------------------------------------------------------*/
void getHashFromFile(int index)
{		

  
	string file_name,line;	
	stringstream ss;

	
	ss << index;
	file_name = ss.str();
	file_name = "hash_" + ss.str() ;
	file_name = file_name + ".txt";
	
	
	input_file.open(file_name.c_str(), ios::in);
	

	
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

/*------------------------------decompress-------------------------------------------------
Decompresses the encoded output using the hash map and writes the output onto a file
-------------------------------------------------------------------------------------------*/

void decompress(int i,int offset)
{
	 MPI_File fh;
	 char *output_file_name = "final_output.txt";
	 ostringstream op ;
	 MPI_File_open (MPI_COMM_WORLD, output_file_name, MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fh); 
	MPI_Offset displace = rank*offset*sizeof(char); 

	MPI_File_set_view (fh , displace , MPI_CHAR, MPI_CHAR, "native" ,MPI_INFO_NULL); 


	
 
	string file_name,line;	
	
	stringstream ss;	
	ss << rank;
	file_name = ss.str();
	file_name = "output_" + ss.str() ;
	file_name = file_name + ".txt"; 
	
	input_file.open(file_name.c_str(), ios::in);	

    if(!input_file.is_open()){		
	
		cout << "Output partial files failed to open. Exiting...........\n";
		exit(1);
	}
	
	string str;	
		
	while (std::getline(input_file, str))
	{

		stringstream ss(str);
	 
		while( ss.good() )
		{
			string substr;
			getline( ss, substr, ',' );			
			op<<hashFromFile[atoi(substr.c_str())];
		}
	}
	string op_string = op.str();
	const char *op_char = op_string.c_str();
	
	
	MPI_File_write(fh, op_char, op_string.size(), MPI_CHAR, &status);
 
	MPI_File_close(&fh ) ; 
}

/*------------------------------compress-------------------------------------------------
Compresses the passed string, and writes the encoded input into multiple files
-------------------------------------------------------------------------------------------*/

void compress(char* str)
{
	NODE *mRootNode = allocateNode(-1);
	initializeSingleCharValue(mRootNode);  // initialize values for single character

	string code;	
	string file_name;	
	stringstream ss;
	
	code = insertWord(str, mRootNode);

	
	
	ss << rank;
	file_name = ss.str();
	
	
	file_name = "output_" + ss.str() ;
	file_name = file_name + ".txt";
	
	output_file.open(file_name.c_str(), ios::out);
	output_file << code;
	output_file.close(); 
}


int main(int argc, char **argv) { 	
	
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	double compress_start_time,compress_start_time_min,compress_end_time,compress_end_time_max;
	double decompress_start_time,decompress_start_time_min,decompress_end_time,decompress_end_time_max;
	

	
	char *filename = "abc.txt";
	char *no_of_procs_file ="nprocs.txt";
	char *input;
	string line;
	string offset;
	int offset_int;
	int no_of_files;
	int *turn;
	
	ofstream no_of_procs_to_file;
	ifstream no_of_procs_from_file;
	ifstream meta_file;
	if(rank == 0)
	{
		no_of_procs_to_file.open(no_of_procs_file,ios::out);	
		no_of_procs_to_file << nprocs;
		no_of_procs_to_file << "\n";
		no_of_procs_to_file.close();
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	bool status = checkFileExists(no_of_procs_file);
	if(status == 0)
	{
			cout<<"File nprocs.txt doesnt exist. exiting......"<<endl;
			exit(1);
	}
		
	no_of_procs_from_file.open(no_of_procs_file,ios::in);
	
	getline(no_of_procs_from_file,line);
	
	no_of_files = atoi(line.c_str());
	

	turn = (int*)malloc(rank*sizeof(int));	
	
	for(i = 0; i<no_of_files;i++)
	{
		turn[i] = i%nprocs;
		
	}
	
	
	input = readInputFile("input1.txt");

	
	compress_start_time = MPI_Wtime();
	
	compress(input);
	
	compress_end_time = MPI_Wtime(); 
	
	MPI_Reduce(&compress_start_time, &compress_start_time_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	
	MPI_Reduce(&compress_end_time, &compress_end_time_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	
	if(rank ==0)
	{
	cout<<"\nTotal Compress Time" << compress_end_time_max - compress_start_time_min ;
	}
	
	
	
	putHashToFile();
	MPI_Barrier(MPI_COMM_WORLD);
	
		status = checkFileExists(meta);
		if(status == 0)
		{
			cout<<"Metadata file meta.txt doesnt exist. Exiting......"<<endl;
			exit(1);
		}
		meta_file.open(meta,ios::in);
		getline(meta_file,offset);
		
	
		meta_file.close();
	
	
	for(i=0;i<no_of_files;i++)
	{
		if(turn[i] == rank)
		{
			
			getHashFromFile(i);
			
			
			decompress_start_time = MPI_Wtime();
			
			decompress(i,atoi(offset.c_str()));
			
			decompress_end_time = MPI_Wtime(); 			
			
			
			
		}
	}
	
	MPI_Reduce(&decompress_start_time, &decompress_start_time_min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	
	MPI_Reduce(&decompress_end_time, &decompress_end_time_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	
	if(rank ==0)
	{
	cout<<"  Total Decompress Time " << decompress_end_time_max - decompress_start_time_min ;
	}

	
	
	
	MPI_Finalize();
	return 0;
	
	}