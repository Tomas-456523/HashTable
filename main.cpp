/* Tomas Carranza Echaniz
*  1/29/2026
*  This program is a student database that uses a hash table which handles collisions using chaining. Chains have a maximum
*  length of 3. When this is exceeded, the hash table length is doubled and all the nodes are rehashed. The hash algorithm
*  used is SHA-3; all nodes are assigned a hash on creation based on their student ID. The user can ADD a new student, which
*  will be added to the table according to its hash. You can DELETE the student, and PRINT all the students' data. You can
*  also print the AVERAGE of all their GPAs, ask for HELP to print all the valid commands, or QUIT the program. The user can
*  also RELOAD the name files if necessary.
*
*  Hash tables are efficient because you don't have to iterate across every student to check if they have the right ID,
*  you just plug in the ID and it gives you the index!
*/

#include <iostream>
#include <limits>
#include <cstring>
#include <iomanip>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <cctype>
#include "Student.h"
#include "Node.h"
#include "SHA3.h"
using namespace std;

//for ignoring faulty input and extra characters, functionality taken from my previous projects
void CinIgnoreAll(bool force = false) {
    if (!cin || force) { //only go if input is faulty, otherwise forces the user to input an extra enter
        cin.clear(); //clears error flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); //ignores all characters up until a new line
    }
}

//capitalizes the given string for easier command interpretation
void AllCaps(string& word) {
    for (size_t i = 0; i < word.size(); i++) { //sets all the characters to a capitalized unsigned version of the char (unsigned because some systems sign the chars)
        word[i] = toupper((unsigned char)word[i]);
    }
}

//for getting an integer from the player, ususally the id of a student
int makeNum(bool id = true) { //id bool is for giving id error or normal error
    int num = 0;
    while (true) {
        cout << "\n> ";
        cin >> num; //gets the integer
        if (cin) { //return the number and end the loop if input was valid
            CinIgnoreAll(true); //removes the newline character after valid integer input
            return num;
        } else { //otherwise give error message and try again
            if (id) { //id-specific error "\nID must be an integer."
                cout << "\nID m";
            } else { //generic integer error "\nMust be an integer."
                cout << "\nM";
            }
            cout << "ust be an integer.";
        }
        CinIgnoreAll(); //removes the newline character or invalid input
    }
}

//checks the table for if it's empty based on if it's all NULL nodes
bool getEmpty(Node** table, size_t tablelen) {
    for (size_t i = 0; i < tablelen; i++) { //iterates through the table, returns false, table is not empty, upon reaching a non-NULL node
        if (table[i] != NULL) {
            return false;
        }
    } //return true, table is empty, if no non-NULL nodes were found
    return true;
}

//get an index in the hash table based on the given hash and table length, based on Absorb() in SHA3.cpp
size_t deHash(const string& hash, size_t tablelen) {
    size_t index = 0; //create an index
    //de-stringifies hash so we can use the bytes
    const uint8_t* hashBytes = reinterpret_cast<const uint8_t*>(&hash[0]);
    size_t hashSize = hash.size(); //the full size of the hash
    //the amount of bytes size_t takes up (the size of size!)
    const size_t sizesize = sizeof(size_t);

    while (hashSize > 0) { //xors the hash into the index in size-sized chunks (so we don't just waste the rest of the hash we worked so hard to make)
        size_t hashChunk = 0; //creates an empty chunk to memcpy into
        size_t chunkSize = min(hashSize, sizesize); //get the size of the chunk (sizesize for everything except the end of the hash, so we don't go over the end of hash if hashSize%sizesize != 0)
        memcpy(&hashChunk, hashBytes, chunkSize); //get the chunkSize-sized chunk of the hash
        index ^= hashChunk; //xor the chunk into the index
        hashBytes += chunkSize; //advance the hash pointer forward so we read the next chunk next loop
        hashSize -= chunkSize; //subtract from the hashSize so we know how big the last chunk is when we get there
    } //xors the index's top half into the bottom half so that both halves matter equally in the final modulo, because otherwise the top half wouldn't matter as much
    index ^= index >> sizesize*4+1; //we use that amount cause bitshift uses bit amounts, and 8 bits = 1 byte, and we want to shift it by half, so we multiply by 4, and then we add one to reduce symmetry. sizesize*4+1 is computed before >>
    //mix the index with Knuth's constant, well known for de-linearizing hashes (so more random, less collisions). SHA-3 is very good at hashing, but since we fold it so much a lot of the entropy is gone, so we also do this
    index *= 11400714819323198485ULL;
    return index % tablelen; //modulo the index based on tablelen to stay within bounds and return that
}

