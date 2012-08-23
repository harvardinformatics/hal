/*
 * Copyright (C) 2012 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <deque>

#include "defaultGappedTopSegmentIterator.h"
#include "hal.h"
using namespace std;
using namespace hal;

DefaultGappedTopSegmentIterator::DefaultGappedTopSegmentIterator(
  TopSegmentIteratorConstPtr left,
  hal_size_t gapThreshold) :
  _gapThreshold(gapThreshold)
{
  const Genome* genome = left->getTopSegment()->getGenome();
  const Genome* parent = genome->getParent();
  _left = left->copy();
  _right = left->copy();
  _temp = left->copy();
  _temp2 = left->copy();
  _leftParent = parent->getBottomSegmentIterator();
  _rightParent = _leftParent->copy();
  setLeft(left);
}

DefaultGappedTopSegmentIterator::~DefaultGappedTopSegmentIterator()
{

}

// Gpped Segment Iterator methods
hal_size_t DefaultGappedTopSegmentIterator::getGapThreshold() const
{
  return _gapThreshold;
}

hal_size_t DefaultGappedTopSegmentIterator::getChildIndex() const
{
  return _childIndex;
}

hal_size_t DefaultGappedTopSegmentIterator::getNumSegments() const
{
  return (hal_size_t)abs(_right->getTopSegment()->getArrayIndex() - 
                         _left->getTopSegment()->getArrayIndex() + 1);
}

hal_size_t DefaultGappedTopSegmentIterator::getNumGaps() const
{
  hal_size_t count = 0;
  _temp->copy(_left);
  for (; _temp->equals(_right) == false; _temp->toRight())
  {
    if (_temp->hasParent() == false)
    {
      ++count;
    }
  }
  return count;
}

hal_size_t DefaultGappedTopSegmentIterator::getNumGapBases() const
{
   hal_size_t count = 0;
  _temp->copy(_left);
  for (; _temp->equals(_right) == false; _temp->toRight())
  {
    if (_temp->hasParent() == false)
    {
      count += _temp->getLength();
    }
  }
  return count;
}

bool DefaultGappedTopSegmentIterator::isLast() const
{
  return getReversed() ? _left->getTopSegment()->isLast() :
     _right->getTopSegment()->isLast();
}

bool DefaultGappedTopSegmentIterator::isFirst() const
{
  return getReversed() ? _right->getTopSegment()->isFirst() :
     _left->getTopSegment()->isFirst();
}

hal_index_t DefaultGappedTopSegmentIterator::getLeftArrayIndex() const
{
  return _left->getTopSegment()->getArrayIndex();
}

hal_index_t DefaultGappedTopSegmentIterator::getRightArrayIndex() const
{
  return _right->getTopSegment()->getArrayIndex();
}

const Sequence* DefaultGappedTopSegmentIterator::getSequence() const
{
  assert(_left->getTopSegment()->getSequence() ==
         _right->getTopSegment()->getSequence());
  return _left->getTopSegment()->getSequence();
}

// Segment Iterator methods
void DefaultGappedTopSegmentIterator::toLeft(hal_index_t leftCutoff) const
{
  assert(_right->getReversed() == _left->getReversed());
  assert(_left->equals(_right) || _left->getReversed() || 
         _left->leftOf(_right->getStartPosition()));
  assert(_left->equals(_right) || !_left->getReversed() || 
         _left->rightOf(_right->getStartPosition()));

  _right->copy(_left);
  _right->toLeft();
  _left->copy(_right);

  hal_index_t ns = _right->getTopSegment()->getGenome()->getNumTopSegments();
  hal_index_t i = _right->getTopSegment()->getArrayIndex();
  if ((_right->getReversed() == true && i < ns) ||
      (_right->getReversed() == false && i > 0))
  {
    extendLeft();
  }
}

void DefaultGappedTopSegmentIterator::toRight(hal_index_t rightCutoff) const
{
  assert(_right->getReversed() == _left->getReversed());
  assert(_left->equals(_right) || _left->getReversed() || 
         _left->leftOf(_right->getStartPosition()));
  assert(_left->equals(_right) || !_left->getReversed() || 
         _left->rightOf(_right->getStartPosition()));

  _left->copy(_right);
  _left->toRight();
  _right->copy(_left);
  
  hal_index_t ns = _left->getTopSegment()->getGenome()->getNumTopSegments();
  hal_index_t i = _left->getTopSegment()->getArrayIndex();
  if ((_left->getReversed() == true && i > 0) ||
      (_left->getReversed() == false && i < ns))
  {
    extendRight();
  }
}

void DefaultGappedTopSegmentIterator::toReverse() const
{
  assert(_right->getReversed() == _left->getReversed());
  assert(_left->equals(_right) || _left->getReversed() || 
         _left->leftOf(_right->getStartPosition()));
  assert(_left->equals(_right) || !_left->getReversed() || 
         _left->rightOf(_right->getStartPosition()));
  _left->toReverse();
  _right->toReverse();
  swap(_left, _right);
}

void DefaultGappedTopSegmentIterator::toSite(hal_index_t position, 
                                             bool slice) const
{
  throw hal_exception("tosite not currently supported in gapped iterators");
}

bool DefaultGappedTopSegmentIterator::hasNextParalogy() const
{
  _temp->copy(_left);
  _temp2->copy(_right);
  
  toRightNextUngapped(_temp);
  toLeftNextUngapped(_temp2);

  assert(_temp->hasNextParalogy() == _temp2->hasNextParalogy());
  return _temp->hasNextParalogy();
}

void DefaultGappedTopSegmentIterator::toNextParalogy() const
{
  assert(hasNextParalogy());

  toRightNextUngapped(_left);
  toLeftNextUngapped(_right);

  _left->toNextParalogy();
  _right->toNextParalogy();
}

hal_offset_t DefaultGappedTopSegmentIterator::getStartOffset() const
{
  return 0;
}

hal_offset_t DefaultGappedTopSegmentIterator::getEndOffset() const
{
  return 0;
}

void DefaultGappedTopSegmentIterator::slice(hal_offset_t startOffset,
                                            hal_offset_t endOffset) const
{
  throw hal_exception("slice not currently supported in gapped iterators");
}

hal_index_t DefaultGappedTopSegmentIterator::getStartPosition() const
{
  return 
     getReversed() ? _right->getStartPosition() : _left->getStartPosition();
}

hal_size_t DefaultGappedTopSegmentIterator::getLength() const
{
  if (getReversed() == false)
  {
    return _right->getStartPosition() + _right->getLength() - 
       _left->getStartPosition();
  }
  else
  {
    return _left->getStartPosition() - _right->getStartPosition() + 
       _right->getLength();
  }
}

hal_bool_t DefaultGappedTopSegmentIterator::getReversed() const
{
  assert(_left->getReversed() == _right->getReversed());
  return _left->getReversed();
}

void DefaultGappedTopSegmentIterator::getString(std::string& outString) const
{
  throw hal_exception("getString not currently supported in gapped iterators");
}

bool DefaultGappedTopSegmentIterator::leftOf(hal_index_t genomePos) const
{
  return _right->leftOf(genomePos);
}

bool DefaultGappedTopSegmentIterator::rightOf(hal_index_t genomePos) const
{
  return _left->rightOf(genomePos);
}

bool DefaultGappedTopSegmentIterator::overlaps(hal_index_t genomePos) const
{
  return !leftOf(genomePos) && !rightOf(genomePos);
}

// GappedTopSegmentIterator Interface Methods
GappedTopSegmentIteratorPtr DefaultGappedTopSegmentIterator::copy()
{
  DefaultGappedTopSegmentIterator* newIt =
     new DefaultGappedTopSegmentIterator(_left, _gapThreshold);
  return GappedTopSegmentIteratorPtr(newIt);
}

GappedTopSegmentIteratorConstPtr DefaultGappedTopSegmentIterator::copy() const
{
  const DefaultGappedTopSegmentIterator* newIt =
     new DefaultGappedTopSegmentIterator(_left, _gapThreshold);
  return GappedTopSegmentIteratorConstPtr(newIt);
}

void DefaultGappedTopSegmentIterator::copy(
  GappedTopSegmentIteratorConstPtr ts) const
{
  _left->copy(ts->getLeft());
  _right->copy(ts->getRight());
  _childIndex = ts->getChildIndex();
  _gapThreshold = ts->getGapThreshold();
}

void DefaultGappedTopSegmentIterator::toChild(
  GappedBottomSegmentIteratorConstPtr bs) const
{
  _leftParent->copy(bs->getLeft());
  _rightParent->copy(bs->getRight());

  toRightNextUngapped(_leftParent);
  toLeftNextUngapped(_rightParent);

  _left->toChild(_leftParent, _childIndex);
  _right->toChild(_rightParent, _childIndex);
}

bool DefaultGappedTopSegmentIterator::equals(
  GappedTopSegmentIteratorConstPtr other) const
{
  _temp->copy(_left);
  toRightNextUngapped(_temp);
  _temp2->copy(other->getLeft());
  toRightNextUngapped(_temp2);
  if (_temp->equals(_temp2) == false)
  {
    return false;
  }
  _temp->copy(_right);
  toLeftNextUngapped(_temp);
  _temp2->copy(other->getRight());
  toLeftNextUngapped(_temp2);
  return _temp->equals(_temp2);
}

bool DefaultGappedTopSegmentIterator::adjacentTo(
  GappedTopSegmentIteratorConstPtr other) const
{
  _temp->copy(_left);
  toLeftNextUngapped(_temp);
  if (_temp->isFirst() == false)
  {
    _temp2->copy(other->getLeft());
    if (_temp2->isFirst() == false)
    {
      _temp2->toLeft();
      toLeftNextUngapped(_temp2);
      if (_temp->equals(_temp2))
      {
        return true;
      }
    }
    _temp2->copy(other->getRight());
    if (_temp2->isLast() == false)
    {
      _temp2->toRight();
      toRightNextUngapped(_temp2);
      if (_temp->equals(_temp2))
      {
        return true;
      }
    }
  }

  _temp->copy(_right);
  toRightNextUngapped(_temp);
  if (_temp->isLast() == false)
  {
    _temp2->copy(other->getLeft());
    if (_temp2->isFirst() == false)
    {
      _temp2->toLeft();
      toLeftNextUngapped(_temp2);
      if (_temp->equals(_temp2))
      {
        return true;
      }
    }
    _temp2->copy(other->getRight());
    if (_temp2->isLast() == false)
    {
      _temp2->toRight();
      toRightNextUngapped(_temp2);
      if (_temp->equals(_temp2))
      {
        return true;
      }
    }
  }

  return false;
}

bool DefaultGappedTopSegmentIterator::hasParent() const
{
  _temp->copy(_left);
  _temp2->copy(_right);
  
  toRightNextUngapped(_temp);
  toLeftNextUngapped(_temp2);

  assert(_temp->hasParent() == _temp2->hasParent());
  return _temp->hasParent();
}

bool DefaultGappedTopSegmentIterator::getParentReversed() const
{
  if (hasParent())
  {
    _temp->copy(_left);
    _temp2->copy(_right);

    toRightNextUngapped(_temp);
    toLeftNextUngapped(_temp2);
    assert(_temp->hasParent() && _temp2->hasParent());
    assert(_temp->getTopSegment()->getParentReversed() == 
           _temp2->getTopSegment()->getParentReversed());
    return _temp->getTopSegment()->getParentReversed();
  }
  return false;
}

TopSegmentIteratorConstPtr DefaultGappedTopSegmentIterator::getLeft() const
{
  return _left;
}

TopSegmentIteratorConstPtr DefaultGappedTopSegmentIterator::getRight() const
{
  return _right;
}

void 
DefaultGappedTopSegmentIterator::setLeft(TopSegmentIteratorConstPtr ti) const
{
  if (ti->getStartOffset() != 0 || ti->getEndOffset() != 0)
  {
    throw hal_exception("offset not currently supported in gapped iterators");
  }
  const Genome* genome = ti->getTopSegment()->getGenome();
  const Genome* parent = genome->getParent();
  if (parent == NULL)
  {
    throw hal_exception("can't init GappedTopIterator with no parent genome");
  }
  for (hal_size_t i = 0; i < parent->getNumChildren(); ++i)
  {
    if (parent->getChild(i) == genome)
    {
      _childIndex = i;
      break;
    }
  }
  _left->copy(ti);
  _right->copy(ti);
  _temp->copy(ti);
  _temp2->copy(ti);
  _leftParent->copy(parent->getBottomSegmentIterator());
  _rightParent->copy(_leftParent);
  extendRight();
}


// Internal methods

bool DefaultGappedTopSegmentIterator::compatible(
  TopSegmentIteratorConstPtr left,
  TopSegmentIteratorConstPtr right) const
{
  assert(left->hasParent() && right->hasParent());
  assert(left->equals(right) == false);
  _leftParent->toParent(left);
  _rightParent->toParent(right);

  if (_leftParent->getReversed() != _rightParent->getReversed())
  {
    return false;
  }

  if (_left->hasNextParalogy() != _right->hasNextParalogy())
  {
    return false;
  }
  if (_left->hasNextParalogy() == true && 
      _left->getTopSegment()->getNextParalogyReversed() !=
      _right->getTopSegment()->getNextParalogyReversed())
  {
    return false;
  }

  if ((!_leftParent->getReversed() && 
       _leftParent->leftOf(_rightParent->getStartPosition()) == false) ||
     (_leftParent->getReversed() && 
      _leftParent->rightOf(_rightParent->getStartPosition()) == false))
  {
    return false;
  }
  
  while (true)
  {
    assert(_leftParent->getReversed() == true ||
           _leftParent->getBottomSegment()->isLast() == false);
    assert(_leftParent->getReversed() == false ||
           _leftParent->getBottomSegment()->isFirst() == false);
    _leftParent->toRight();
    if (_leftParent->hasChild(_childIndex) == true || 
        _leftParent->getLength() >= _gapThreshold)
    {
      if (_leftParent->equals(_rightParent))
      {
        break;
      }
      else
      {
        return false;
      }
    }
  }
  if (_left->hasNextParalogy() == true)
  {
    _temp->copy(_left);
    _temp->toNextParalogy();
    _temp2->copy(_right);
    _temp2->toNextParalogy();
  
    if (_temp->leftOf(_temp2->getStartPosition()) == false)
    {
      return false;
    }
  
    while (true)
    {
      assert(_temp->getReversed() == true ||
             _temp->getTopSegment()->isLast() == false);
      assert(_temp->getReversed() == false ||
             _temp->getTopSegment()->isFirst() == false);
      assert(_temp->getTopSegment()->isLast() == false);
      _temp->toRight();
      if (_temp->hasParent() == true || 
          _temp->getLength() >= _gapThreshold)
      {
        if (_temp->equals(_temp2))
        {
          break;
        }
        else
        {
          return false;
        }
      }
    }
  }

  return true;
}

void DefaultGappedTopSegmentIterator::extendRight() const
{
  _right->copy(_left);
  if ((!_right->getReversed() && _right->getTopSegment()->isLast()) ||
      (_right->getReversed() && _right->getTopSegment()->isFirst()))
  {
    return;
  }
  toRightNextUngapped(_right);
  _temp->copy(_right);

  while ((!_right->getReversed() && !_right->getTopSegment()->isLast()) ||
         (_right->getReversed() && !_right->getTopSegment()->isFirst()))
  {
    _right->toRight();
    toRightNextUngapped(_right);

    if ((_right->hasParent() == false && _right->getLength() > _gapThreshold) ||
        (_temp->hasParent() == false && _temp->getLength() > _gapThreshold) ||
        (_right->hasParent() == true && _temp->hasParent() == true &&
         compatible(_temp, _right) == false))
    {
      _right->toLeft();
      break;
    }
    _temp->toRight();
    toRightNextUngapped(_temp);
  }
}

void DefaultGappedTopSegmentIterator::extendLeft() const
{
  _left->copy(_right);
  if ((!_left->getReversed() && _left->getTopSegment()->isFirst()) ||
      (_left->getReversed() && _left->getTopSegment()->isLast()))
  {
    return;
  };
  toLeftNextUngapped(_left);
  _temp->copy(_left);

  while ((!_left->getReversed() && !_left->getTopSegment()->isFirst()) ||
         (_left->getReversed() && !_left->getTopSegment()->isLast()))
  {
    _left->toLeft();
    toLeftNextUngapped(_left);
    
    if ((_left->hasParent() == false && _left->getLength() > _gapThreshold) ||
        (_temp->hasParent() == false && _temp->getLength() > _gapThreshold) ||
        (_left->hasParent() == true && _temp->hasParent() == true && 
         compatible(_left, _temp) == false))
    {
      _left->toRight();
      break;
    }
    _temp->toLeft();
    toLeftNextUngapped(_temp);

  }
}

void DefaultGappedTopSegmentIterator::toLeftNextUngapped(
  BottomSegmentIteratorConstPtr bs) const
{
  while (bs->hasChild(_childIndex) == false && 
         bs->getLength() <= _gapThreshold)
  {
    if ((!bs->getReversed() && bs->getBottomSegment()->isFirst() ||
         (bs->getReversed() && bs->getBottomSegment()->isLast())))
    {
      break;
    }
    bs->toLeft();
  }
}

void DefaultGappedTopSegmentIterator::toRightNextUngapped(
  BottomSegmentIteratorConstPtr bs) const
{
  while (bs->hasChild(_childIndex) == false &&
         bs->getLength() <= _gapThreshold)
  {
    if ((!bs->getReversed() && bs->getBottomSegment()->isLast() ||
         (bs->getReversed() && bs->getBottomSegment()->isFirst())))
    {
      break;
    }
    bs->toRight();
  }
}
   
void DefaultGappedTopSegmentIterator::toLeftNextUngapped(
  TopSegmentIteratorConstPtr ts) const
{
  while (ts->hasParent() == false && 
         ts->getLength() <= _gapThreshold)
  {
    if ((!ts->getReversed() && ts->getTopSegment()->isFirst() ||
         (ts->getReversed() && ts->getTopSegment()->isLast())))
    {
      break;
    }
    ts->toLeft();
  }
}

void DefaultGappedTopSegmentIterator::toRightNextUngapped(
  TopSegmentIteratorConstPtr ts) const
{
  while (ts->hasParent() == false &&
         ts->getLength() <= _gapThreshold)
  {
    if ((!ts->getReversed() && ts->getTopSegment()->isLast() ||
         (ts->getReversed() && ts->getTopSegment()->isFirst())))
    {
      break;
    }
    ts->toRight();
  }
}
   
