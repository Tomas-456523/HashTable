//implementation file for nodes

#include "Node.h" //must have node and student files
#include "Student.h"
using namespace std;

Node::Node(Student* _student, const string& _hash) { //create the node and assign it the given student and hash based on the student's ID
    student = _student;
    hash = _hash;
    nextNode = NULL;
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
const string& Node::getHash() { //get the hash associated with this node's student's ID
    return hash;
}
Node::~Node() { //delete the student when the node is deleted since the two are associated, make sure the students aren't used anywhere else
    delete student;
}