//place the node into the hash table at the given index and return true if we need to rehash, based on chain length
bool placeNode(Node* node, Node** table, size_t index) {
    if (table[index] == NULL) { //if the bucket at the given index is empty, we just tell the table in main the node goes here
        table[index] = node;
        return false; //no need to rehash because the chain length is 1 guaranteed!
    }
    Node* current = table[index]; //finds the beginning node
    int chainlen = 2; //how long the chain is, starts at 2 because it includes the first and (new) last nodes
    for (; current->getNext() != NULL;) { //iterate through the chain until we reach the last node
        current = current->getNext(); //go to the next node
        chainlen++; //increment the chain length since we checked one more node
    }
    current->setNext(node); //places the node after the previous last node
    return chainlen > 3; //if the chain length exceeds 3, we say to rehash
}

//double the table size and rearrange all the nodes in into new buckets, when we exceed the maximum chain length of 3
void reHash(Node**& table, size_t& tablelen) {
    vector<Node*> nodes; //vector of all nodes which they're chucked into until they're all rehashed
    for (size_t i = 0; i < tablelen; i++) { //iterates through table indices
        for (Node* current = table[i]; current != NULL; current = current->getNext()) { //starting at the current node, iterates through the chain until it reaches the NULL end
            nodes.push_back(current); //add the current node to the nodes vector
        }
    }
    //continues looping until we successfully rehash (usually this just runs once, but this accounts for the unlikely edge case of accidentally creating another 4 chain while rehashing)
    for (bool continuing = true; continuing;) {
        delete[] table; //deletes the old overcrowded hash table
        for (Node* node : nodes) { //nullify all node linkages since they're gonna be in different buckets now
            node->setNext(NULL);
        }
        tablelen *= 2; //double the hash table length and create a new shiny hash table with the new length
        table = new Node*[tablelen](); //the new shiny hash table
        continuing = false; //assume success to start
        for (Node* node : nodes) { //sort all the nodes into the new hash table based on their hash and the new length
            size_t i = deHash(node->getHash(), tablelen); //gets the index based on the hash
            if (placeNode(node, table, i)) { //puts the node in the new spot and checks for a chain length greater than 3
                continuing = true; //if we detect too long a chain, we must do all that again, so we continuing!
                break; //break, no need to sort the rest of the nodes if we're just gonna unsort them immediately
            }
        }
    }
}

//reads the data from a text file into a vector of strings (each line in the file is an item in the vector)
void readTxtData(const string& file, vector<string>& lines) { //needs the name of the file and the vector to write into
    lines.clear(); //removes any existing data from the given vector, so we don't just inflate it on every RELOAD
    ifstream txt(file); //opens the file
    if (!txt) { //says error message if we couldn't open the file for some reason
        cout << "\nError encountered while opening " << file << ".";
    } //no need to return because it just fails the for loop condition immediately and then the function ends anyway
    //reads each line from txt and adds them to the vector of lines
    for (string line; getline(txt, line); lines.push_back(line));
}

//takes the two names vectors and reads the associated files into them, (item in vector = line in file)
void loadNames(vector<string>& firstnames, vector<string>& lastnames, bool initial = false) {
    readTxtData("firstnames.txt", firstnames); //reads the files into their corresponding vectors
    readTxtData("lastnames.txt", lastnames);
    bool faulty1 = firstnames.empty(); //gives errors if the vectors are empty for whatever reason
    bool faulty2 = lastnames.empty();
    if (faulty1 && faulty2) { //gives specific error, which one or if both files are faulty/empty
        cout << "\nList of first and last names empty. GENERATE command disabled."; //both faulty
    } else if (faulty1) { //first names faulty
        cout << "\nList of first names empty. GENERATE command disabled.";
    } else if (faulty2) { //last names faulty
        cout << "\nList of last names empty. GENERATE command disabled.";
    } else if (!initial) { //otherwise, success text! (unless it's the first one, because that runs without player input, so they don't need to know)
        cout << "\nSuccessfully reloaded first and last name lists!";
    }
}

