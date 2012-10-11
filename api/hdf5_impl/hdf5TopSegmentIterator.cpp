/*
 * Copyright (C) 2012 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "hal.h"
#include "hdf5TopSegmentIterator.h"

using namespace std;
using namespace hal;

HDF5TopSegmentIterator::HDF5TopSegmentIterator(TopSegment* topSegment, 
                                               hal_offset_t startOffset, 
                                               hal_offset_t endOffset,
                                               bool reversed) :
  _topSegment(topSegment),
  _startOffset(startOffset),
  _endOffset(endOffset),
  _reversed(reversed)
{

}

HDF5TopSegmentIterator::~HDF5TopSegmentIterator()
{

}
   
//////////////////////////////////////////////////////////////////////////////
// SEGMENT INTERFACE
//////////////////////////////////////////////////////////////////////////////
void HDF5TopSegmentIterator::setArrayIndex(const Genome* genome, 
                                           hal_index_t arrayIndex) const
{
  _topSegment->setArrayIndex(genome, arrayIndex);
}

const Genome* HDF5TopSegmentIterator::getGenome() const
{
  return _topSegment->getGenome();
}

Genome* HDF5TopSegmentIterator::getGenome()
{
  return _topSegment->getGenome();
}

const Sequence* HDF5TopSegmentIterator::getSequence() const
{
  return _topSegment->getSequence();
}

Sequence* HDF5TopSegmentIterator::getSequence()
{
  return _topSegment->getSequence();
}

hal_index_t HDF5TopSegmentIterator::getStartPosition() const
{
  assert (inRange() == true);
  if (_reversed == false)
  {
    return _topSegment->getStartPosition() + _startOffset;
  }
  else
  {
    return _topSegment->getStartPosition() + _topSegment->getLength() - 
       _startOffset - 1;
  }
}

hal_index_t HDF5TopSegmentIterator::getEndPosition() const
{
  assert (inRange() == true);
  if (_reversed == false)
  {
    return getStartPosition() + (hal_index_t)(getLength() - 1);
  }
  else
  {
    return getStartPosition() - (hal_index_t)(getLength() - 1);
  }
}

hal_size_t HDF5TopSegmentIterator::getLength() const
{
  assert (inRange() == true);
  return _topSegment->getLength() - _endOffset - _startOffset;
}

void HDF5TopSegmentIterator::getString(std::string& outString) const
{
  assert (inRange() == true);
  _topSegment->getString(outString);
  if (_reversed == true)
  {
    reverseComplement(outString);
  }
  outString = outString.substr(_startOffset, getLength());
}

void HDF5TopSegmentIterator::setCoordinates(hal_index_t startPos, 
                                               hal_size_t length)
{
  _topSegment->setCoordinates(startPos, length);
}

hal_index_t HDF5TopSegmentIterator::getArrayIndex() const
{
  return _topSegment->getArrayIndex();
}

bool HDF5TopSegmentIterator::leftOf(hal_index_t genomePos) const
{
  assert(genomePos != NULL_INDEX);
  assert(_topSegment->getStartPosition() != NULL_INDEX);
  if (_reversed == false)
  {
    return (hal_index_t)(getStartPosition() + getLength()) <= genomePos;
  }
  else
  {
    return (hal_index_t)getStartPosition() < genomePos;
  }
}

bool HDF5TopSegmentIterator::rightOf(hal_index_t genomePos) const
{
  assert(genomePos != NULL_INDEX);
  assert(_topSegment->getStartPosition() != NULL_INDEX);
  if (_reversed == false)
  {
    return getStartPosition() > genomePos;
  }
  else
  {
    return getStartPosition() - (hal_index_t)getLength() >= genomePos;
  }
}

bool HDF5TopSegmentIterator::overlaps(hal_index_t genomePos) const
{
  return !leftOf(genomePos) && !rightOf(genomePos);
}

bool HDF5TopSegmentIterator::isFirst() const
{
  return !_reversed ? _topSegment->isFirst() : _topSegment->isLast();
}

bool HDF5TopSegmentIterator::isLast() const
{
  return !_reversed ? _topSegment->isLast() : _topSegment->isFirst();
}


//////////////////////////////////////////////////////////////////////////////
// TOP SEGMENT INTERFACE
//////////////////////////////////////////////////////////////////////////////
hal_index_t HDF5TopSegmentIterator::getParentIndex() const
{
  return _topSegment->getParentIndex();
}

bool HDF5TopSegmentIterator::hasParent() const
{
  assert(inRange() == true);
  return _topSegment->getParentIndex() != NULL_INDEX;
}

void HDF5TopSegmentIterator::setParentIndex(hal_index_t parIdx)
{
  _topSegment->setParentIndex(parIdx);
}

bool HDF5TopSegmentIterator::getParentReversed() const
{
  return _topSegment->getParentReversed();
}

void HDF5TopSegmentIterator::setParentReversed(bool isReversed)
{
  _topSegment->setParentReversed(isReversed);
}

hal_index_t HDF5TopSegmentIterator::getBottomParseIndex() const
{
  return _topSegment->getBottomParseIndex();
}

void HDF5TopSegmentIterator::setBottomParseIndex(hal_index_t botParseIdx)
{
  _topSegment->setBottomParseIndex(botParseIdx);
}

hal_offset_t HDF5TopSegmentIterator::getBottomParseOffset() const
{
  return _topSegment->getBottomParseOffset();
}

bool HDF5TopSegmentIterator::hasParseDown() const
{
  assert (inRange() == true);
  assert (_topSegment->getBottomParseIndex() == NULL_INDEX ||
          _topSegment->getGenome()->getNumChildren() > 0);
  return _topSegment->getBottomParseIndex() != NULL_INDEX;
}

hal_index_t HDF5TopSegmentIterator::getNextParalogyIndex() const
{
  return _topSegment->getBottomParseOffset();
}

bool HDF5TopSegmentIterator::hasNextParalogy() const
{
  return _topSegment->hasNextParalogy();
}

void HDF5TopSegmentIterator::setNextParalogyIndex(hal_index_t parIdx)
{
  _topSegment->setNextParalogyIndex(parIdx);
}

hal_index_t HDF5TopSegmentIterator::getLeftParentIndex() const
{
  return _topSegment->getLeftParentIndex();
}

hal_index_t HDF5TopSegmentIterator::getRightParentIndex() const
{
  return _topSegment->getRightParentIndex();
}

//////////////////////////////////////////////////////////////////////////////
// SEGMENT ITERATOR INTERFACE
//////////////////////////////////////////////////////////////////////////////
void HDF5TopSegmentIterator::toLeft(hal_index_t leftCutoff) const
{
  if (_reversed == false)
  {
    if (_startOffset == 0)
    {
      _topSegment->setArrayIndex(getGenome(), _topSegment->getArrayIndex() - 1);
      _endOffset = 0;
    }
    else
    {
      _endOffset = _topSegment->getLength() - _startOffset;
      _startOffset = 0;
    }
    if (leftCutoff != NULL_INDEX && overlaps(leftCutoff))
    {
      assert(_topSegment->getStartPosition() <= leftCutoff);
      _startOffset = leftCutoff - _topSegment->getStartPosition();
    }
  }
  
  else
  {
    if (_startOffset == 0)
    {
      _topSegment->setArrayIndex(getGenome(), _topSegment->getArrayIndex() + 1);
      _endOffset = 0;
    }
    else
    {
      _endOffset = _topSegment->getLength() - _startOffset;
      _startOffset = 0;
    }    
    if ((hal_size_t)getArrayIndex() < getGenome()->getNumTopSegments() &&
        leftCutoff != NULL_INDEX && overlaps(leftCutoff))
    {
      _startOffset = _topSegment->getStartPosition() + 
         _topSegment->getLength() - 1 - leftCutoff;
    }
  }
  assert((hal_size_t)getArrayIndex() >= 
         getGenome()->getNumTopSegments() ||
         getArrayIndex() < 0 || 
         _startOffset + _endOffset <= _topSegment->getLength());
}

void HDF5TopSegmentIterator::toRight(hal_index_t rightCutoff) const  
{
  if (_reversed == false)
  {
    if (_endOffset == 0)
    {
      _topSegment->setArrayIndex(getGenome(), _topSegment->getArrayIndex() + 1);
      _startOffset = 0;
    }
    else
    {
      _startOffset =  _topSegment->getLength() - _endOffset;
      _endOffset = 0;
    }
    
    if ((hal_size_t)getArrayIndex() < getGenome()->getNumTopSegments() &&
        rightCutoff != NULL_INDEX && overlaps(rightCutoff))
    {
      _endOffset = _topSegment->getStartPosition() +
         _topSegment->getLength() - rightCutoff - 1;
    }
  }
  
  else
  {
    if (_endOffset == 0)
    {
      _topSegment->setArrayIndex(getGenome(), _topSegment->getArrayIndex() - 1);
      _startOffset = 0;
    }
    else
    {
      _startOffset =  _topSegment->getLength() - _endOffset;
      _endOffset = 0;
    }

    if (rightCutoff != NULL_INDEX && overlaps(rightCutoff))
    {
      _endOffset = rightCutoff - _topSegment->getStartPosition(); 
    }
  }
  assert ((hal_size_t)getArrayIndex() >= 
          getGenome()->getNumTopSegments() ||
          getArrayIndex() < 0 || 
          _startOffset + _endOffset <= _topSegment->getLength());
}

void HDF5TopSegmentIterator::toReverse() const
{
  assert (inRange() == true);
  _reversed = !_reversed;
  swap(_startOffset, _endOffset);
}

void HDF5TopSegmentIterator::toSite(hal_index_t position, bool slice) const
{
  const Genome* genome = getGenome();
  hal_index_t len = (hal_index_t)genome->getSequenceLength();
  hal_index_t nseg = (hal_index_t)genome->getNumTopSegments();
  
  assert(len != 0);
  double avgLen = (double)len / (double)nseg;
  hal_index_t hint = 
     (hal_index_t)min(nseg - 1., avgLen * ((double)position / (double)len));
  _topSegment->setArrayIndex(genome, hint);
  _startOffset = 0;
  _endOffset = 0;
  
  // out of range
  if (position < 0)
  {
    _topSegment->setArrayIndex(genome, NULL_INDEX);
    return;
  }
  else if (position >= len)
  {
    _topSegment->setArrayIndex(genome, len);
    return;
  }

  hal_index_t left = 0;
  hal_index_t right = nseg - 1;
  assert(_topSegment->getArrayIndex()  >= 0 &&  
         _topSegment->getArrayIndex() < nseg);
  
  while (overlaps(position) == false)
  {
    assert(left != right);
    if (rightOf(position) == true)
    {
      right = _topSegment->getArrayIndex();
      hal_index_t delta = max((_topSegment->getArrayIndex() - left) / 2,
                              (hal_index_t)1);
      _topSegment->setArrayIndex(genome, _topSegment->getArrayIndex() - delta);
      assert(_topSegment->getArrayIndex()  >= 0 &&
             _topSegment->getArrayIndex() < nseg);
    }
    else
    {
      assert(leftOf(position) == true);
      left = _topSegment->getArrayIndex();
      hal_index_t delta = max((right - _topSegment->getArrayIndex()) / 2,
                              (hal_index_t)1);
      _topSegment->setArrayIndex(genome, _topSegment->getArrayIndex() + delta);
      assert(_topSegment->getArrayIndex() >= 0 &&
             _topSegment->getArrayIndex() < nseg);
    }
  }
  
  assert(overlaps(position) == true);
  
  if (slice == true)
  {
     _startOffset = position - _topSegment->getStartPosition();
     _endOffset = _topSegment->getStartPosition() + _topSegment->getLength()
        - position - 1;
     if (_reversed)
     {
       swap(_startOffset, _endOffset);
     }
  }  
}

hal_offset_t HDF5TopSegmentIterator::getStartOffset() const
{
  return _startOffset;
}

hal_offset_t HDF5TopSegmentIterator::getEndOffset() const
{
  return _endOffset;
}

void HDF5TopSegmentIterator::slice(hal_offset_t startOffset, 
                                   hal_offset_t endOffset) const
{
  assert(startOffset < _topSegment->getLength());
  assert(endOffset < _topSegment->getLength());
  _startOffset = startOffset;
  _endOffset = endOffset;
}


bool HDF5TopSegmentIterator::getReversed() const
{
  return _reversed;
}


//////////////////////////////////////////////////////////////////////////////
// TOP SEGMENT ITERATOR INTERFACE
//////////////////////////////////////////////////////////////////////////////
void HDF5TopSegmentIterator::toChild(BottomSegmentIteratorConstPtr bs, 
                                     hal_size_t child) const
{
  assert (inRange() == true);
  _topSegment->setArrayIndex(bs->getGenome()->getChild(child),
                             bs->getChildIndex(child));
  _startOffset = bs->getStartOffset();
  _endOffset = bs->getEndOffset();
  _reversed = bs->getReversed();
  if (bs->getChildReversed(child) == true)
  {
    toReverse();
  }
}

void HDF5TopSegmentIterator::toChildG(BottomSegmentIteratorConstPtr bs, 
                                      const Genome* childGenome) const
{
  assert (inRange() == true);
  toChild(bs, getGenome()->getChildIndex(childGenome));
}


void HDF5TopSegmentIterator::toParseUp(BottomSegmentIteratorConstPtr bs) const
{
  assert (inRange() == true);
  
  const Genome* genome = bs->getGenome();
  hal_index_t index = bs->getTopParseIndex();

  _topSegment->setArrayIndex(genome, index);
  _reversed = bs->getReversed();

  hal_index_t startPos = bs->getStartPosition();

  while (startPos >= _topSegment->getStartPosition() + 
         (hal_index_t)_topSegment->getLength())
  {
    _topSegment->setArrayIndex(genome, ++index);
  }

  if (_reversed == false)
  {
    _startOffset = startPos - _topSegment->getStartPosition();    
    hal_index_t topEnd = _topSegment->getStartPosition() + 
       (hal_index_t)_topSegment->getLength();
    hal_index_t botEnd = bs->getStartPosition() + 
       (hal_index_t)bs->getLength();
    _endOffset = max((hal_index_t)0, topEnd - botEnd);
  }
  else
  {
    _startOffset = _topSegment->getStartPosition() + _topSegment->getLength() -
       1 - startPos;
    hal_index_t topEnd = _topSegment->getStartPosition();
    hal_index_t botEnd = bs->getStartPosition() - 
       (hal_index_t)bs->getLength() + 1;
    _endOffset = max((hal_index_t)0, botEnd - topEnd);  
  }
  assert (_startOffset + _endOffset <= _topSegment->getLength());
}

TopSegmentIteratorPtr HDF5TopSegmentIterator::copy()
{
  TopSegmentIteratorPtr newIt = 
     getGenome()->getTopSegmentIterator(getArrayIndex());
  if (_reversed)
  {
    newIt->toReverse();
  }
  newIt->slice(_startOffset, _endOffset);
  return newIt;
}

TopSegmentIteratorConstPtr HDF5TopSegmentIterator::copy() const
{
  TopSegmentIteratorConstPtr newIt = 
     getGenome()->getTopSegmentIterator(getArrayIndex());
  if (_reversed)
  {
    newIt->toReverse();
  }
  newIt->slice(_startOffset, _endOffset);
  return newIt;
}

void HDF5TopSegmentIterator::copy(TopSegmentIteratorConstPtr ts) const
{
  _topSegment->setArrayIndex(ts->getGenome(), ts->getArrayIndex());
  _startOffset = ts->getStartOffset();
  _endOffset = ts->getEndOffset();
  _reversed = ts->getReversed();
}

TopSegment* HDF5TopSegmentIterator::getTopSegment()
{
  // Deprecated now, but so much current code relies on these functions
  return _topSegment.get();
}

const TopSegment* HDF5TopSegmentIterator::getTopSegment() const
{
  // Deprecated now, but so much current code relies on these functions
  return _topSegment.get();
}

bool HDF5TopSegmentIterator::equals(TopSegmentIteratorConstPtr other) const
{
  assert(getGenome() == other->getGenome());
  return getArrayIndex() == other->getArrayIndex();
}

void HDF5TopSegmentIterator::toNextParalogy() const
{
  assert(_topSegment->getNextParalogyIndex() != NULL_INDEX);
  assert(_topSegment->getNextParalogyIndex() != _topSegment->getArrayIndex());
  bool rev = _topSegment->getParentReversed();
  _topSegment->setArrayIndex(getGenome(), _topSegment->getNextParalogyIndex());
  if(_topSegment->getParentReversed() != rev)
  {
    toReverse();
  }
}

