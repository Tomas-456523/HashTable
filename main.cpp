/* Tomas Carranza Echaniz
*  1/26/2026
*  This program is a student database that uses a hash table which handles collisions using chaining. The hash algorithm
*  used is SHA-3. The user can ADD a new student, which will be added to the list in increasing ID order. You can DELETE
*  the student, and PRINT all the students' data. You can also print the AVERAGE of all their GPAs, ask for HELP to print
*  all the valid commands, or QUIT the program.
*
*  Hash tables are efficient because you don't have to iterate across every student to check if they have the right ID,
*  you just plug in the name and it gives you the index!
*/

#include <iostream>
#include <limits>
#include <cstring>
#include <iomanip>
#include <string>
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

//capitalizes the given charray for easier command interpretation
void AllCaps(char* word) {
    for (int i = 0; i < strlen(word); i++) { //sets all the characters to a capitalized unsigned version of the char (unsigned because some systems sign the chars)
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
    for (int i = 0; i < tablelen; i++) { //iterates through the table, returns false, table is not empty, upon reaching a non-NULL node
        if (table[i] != NULL) {
            return false;
        }
    } //return true, table is empty, if no non-NULL nodes were found
    return true;
}

//get an index in the hash table based on the given hash and table length
size_t deHash(const string& hash, size_t tablelen) {
    size_t index; //create an index and write as much data from the hash into the index as possible
    memcpy(&index, &hash[0], sizeof(size_t));
    return index % tablelen; //modulo the index based on tablelen to stay within bounds and return that
}

/*void placeNode(Node** table, Node* node) {
    for (; node->getNext() != NULL; node = node->getNext());
    node->setNext();
}*/

//creates a new student and a new node pointing to it, needs the table as input so we can make sure to not repeat IDs, because that would cause infinite rehashing (since upon reaching 3 collisisons, the same 4 nodes would be rehashed into the same bucket again)
Node* createStudent(Node** table, size_t tablelen) {
    string firstname;
    string lastname;
    int id;
    float gpa;

    //get the student's first (and middle if applicable) names
    cout << "\nEnter first (and middle) name for new student.";
    getline(cin, firstname);

    ///get the student's last name(s)
    cout << "\nEnter last name(s) for new student.";
    getline(cin, lastname);

    //get the student's ID
    cout << "\nEnter " << firstname << "'s student ID.";
    continuing = true; //continues until valid input is given
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
    continuing = true; //continues until valid input is given
    while (continuing) {
        cout << "\n> ";
        cin >> gpa; //gets the gpa float
        if (cin) { //end loop if valid input was given
            continuing = false;
        }
        else { //error message otherwise
            cout << "\nGPA must be a float.";
        }
        CinIgnoreAll(true); //removes the newline character of invalid input
    }

    //creates a new student using the given data
    Student* student = new Student(firstname, lastname, id, gpa);
    //create and return a new node pointing to the new student
    return new Node(student);
}

//creates a new student node and adds it into the linked list according to increasing id order
void addNode(Node*& current, Node* newguy = NULL) {
    if (newguy == NULL) { //if we just called addNode, the person we're adding will be NULL so we must set them at the start
        newguy = createStudent(current);
        if (current != NULL && newguy->getStudent()->getID() < current->getStudent()->getID()) { //if the new student's id is less than the current first one, we put the new node at the start
            newguy->setNext(current); //makes the new first node point to the old first node
            current = newguy; //update main's starting node
            cout << "\nSuccessfully added " << newguy->getStudent()->getName(0) << " before " << newguy->getNext()->getStudent()->getName(0) << "!"; //success message
            return;
        }
    }
    if (current == NULL) { //I always check the following node, so we will only reach this if we input a null node from main, meaning we have no nodes
        current = newguy; //thus, we just set the node in main to the new student!
    //if we're at the end of the linked list (meaning the next node is null), or the next one has a greater ID, meaning we're at the point where we can place the new node in proper ID order
    } else if (current->getNext() == NULL || current->getNext()->getStudent()->getID() > newguy->getStudent()->getID()) {
        newguy->setNext(current->getNext()); //since the new student will be the new next node, we make it point to the old next node
        current->setNext(newguy); //make the node we're at point to the new next node
    } else { //otherwise keep checking for a valid position; continue the loop!
        Node* next = current->getNext(); //I can't just put this all in one line, I have to define this and then pass it into the next call, because getNext() returns a Node*, but I need to pass a Node*&, but I can't just pass a pointer I got from a function for that. "cannot bind non-const lvalue reference of type 'Node*&' to an rvalue of type 'Node*'"
        addNode(next, newguy); //check the next node and carry over the new student's data
        return; //returns because we didn't add the student yet
    }
    
    cout << "\nSuccessfully added " << newguy->getStudent()->getName(0); //common beginning of success message
    if (current != newguy) { //if it isn't the only node, we print which node we put it after (current)
        cout << " after " << current->getStudent()->getName(0);
    }
    cout << "!"; //exclamation mark!
}

//prints the given student (with option to put the data in a new line)
void printStudent(Student* student, bool newline = true) {
    if (newline) { //prints the new line if we need to
        cout << "\n";
    } //prints all the student's data
    cout << student->getName(0) << " " << student->getName(1) << " (" << student->getID() << ") - GPA of " << fixed << setprecision(2) << student->getGPA();
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
    cout << "\nAverage GPA: " << fixed << setprecision(2) << sum / count;
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
    size_t tableLen = 100; //the length of the hash table which gets doubled when 3+ collisions happen on the same index
    Node** table = new Node*[tableLen](); //the hash table of linked list chains

    //welcome message with instructions
    cout << "\nHello I am Harry the hash table!\nI am managing a database of students.\nType HELP for help.\n\nThere are currently no students. (type ADD for add)";
    
    //continues until continuing is falsified (by typing QUIT)
    bool continuing = true;
    while (continuing) {
        char command[255]; //the command that the user inputs into
        
        cout << "\n> "; //thing for the player to type after
        
        cin.getline(command, 255); //gets the player input, up to 255 characters
        
        AllCaps(&command[0]); //capitalizes the command for easier interpretation
        
        //calls function corresponding to the given command word
        if (!strcmp(command, "ADD")) { //add student
            addNode(table, tableLen);
        } else if (!strcmp(command, "GENERATE")) { //randomly generate new student(s)

        } else if (!strcmp(command, "DELETE")) { //delete student
            deleteNode(table, tableLen);
        } else if (!strcmp(command, "PRINT")) { //print all students
            printAll(table, tableLen);
        } else if (!strcmp(command, "AVERAGE")) { //print average gpa of all students
            average(table, tableLen);
        } else if (!strcmp(command, "HELP")) { //print all valid command words
            cout << "\nYour command words are:\nADD      - Manually create a new student.\nGENERATE - Randomly generate a given amount of students.\nDELETE   - Delete an existing student by ID.\nPRINT    - Print the data of all students.\nAVERAGE  - Calcuate the average GPA of all students.\nHELP     - Print all valid commands.\nQUIT     - Exit the program.";
        } else if (!strcmp(command, "QUIT")) { //quit the program
            continuing = false; //leave the main player loop
        } else { //give error message if the user typed something unacceptable
            cout << "\nInvalid command \"" << command << "\". (type HELP for help)";
        }
        
        CinIgnoreAll(); //ignore any invalid or extra input that may have been typed this time
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