//creates a new student and a new node pointing to it, needs the table as input so we can make sure to not repeat IDs, because that would cause infinite rehashing (since upon reaching 3 collisisons, the same 4 nodes would be rehashed into the same bucket again)
Node* createStudent(Node** table, size_t tablelen) {
    string firstname;
    string lastname;
    int id;
    float gpa;

    //get the student's first (and middle if applicable) names
    cout << "\nEnter first (and middle) name for new student.\n> ";
    getline(cin, firstname);

    //get the student's last name(s)
    cout << "\nEnter last name(s) for new student.\n> ";
    getline(cin, lastname);

    //get the student's ID
    cout << "\nEnter " << firstname << "'s student ID.";
    bool continuing = true; //continues until valid input is given
    while (continuing) {
        id = makeNum(); //gets the ID using the number getting function, "\n> " is provided there
        continuing = false; //assumes valid input to start since makeNum doesn't return faulty input
        size_t i = deHash(SHA3::Hash(id), tablelen); //gets the index that the given ID would be placed at

        //iterates through the chain at index i and goes to the next one each iteration until it meets a null node, that being the end
        for (Node* current = table[i]; current != NULL; current = current->getNext()) { //in order to check if the currently considered ID is taken for reasons stated above the createStudent function
            if (current->getStudent()->getID() == id) { //if the IDs match that's bad
                continuing = true; //we keep continuing and get a new ID from the user because the IDs conflict
                Student* student = current->getStudent(); //get the student so we don't getStudent() twice
                cout << "\nID " << id << " is taken by " << student->getName(0) << " " << student->getName(1) << "."; //error message; shows who is causing the conflict
                break; //break since we know there's a conflict alredy so more checks would waste valuable time
            }
        }
    }

    //get the student's gpa
    cout << "\nEnter " << firstname << "'s GPA.";
    for (continuing = true; continuing;) { //continues until valid input is given
        cout << "\n> ";
        cin >> gpa; //gets the gpa float
        if (cin) { //end loop if valid input was given
            continuing = false;
        } else { //error message otherwise
            cout << "\nGPA must be a float.";
        }
        CinIgnoreAll(true); //removes the newline character of invalid input
    }

    //creates a new student using the given data
    Student* student = new Student(firstname, lastname, id, gpa);
    //create and return a new node pointing to the new student, featuring a hash generated using the ID
    return new Node(student, SHA3::Hash(id));
}

//pseudorandomly generate a student using the name files, the stored genID, and a GPA between 0 and 4.5
void generateStudent(Node**& table, size_t& tablelen, vector<string>& firstnames, vector<string>& lastnames, int& genID) {
    string firstname = firstnames[rand()%firstnames.size()]; //use the lists to choose one of each type of name
    string lastname = lastnames[rand()%lastnames.size()];

    int id; //the ID this student will have, we get it by incrementing genID until we find an unused ID
    size_t index; //the bucket of the hash table the student will be placed into
    string hash; //the student's hash based on the ID

    for (bool continuing = true; continuing;) { //continues until valid ID is found
        id = genID++; //gets the next ID and then increments it (the one in main())
        hash = SHA3::Hash(id); //gets the hash of the new ID
        continuing = false; //assumes valid ID to start
        index = deHash(hash, tablelen); //gets the index that the given ID would be placed at

        //iterates through the chain at index i and goes to the next one each iteration until it meets a null node, that being the end
        for (Node* current = table[index]; current != NULL; current = current->getNext()) { //in order to check if the currently considered ID is taken for reasons stated above the createStudent function
            if (current->getStudent()->getID() == id) { //if the IDs match that's bad
                continuing = true; //we keep continuing and get a new ID because the IDs conflict
                break; //break since we know there's a conflict alredy so more checks would waste valuable time
            }
        }
    } //generates a random gpa between 0.0 and 4.5
    float gpa = (rand()%450)/100.0;

    //creates a new student and node using the generated data
    Student* student = new Student(firstname, lastname, id, gpa);
    Node* node = new Node(student, hash);
    
    //place the student at the found index
    if (placeNode(node, table, index)) { //if placeNode says we exceeded the chain length limit, we rehash!
        reHash(table, tablelen);
    }
}

