//header file for nodes

#ifndef NODE
#define NODE

class Student; //forward declare student class so we can use it here without #including Student.h

class Node {
public:
    Node(Student* _student); //constructor, requires student to point to, and sets nextNode to NULL to start
    ~Node(); //destructor which also deletes the student
    
    void setNext(Node* next); //sets the node which goes after this node and is gotten from getNext
    
    Student* getStudent(); //get the student associated with this node
    Node* getNext(); //get the node that goes after this one
private:
    Node* nextNode; //the next node that goes after this one in the linked list, defaults to NULL in constructor
    Student* student; //the student pointed to by this node
};
#endif