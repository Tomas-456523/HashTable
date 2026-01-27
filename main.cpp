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

//for getting the id of a student since that happens twice
int makeID() {
    int id = 0;
    while (true) {
        cout << "\n> ";
        cin >> id; //gets the id integer
        if (cin) { //return the id and end the loop if input was valid
            CinIgnoreAll(true); //removes the newline character after valid integer input
            return id;
        } else { //otherwise give error message and try again
            cout << "\nID must be an integer.";
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

/*void placeNode(Node** table, Node* node) {
    for (; node->getNext() != NULL; node = node->getNext());
    node->setNext();
}*/

//creates a new student and a new node pointing to it, needs start of linked list as input so we can make sure to not repeat IDs, as that would lead to annoying behavior, such as needing to delete the first one before being able to delete the second one that uses that ID
Node* createStudent(Node* start) {
    char firstname[255];
    char lastname[255];
    int id;
    float gpa;

    //get the student's first (and middle if applicable) names
    cout << "\nEnter first (and middle) name for new student.";
    bool continuing = true; //continues until valid input is given
    while (continuing) {
        cout << "\n> ";
        cin.getline(firstname, 255);
        if (cin) { //end loop if input was valid
            continuing = false;
        }
        else { //error message
            cout << "\nInput too long.";
        }
        CinIgnoreAll(); //clear any faulty or extra input given
    }

    ///get the student's last name(s)
    cout << "\nEnter last name(s) for new student.";
    continuing = true; //continues until valid input is given
    while (continuing) {
        cout << "\n> ";
        cin.getline(lastname, 255);
        if (cin) { //end loop if input was valid
            continuing = false;
        }
        else { //error message
            cout << "\nInput too long.";
        }
        CinIgnoreAll(); //clear any faulty or extra input given
    }

    //get the student's id
    cout << "\nEnter " << firstname << "'s student ID.";
    continuing = true; //continues until valid input is given
    while (continuing) {
        id = makeID(); //gets the id using the already established id getting function, "\n> " is provided there
        continuing = false; //assumes valid input to start since makeID can return no faulty input
        //iterates through the linked list using current which starts at starts and goes to the next one each iteration until it meets a null node, that being the end
        for (Node* current = start; current != NULL; current = current->getNext()) { //in order to check if the currently considered ID is taken for reasons stated above the createStudent function
            if (current->getStudent()->getID() == id) { //if the IDs match that's bad
                continuing = true; //we keep continuing and get a new ID from the user because the IDs conflict
                cout << "\nID " << id << " is taken by " << current->getStudent()->getName(0) << "."; //error message; shows who is causing the conflict
                break; //break since we know there's a conflict alredy so more checks would waste valuable time
            }
        }
    }

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
    Student* student = new Student(&firstname[0], &lastname[0], id, gpa);
    //create and return a new node pointing to the new student
    return NULL; /*new Node(student);*/
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

//deletes the current node, and returns the node after the given one in order to bridge the gap created as a result (we need to pass by reference so we can nullify it from here)
Node* deleteNode(Node*& current, int id = 0, bool first = 0) {
    if (current == NULL) { //return if we ran out of students
        if (first) { //if it's the first one that means there is actually literally nobody to delete
            cout << "\nThere are no students to delete. (type ADD for add)";
        } else { //if it's not the first check, this is after the last node and there were no matching students along the way
            cout << "\nThere are no students with ID " << id << ".";
        }
        return NULL;
    }
    if (first) { //get the id if it's the first student so we know who to delete
        cout << "\nEnter ID of student to delete.";
        id = makeID();
    }
    
    //if the current student node has the needed id, we found something to delete!
    if (current->getStudent()->getID() == id) {
        cout << "\nSuccessfully deleted " << current->getStudent()->getName(0) << "!"; //success message
        Node* next = current->getNext(); //stores next in case we delete current so we can access it in that situation
        delete current; //does the deleting
        if (first) { //update current node if it's the first one so int main is updated since the first node is there as well
            current = next;
        }
        return next; //return the next node so we can relink the list in case of successful deletion
    } else {
        Node* next = current->getNext(); //I have to define next first for same reasons as when calling next recursive call in addNode
        current->setNext(deleteNode(next, id)); //sets next to whatever the next recursive call returns, usually the next node again
    }

    if (first && current == NULL) { //prints if we have deleted the last student in the database
        cout << "\nThere are no students left.";
    }

    return current;
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
    Node** table = new Node*[tableLen](); //the hash table of linked lists

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
            addNode(table);
        } else if (!strcmp(command, "GENERATE")) { //randomly generate new student(s)

        } else if (!strcmp(command, "DELETE")) { //delete student
            deleteNode(table, 0, true);
        } else if (!strcmp(command, "PRINT")) { //print all students
            printAll(table, tableLen);
        } else if (!strcmp(command, "AVERAGE")) { //print average gpa of all students
            average(table);
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

    //IMPLEMENT THIS LATER
    //deletes all the nodes for good practice, iterates until the node is null meaning they're all deleted and goes to the stored next node at the end of each iteration

    /*Node* next = NULL; //stores the next node temporarily so we can delete the current one
    for (; first != NULL; first = next) { //this is the same as using a while loop but it looks cooler :)
        next = first->getNext(); //go to the next one
        delete first; //deletes the node
    }*/

    delete[] table;
}