//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//        This file is part of E-Cell Simulation Environment package
//
//                Copyright (C) 1996-2002 Keio University
//                Copyright (C) 2005 The Molecular Sciences Institute
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//
// E-Cell is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// E-Cell is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with E-Cell -- see the file COPYING.
// If not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
//END_HEADER
//
// written by Eiichiro Adachi
// modified by Koichi Takahashi
//


#ifndef __DYNAMICPRIORITYQUEUE_HPP
#define __DYNAMICPRIORITYQUEUE_HPP
#include <vector>
#include <algorithm>
#include <map>
#include <tr1/unordered_map>

//#include "Util.hpp"

template < class T >
struct PtrGreater
{
    bool operator()( T x, T y ) const { return *y < *x; }
};


template < typename Item >
class DynamicPriorityQueue
{
  

public:

    typedef unsigned long long Serial;

    typedef std::vector< Item >    ItemVector;
    typedef std::vector< Item* >   ItemPtrVector;

    typedef typename ItemVector::size_type       Index;

    typedef std::vector< Index >  IndexVector;
//    typedef std::tr1::unordered_map<const Serial, Index> IndexMap;
    typedef std::map<const Serial, Index> IndexMap;


    DynamicPriorityQueue();
  
    inline void move( const Index index );

    void moveTop()
    {
	moveDownPos( 0 );
    }

    const Index getIndex( const Serial serial ) const
    {
	return this->indexMap[ serial ];
    }

    const Index getSerialByIndex( const Index index ) const
    {
//	return this->serialMap[ index ];
    }

    const Index getTopIndex() const 
    {
	return getItemIndex( this->heap.front() );
    }

    const Item& getTopItem() const
    {
	return *( this->heap.front() );
    }

    Item& getTopItem()
    {
	return *( this->heap.front() );
    }

    const Item& getItemByIndex( const Index index ) const
    {
	return this->itemVector[ index ];
    }

    Item& getItemByIndex( const Index index )
    {
	return this->itemVector[ index ];
    }

    void popTop();

    void popItem( const Serial serial );

    void popItemByIndex( const Index index );

    const Serial pushItem( const Item& item )
    {
	const Index oldSize( this->size );
    
	++this->size;
    
	if( getSize() > this->heap.size() )
	{
	    this->itemVector.resize( getSize() );
	    this->heap.resize( getSize() );
	    this->positionMap.resize( getSize() );
	
	    this->itemVector.push_back( item );

	    for( Index i( 0 ); i < getSize(); ++i )
	    {
		this->heap[i] = &this->itemVector[i];
	    }

	    *this->heap[ oldSize ] = item;
 
	    std::make_heap( this->heap.begin(), this->heap.end(), this->comp );

	    for( Index i( 0 ); i < getSize(); ++i )
	    {
		this->positionMap[ getItemIndex( this->heap[i] ) ] = i;
	    }
	}
	else
	{
	    *this->heap[ oldSize ] = item;  
	    if( this->comp( &item, this->heap[ oldSize ] ) )
	    {
		moveDownPos( oldSize );
	    }
	    else
	    {
		moveUpPos( oldSize ); 
	    }
	}

	const Serial serial( this->serialCounter );
	++this->serialCounter;

	this->indexMap[ serial ] = oldSize;

	return serial;
    }


    const bool isEmpty() const
    {
	return ( getSize() == 0 );
    }

    const Index getSize() const
    {
	return this->size;
    }


    void clear();

    void moveUp( const Index index )
    {
	const Index position( this->positionMap[index] );
	moveUpPos( position );
    }


    void moveDown( const Index index )
    {
	const Index position( this->positionMap[index] );
	moveDownPos( position );
    }

private:

    inline void moveUpPos( const Index position );
    inline void moveDownPos( const Index position );

    /*
      This method returns the index of the given pointer to Item.

      The pointer must point to a valid item on this->itemVector.
      Returned index is that of the itemVector.
    */
    const Index getItemIndex( const Item * const itemPtr ) const
    {
	return itemPtr - this->itemVector.begin().base();
    }

private:

    ItemVector    itemVector;
    ItemPtrVector heap;

    // maps index to position
    IndexVector   positionMap;

    // maps serial to index.
    IndexMap indexMap;

    Serial   serialCounter;
    Index    size;

    PtrGreater< const Item* const > comp;

};



// begin implementation

template < typename Item >
DynamicPriorityQueue< Item >::DynamicPriorityQueue()
    :
    size( 0 ),
    serialCounter( 0 )
{
    ; // do nothing
}


template < typename Item >
void DynamicPriorityQueue< Item >::clear()
{
    this->itemVector.clear();
    this->heap.clear();
    this->positionMap.clear();
  
    this->size = 0;
  
}


