//implementation file for nodes

#include "Node.h" //must have node and student files
#include "Student.h"

Node::Node(Student* _student) { //create the node and assign it the given student
    student = _student;
    nextNode = 0; //next node starts as 0 which is equal to NULL except NULL requires #including stuff  
}
void Node::setNext(Node* next) { //set the node that goes after this one
    nextNode = next;
}
Student* Node::getStudent() { //get the student associated with this node
    return student;
}
Node* Node::getNext() { //get the node this node points to
    return nextNode;
}
Node::~Node() { //delete the student when the node is deleted since the two are associated, make sure the students aren't used anywhere else
    delete student;
}