//get an amount from the player, and then generate that many new students
void initGeneration(Node**& table, size_t& tablelen, vector<string>& firstnames, vector<string>& lastnames, int& genID) {
    if (!firstnames.size() || !lastnames.size()) { //if our name lists are empty due to file issues, we can't generate any students
        cout << "\nNo valid names available; can't generate students."; //so we give an error and return, no generating allowed
        return;
    }
    cout << "\nHow many students to generate?";
    int amount = makeNum(false); //gets how many students to generate
    cout << "\n"; //formatting!
    for (int i = 0; i < amount; i++) { //generates as many students as specified
        generateStudent(table, tablelen, firstnames, lastnames, genID);
        cout << "\rProgress: " << i * 100.0 / amount << "%" << flush; //prints the progress percentage in float form, for very large amounts (also overwrites the last percentage printing, looks more progress bar-y that way)
    }
    cout << "\rSuccessfully generated " << amount << " student"; //overwrites the progress indicator with the success message! (looks better by overwriting rather than a new line)
    if (amount != 1) { //make it plural if it wasn't specifically 1 student
        cout << "s";
    }
    cout << "!"; //exclamation mark!
}

//creates a new student node which the player can manually set the values for, and inserts it into the hash table
void makeStudent(Node**& table, size_t& tablelen) {
    Node* newguy = createStudent(table, tablelen); //create the student with all their values and get their node
    size_t index = deHash(newguy->getHash(), tablelen); //find where to put the student node
    //try to place the student; if doing so creates a chain longer than 3 nodes, we rehash the hash table!
    if (placeNode(newguy, table, index)) {
        reHash(table, tablelen); //rehash the hash table!
    } //success text!
    cout << "\nSuccessfully created " << newguy->getStudent()->getName(0) << "!";
}

//prints the given student (with option to put the data in a new line)
void printStudent(Student* student, bool newline = true) {
    if (newline) { //prints the new line if we need to
        cout << "\n";
    } //prints all the student's data
    cout << student->getName(0) << " " << student->getName(1) << " (" << student->getID() << ") - GPA of " << student->getGPA();
}

//uses id to find and delete a node in the hash table
void deleteNode(Node** table, size_t tablelen) {
    cout << "\nEnter ID of student to delete.";
    int id = makeNum(); //gets the ID from the player to search for
    size_t index = deHash(SHA3::Hash(id), tablelen); //creates a hash based on the ID and immediately dehashes it to get an index to start searching in

    Node* previous = NULL; //we store the previous node so we can bridge the gap created by deleting middle or end nodes
    //check the chain starting from the index position in the table, continue until going off the end of the chain (entering NULL territory)
    for (Node* current = table[index]; current != NULL; current = current->getNext()) {
        if (current->getStudent()->getID() == id) { //delete the node if its student's ID matches
            if (current == table[index]) { //if it's the first one in the chain, update the table in main() by bringing the next node into the table at the index
                table[index] = current->getNext();
            } else { //if it's the middle or end one, bridge the gap by setting previous's next to the deleted node's next (1 -> 2 -> 3)  ==>  (1 ->   -> 3)  ==>  (1 -> 3)
                previous->setNext(current->getNext());
            }
            Student* student = current->getStudent(); //avoids extra student getting
            cout << "\nDeleted " << student->getName(0) << " " << student->getName(1) << "."; //deletion success text!
            delete current; //deletes the student
            return; //returns because there's nothing else to do here
        }
        previous = current; //updates previous, because the next current's previous is going to be current
    } //error, no student with ID id found
    cout << "\nNo student found with ID " << id << ".";
}

