/*
 * Copyright (C) 2012 by Glenn Hickey (hickey@soe.ucsc.edu)
 * Copyright (C) 2012-2019 by UCSC Computational Genomics Lab
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef _HALDEFS_H
#define _HALDEFS_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>

/**
 *  Keep simple compiler-related definitions in this file.
 */

/** Version of the library.  Increase before the decimal means
 * backward compatibility with files generated by previous versions
 * is lost (see checking function in halCommon.h) */
static const std::string HAL_VERSION = "2.1";

/**
 * An index in a genome (must be large enough to represent the
 * number of bases in the largest genome in the file)
 */
typedef int64_t hal_index_t;

/**
 * An index in a segment (top or bottom) array
 */
typedef uint64_t hal_offset_t;

/**
 * Size type
 */
typedef uint64_t hal_size_t;

/* gigabyte constant */
static const uint64_t GIGABYTE = 1024ULL * 1024ULL * 1024ULL;

/*
 * General usage exception class, used for all critical errors.
 */
class hal_exception : public std::runtime_error {
  public:
    hal_exception(const std::string &msg) : std::runtime_error(msg) {
    }
};

/**
 * Exception raised when on a Unix errno error.
 */
class hal_errno_exception : public hal_exception {
  public:
    hal_errno_exception(const std::string &msg, int errnum) : hal_exception(msg + ": " + std::strerror(errnum)) {
    }
    hal_errno_exception(const std::string &fileName, const std::string &msg, int errnum)
        : hal_exception(fileName + ": " + msg + ": " + std::strerror(errnum)) {
    }
};

namespace hal {

    extern const hal_index_t NULL_INDEX; /// FIXME: make inline.

// FORWARD DECLARATIONS
#define HAL_FORWARD_DEC_CLASS(T)                                                                                               \
    class T;                                                                                                                   \
    typedef std::shared_ptr<T> T##Ptr;                                                                                         \
    typedef std::shared_ptr<const T> T##ConstPtr;

#define HAL_FORWARD_DEC_MUTABLE_CLASS(T)                                                                                       \
    class T;                                                                                                                   \
    typedef std::shared_ptr<T> T##Ptr;

    HAL_FORWARD_DEC_CLASS(Alignment)
    HAL_FORWARD_DEC_CLASS(Genome)
    HAL_FORWARD_DEC_CLASS(MetaData)
    HAL_FORWARD_DEC_MUTABLE_CLASS(TopSegment)
    HAL_FORWARD_DEC_MUTABLE_CLASS(BottomSegment)
    HAL_FORWARD_DEC_CLASS(Segment)
    HAL_FORWARD_DEC_CLASS(Sequence)
    HAL_FORWARD_DEC_CLASS(SlicedSegment)
    HAL_FORWARD_DEC_CLASS(Rearrangement)
    HAL_FORWARD_DEC_MUTABLE_CLASS(DnaAccess)
    HAL_FORWARD_DEC_MUTABLE_CLASS(MappedSegment)
    HAL_FORWARD_DEC_MUTABLE_CLASS(SegmentIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(GappedSegmentIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(TopSegmentIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(GappedTopSegmentIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(BottomSegmentIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(GappedBottomSegmentIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(DnaIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(SequenceIterator)
    HAL_FORWARD_DEC_MUTABLE_CLASS(ColumnIterator)
}
#endif
// Local Variables:
// mode: c++
// End:
