#ifndef NODEH
#define NODEH

#include <stdlib.h>

class Node{
	public:
		Node * next;
		Node * prev;
		void * target;

		Node( );
		Node * GetNext( );
		void   SetNext( Node * node );
		Node * GetPrev( );
		void   SetPrev( Node * node );
		void * GetTarget( );
		void   SetTarget( void * target );
};

#endif