//prints the average gpa of all the students
void average(Node** table, size_t tablelen) {
    double sum = 0; //the sum of all gpas, double for extra precision in case we have like a million students
    int count = 0; //how many students in table
    for (size_t i = 0; i < tablelen; i++) { //iterates through table indices
        for (Node* current = table[i]; current != NULL; current = current->getNext()) { //starting at the current node, iterates through the chain until it reaches the NULL end
            sum += current->getStudent()->getGPA(); //add the gpa to the sum total
            count++; //increment the count because there's one more student to count
        }
    }
    if (!count) { //if we didn't find anyone, we give error message and return
        cout << "\nThere are no students with GPAs to average. (type ADD for add)";
        return;
    } //print the average gpa to two decimals of precision
    cout << "\nAverage GPA: " << sum / count;
}

//print all the students' data by iterating through the table and iterating through any chains it finds
void printAll(Node** table, size_t tablelen) {
    if (getEmpty(table, tablelen)) { //check if there's any students to print
        cout << "\nThere are no students to print."; //if not, error and return
        return;
    }
    cout << "\nStudents:";
    for (size_t i = 0; i < tablelen; i++) { //iterates through table indices
        for (Node* current = table[i]; current != NULL; current = current->getNext()) { //starting at the current node, iterates through the chain until it reaches the NULL end
            printStudent(current->getStudent()); //prints the current student data
        }
    }
}

//the main loop
int main() {
    size_t tableLen = 100; //the length of the hash table which gets doubled when chain length exceeds 3 on the same index
    Node** table = new Node*[tableLen](); //the hash table of linked list chains

    vector<string> firstNames; //the vectors of names that are used to pseudorandomly generate students
    vector<string> lastNames;

    int genID = 1; //the ID used when randomly generating a student, gets incremented after every generation

    srand(time(NULL)); //seed the random number generation with the time, so the random student generation can use it

    //welcome message with instructions, also sets float printings to 2 decimal points of precision
    cout << "\nHello I am Harry the hash table!\nI am managing a database of students.\nType HELP for help." << fixed << setprecision(2);
    loadNames(firstNames, lastNames, true); //loads the names from the files "firstnames.txt" and "lastnames.txt"
    cout << "\n\nThere are currently no students. (type ADD for add)";
    
    string command; //the command that the user inputs into (now outside the loop! how exciting!)
    //continues until continuing is falsified (by typing QUIT)
    for (bool continuing = true; continuing;) {
        cout << "\n> "; //thing for the player to type after
        
        getline(cin, command); //gets the player input, up to 255 characters
        
        AllCaps(command); //capitalizes the command for easier interpretation
        
        //calls function corresponding to the given command word
        if (command == "ADD") { //add student
            makeStudent(table, tableLen);
        } else if (command == "GENERATE") { //randomly generate new student(s)
            initGeneration(table, tableLen, firstNames, lastNames, genID);
        } else if (command == "DELETE") { //delete student
            deleteNode(table, tableLen);
        } else if (command == "PRINT") { //print all students
            printAll(table, tableLen);
        } else if (command == "AVERAGE") { //print average gpa of all students
            average(table, tableLen);
        } else if (command == "RELOAD") { //reload name files
            loadNames(firstNames, lastNames);
        } else if (command == "HELP") { //print all valid command words
            cout << "\nYour command words are:\nADD      - Manually create a new student.\nGENERATE - Randomly generate a given amount of students.\nDELETE   - Delete an existing student by ID.\nPRINT    - Print the data of all students.\nAVERAGE  - Calculate the average GPA of all students.\nRELOAD   - Reload the two name files.\nHELP     - Print all valid commands.\nQUIT     - Exit the program.";
        } else if (command == "QUIT") { //quit the program
            continuing = false; //leave the main player loop
        } else { //give error message if the user typed something unacceptable
            cout << "\nInvalid command \"" << command << "\". (type HELP for help)";
        }
    }

    //says bye
    cout <<"\nPeace out.\n";

    //deletes all the nodes for good practice, iterates through the table and from there iterates through the individual chains
    for (size_t i = 0; i < tableLen; i++) {
        Node* next = NULL; //stores the next node temporarily so we can delete the current one
        for (Node* current = table[i]; current != NULL; current = next) { //starts at the first node in bucket i and iterates through until the end of the chain
            next = current->getNext(); //go to the next node
            delete current; //deletes the node
        }
    }

    delete[] table; //deletes the table structure itself
}
