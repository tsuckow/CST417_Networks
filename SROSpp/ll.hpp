// Quick and dirty linked list
// Author: Thomas Suckow

#pragma once

#include <stdio.h>

template< typename T >
class ll
{
protected:
   struct Node
   {
      T m_item;
	   Node * m_next;
      
      Node( T item, Node * next ) : m_item(item), m_next(next)
      {
      }
   };
   
   class internal_iterator
   {
      friend ll;
      protected:
         Node * current;
      
      public:
         internal_iterator( Node * start ) : current( start )
         {
         }
      
         bool operator!= ( internal_iterator const & rhs ) 
         {
            return current != rhs.current;
         }
         
         void operator++()
         {
            current = current->m_next;
         }
         
         T & operator*()
         {
            return current->m_item;
         }
         
         T * operator->()
         {
            return &current->m_item;
         }
   };
   
   Node * m_head;
   
public:
   typedef internal_iterator iterator;

   ll() : m_head(0)
   {
   }
   
   ~ll()
   {
      while( m_head != 0 )
      {
         Node * next = m_head->m_next;
         delete m_head;
         m_head = next;
      }
   }
   
   void push_front( T const & item )
   {
      m_head = new Node( item, m_head );
   }
   
   void push_back( T const & item )
   {
      Node ** current = &m_head;
      while( *current != 0 )
      {
         current = &(*current)->m_next;
      }
      
      *current = new Node( item, 0 );
   }
   
   iterator begin()
   {
      return iterator( m_head );
   }
   
   iterator end()
   {
      return iterator( 0 );
   }
   
   iterator erase( iterator item )
   {
      Node ** current = &m_head;
      for( ; *current != 0 ; current = &(*current)->m_next )
      {
         if( item.current == *current )
         {
            *current = (*current)->m_next;
            
            delete item.current;
            
            return iterator( *current );
         }
      }
      
      //Not found.
      return end();
   }
};
