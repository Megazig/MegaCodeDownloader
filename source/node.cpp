#include "node.h"

Node::Node( ) {
	this->next = NULL;
	this->prev = NULL;
	this->target = NULL;
}

Node * Node::GetNext( ) {
	return this->next;
}

void   Node::SetNext( Node * node ) {
	this->next = node;
}

Node * Node::GetPrev( ) {
	return this->prev;
}

void   Node::SetPrev( Node * node ) {
	this->prev = node;
}

void * Node::GetTarget( ) {
	return this->target;
}

void   Node::SetTarget( void * target ) {
	this->target = target;
}
