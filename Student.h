//header file for students

#ifndef STUDENT
#define STUDENT

class Student {
public:
    Student(char* firstname, char* lastname, int _id, float _gpa); //constructs the student with all the given data
    ~Student(); //destructor (default, doesn't do anything)
    
    char* getName(int which); //returns first or last name based on if 0 or something else is passed for "which"
    int getID(); //return the student's id
    float getGPA(); //return the student's gpa
private:
    char firstName[255]; //all the student's data
    char lastName[255];
    int id;
    float gpa;
};
#endif