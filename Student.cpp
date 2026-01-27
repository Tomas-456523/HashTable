//implementation file for students

#include "Student.h"
#include <string>
using namespace std;

Student::Student(string& firstname, string& lastname, int _id, float _gpa) { //constructs the student and sets all the data according to the given data
    firstName = firstname;
    lastName = lastname;
    id = _id;
    gpa = _gpa;
}
string& Student::getName(int which) { //returns the first name if 0 was passed, or last if any other int was given
    if (!which) { //!0 == !false == true, so return the first name
        return firstName;
    } //otherwise return the last name
    return lastName;
}
int Student::getID() { //return the student's id
    return id;
}
float Student::getGPA() { //return the student's gpa
    return gpa;
}
Student::~Student() { //default destructor
    
}