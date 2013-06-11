// ==========================================================================
//                 SeqAn - The Library for Sequence Analysis
// ==========================================================================
// Copyright (c) 2006-2013, Knut Reinert, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Knut Reinert or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL KNUT REINERT OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: Manuel Holtgrewe <manuel.holtgrewe@fu-berlin.de>
// Author: David Weese <david.weese@fu-berlin.de>
// ==========================================================================
// Utility macros for parallelism.
// ==========================================================================

#ifndef SEQAN_PARALLEL_PARALLEL_SPLITTING_H_
#define SEQAN_PARALLEL_PARALLEL_SPLITTING_H_

namespace seqan {

struct Equidistant_;
typedef Tag<Equidistant_> Equidistant;

/**
.Class.Splitter
..cat:Parallelism
..summary:Splits an interval into subintervals.
..signature:Splitter<TPosition[, TSpec]>
..concept:Concept.ContainerConcept
..param.TPosition:Type of the interval boundaries.
..param.TSpec:Tag to select the way the values are sampled.
...default:Equidistant
..remarks:The Splitter splits an interval into subintervals whose boundaries are enumerated by the Splitter.
It is a container whose length equals the number of subintervals + 1.
..example:
...text:Simple case for equidistant (default) splitting.
...code:Splitter<unsigned> splitter(0, 10, 3);
for (unsigned i = 0; i < length(splitter); ++i)
    std::cout << splitter[i] << std::endl; 
// outputs 0 4 7 10
..include:seqan/parallel.h

.Memfunc.Splitter#Splitter
..summary:Constructor
..signature:Splitter(beginPos, endPos, count)
..param.beginPos:Left interval boundary.
..param.endPos:Right interval boundary.
..param.count:Number of subintervals.
..class:Class.Splitter
 */

template <typename TPosition, typename TSpec = Equidistant>
struct Splitter
{
    TPosition beginPos;
    TPosition length;

    TPosition blockLength;
    TPosition rest;

    Splitter(TPosition beginPos_, TPosition endPos, TPosition count):
        beginPos(beginPos_),
        length(count + 1),
        blockLength((endPos - beginPos) / count),
        rest((endPos - beginPos) % count)
    {
    }

    TPosition operator[] (TPosition i)
    {
        SEQAN_ASSERT_LEQ(i, length);
        return beginPos + blockLength * i + std::min(i, rest);
    }
};

template <typename TPosition, typename TSpec>
struct Size<Splitter<TPosition, TSpec> >
{
    typedef TPosition Type;
};

template <typename TPosition, typename TSpec>
struct Value<Splitter<TPosition, TSpec> >
{
    typedef TPosition Type;
};

template <typename TPosition, typename TSpec>
inline TPosition
length(Splitter<TPosition, TSpec> const &splitter)
{
    return splitter.length;
}

/**
.Function.computeSplitters
..cat:Parallelism
..summary:Compute splitters for a sequence of objects.
..signature:computeSplitters(splitters, size, count)
..param.splitters:Resulting splitters, will be resized to contain $count + 1$ elements.
...type:Spec.Alloc String
..param.size:The number of objects to split.
..param.count:The number of chunks.
..remarks:The first $count - 1$ chunks will have the size $ceil(size / count)$, the last chunk will contain the rest.
..example.text:Most simple case for splitting.
..example.code:String<unsigned> splitters;
computeSplitters(splitters, 10, 5);
// splitters == {0, 5, 10}
..example.text:In this case, the last chunks will stay empty.
..example.code:computeSplitters(splitters, 3, 5);
// splitters == {0, 1, 2, 3, 3, 3}
..include:seqan/parallel.h
 */

template <typename TPosString, typename TSize, typename TCount>
void computeSplitters(TPosString & splitters, TSize size, TCount count)
{
    typedef typename Value<TPosString>::Type TPos;

    SEQAN_ASSERT_GEQ(count, (TCount)0);

    resize(splitters, count + 1);

    TSize blockLength = (size / count) + 1;
    TCount rest = size % count;
    TPos pos = 0;

    TCount i = 0;

    // the first (size % count) many blocks have length (size / count) + 1
    for (; i < rest; ++i, pos += blockLength)
        splitters[i] = pos;

    // the remaining blocks have length (size / count)
    --blockLength;
    for (; i <= count; ++i, pos += blockLength)
        splitters[i] = pos;

    SEQAN_ASSERT_EQ(back(splitters), static_cast<TPos>(size));
}

}  // namespace seqan

#endif  // SEQAN_PARALLEL_PARALLEL_SPLITTING_H_