template < typename Item >
void DynamicPriorityQueue< Item >::
move( Index index )
{
    //  assert( position < getSize() );
    const Index position( this->positionMap[index] );

    moveDownPos( position );

    // If above moveDown() didn't move this item,
    // then we need to try moveUp() too.  If moveDown()
    // did work, nothing should be done.
    if( this->positionMap[index] == position )
    {
	moveUpPos( position );
    }
}


template < typename Item >
void DynamicPriorityQueue<Item>::moveUpPos( Index position )
{
    Item* const item( this->heap[position] );
    Index predecessor( ( position - 1 ) / 2 );

    // first pass: do nothing if move up doesn't occur.
    Item* predItem( this->heap[predecessor] );
    if( predecessor == position || this->comp( item, predItem ) )
    {
	return;
    }

    // main loop
    while( 1 )
    {
	this->heap[position] = predItem;
	this->positionMap[ getItemIndex( predItem ) ] = position;
	position = predecessor;
      
	predecessor = ( predecessor - 1 ) / 2;

	predItem = this->heap[predecessor];

	if( predecessor == position || this->comp( item, predItem ) )
	{
	    break;
	}
    }

    this->heap[position] = item;
    this->positionMap[ getItemIndex( item ) ] = position;
}

// this is an optimized version.
template < typename Item >
void DynamicPriorityQueue< Item >::moveDownPos( Index position )
{
    Item* const item( this->heap[position] );
    Index successor( position * 2 + 1);
 

    // first pass: simply return doing nothing if move down doesn't occur.
    if( successor < getSize() - 1 )
    {
	if( this->comp( this->heap[ successor ], 
			this->heap[ successor + 1 ] ) )
	{
	    ++successor;
	}
    }
    else if( successor >= getSize() )
    {
	return;
    }
  
    Item* succItem( this->heap[ successor ] );
    if( this->comp( succItem, item ) )
    {
	return;    // if the going down does not occur, return doing nothing.
    }

    // main loop
    while( 1 )
    {
	// bring up the successor
	this->heap[position] = succItem;
	this->positionMap[ getItemIndex( succItem ) ] = position;
	position = successor;

	// the next successor
	successor = successor * 2 + 1;

	if( successor < getSize() - 1 )
	{
	    if( this->comp( this->heap[ successor ], 
			    this->heap[ successor + 1 ] ) )
	    {
		++successor;
	    }
	}
	else if( successor >= getSize() )
	{
	    break;
	}

	succItem = this->heap[ successor ];

	// if the going down is finished, break.
	if( this->comp( succItem, item ) )
	{
	    break;
	}
    }

    this->heap[position] = item;
    this->positionMap[ getItemIndex( item ) ] = position;
}


/* original version
   template < typename Item >
   void DynamicPriorityQueue< Item >::moveDown( Index index )
   {
   Index successor( index * 2 + 1 );

   if( successor < getSize() - 1 && this->comp( this->heap[successor], this->heap[successor + 1] ) )
   {
   ++successor;
   }

   Item* item( this->heap[index] );
  
   while( successor < getSize() && this->comp( item, this->heap[successor] ) )
   {
   this->heap[index] = this->heap[successor];
   this->positionMap[ this->heap[index] - theFirstItemPtr ] = index;
   index = successor;
   successor = index * 2 + 1;

   if( successor < getSize() - 1 && 
   this->comp( this->heap[successor], this->heap[ successor + 1 ] ) )
   {
   ++successor;
   }
   }

   this->heap[index] = item;
   this->positionMap[ this->heap[index] - theFirstItemPtr ] = index;
   }
*/

template < typename Item >
void DynamicPriorityQueue< Item >::popTop()
{
    --this->size;
    if( this->size == 0 )
    {
	return;
    }

    Item* item( this->heap[0] );
    this->heap[0] = this->heap[getSize()];
    this->heap[getSize()] = item;
    this->positionMap[ getItemIndex( this->heap[0] ) ] = 0;
  
    moveDownPos( 0 );
}

template < typename Item >
void DynamicPriorityQueue< Item >::popItem( const Index index )
{
    --this->size;
    if( this->size == 0 )
    {
	return;
    }

    const Index swappedPosition( this->positionMap[getSize()] );
    const Index deletedPosition( this->positionMap[index] );

    Item* item( this->heap[ deletedPosition ] );
    this->heap[ deletedPosition ] = this->heap[getSize()];
    this->heap[getSize()] = item;
    this->positionMap[ getItemIndex( this->heap[ deletedPosition ] ) ] = 0;
  
    moveDownPos( deletedPosition );
}

#endif // __DYNAMICPRIORITYQUEUE_HPP



/*
  Do not modify
  $Author: shafi $
  $Revision: 2529 $
  $Date: 2005-11-19 01:36:40 -0800 (Sat, 19 Nov 2005) $
  $Locker$
*/





