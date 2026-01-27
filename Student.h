//header file for students

#ifndef STUDENT
#define STUDENT

#include <string>

class Student {
public:
    Student(std::string& firstname, std::string& lastname, int _id, float _gpa); //constructs the student with all the given data
    ~Student(); //destructor (default, doesn't do anything)
    
    std::string& getName(int which); //returns first or last name based on if 0 or something else is passed for "which"
    int getID(); //return the student's id
    float getGPA(); //return the student's gpa
private:
    std::string firstName; //all the student's data
    std::string lastName;
    int id;
    float gpa;
};
#endif