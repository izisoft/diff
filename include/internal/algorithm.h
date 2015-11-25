#ifndef DIFF_ALGORITHM_H_
#define DIFF_ALGORITHM_H_

#include <algorithm>
#include <iterator>

namespace izi {
namespace diff {
namespace detail {

template<typename Iterator>
inline Iterator next(Iterator iIterator)
{
    return ++iIterator;
}

template<typename Iterator, class Distance>
inline Iterator next(Iterator iIterator, Distance iDistance)
{
    std::advance(iIterator, iDistance);
    return iIterator;
}

template<typename Iterator>
inline Iterator prior(Iterator iIterator)
{
    return --iIterator;
}

template<typename Iterator, class Distance>
inline Iterator prior(Iterator iIterator, Distance iDistance)
{
    std::advance(iIterator, -iDistance);
    return iIterator;
}

template<typename Iterator>
inline Iterator common_prefix(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2)
{
	if(std::distance(iBegin1, iEnd1) > std::distance(iBegin2, iEnd2))
	{
		return std::mismatch(iBegin2, iEnd2, iBegin1).second;
	}
	else
	{
		return std::mismatch(iBegin1, iEnd1, iBegin2).first;
	}
}

template<typename Iterator>
inline Iterator common_suffix(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2)
{
	typedef typename std::reverse_iterator<Iterator> Reverse_t;
	if(std::distance(iBegin1, iEnd1) > std::distance(iBegin2, iEnd2))
	{
		return std::mismatch(Reverse_t(iEnd2), Reverse_t(iBegin2), Reverse_t(iEnd1)).second.base();
	}
	else
	{
		return std::mismatch(Reverse_t(iEnd1), Reverse_t(iBegin1), Reverse_t(iEnd2)).first.base();
	}
}

template<typename Iterator>
inline bool equal(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2)
{
	return (std::distance(iBegin1, iEnd1) == std::distance(iBegin2, iEnd2)) &&
			std::equal(iBegin1, iEnd1, iBegin2);
}

template<typename Range>
void partition(Range& ioRng1, Range& ioRng2, Range& oCommonPfx, Range& oCommonSfx)
{
	typename Range::iterator aPfxIt = common_prefix(ioRng1.begin(), ioRng1.end(), ioRng2.begin(), ioRng2.end());
	if(aPfxIt != ioRng1.begin())
	{
		oCommonPfx = Range(ioRng1.begin(), aPfxIt);
		ioRng1.erase(ioRng1.begin(), aPfxIt);
		ioRng2.erase(ioRng2.begin(), next(ioRng2.begin(), oCommonPfx.size()));
	}
	typename Range::iterator aSfxIt = common_suffix(ioRng1.begin(), ioRng1.end(), ioRng2.begin(), ioRng2.end());
	if(aSfxIt != ioRng1.end())
	{
		oCommonSfx = Range(aSfxIt, ioRng1.end());
		ioRng1.erase(aSfxIt, ioRng1.end());
		ioRng2.erase(prior(ioRng2.end(), oCommonSfx.size()), ioRng2.end());
	}
}

template<typename Iterator>
inline bool starts_with(Iterator iInputBegin, Iterator iInputEnd, Iterator iTestBegin, Iterator iTestEnd)
{
	Iterator anInputIt = iInputBegin;
	Iterator aTestIt = iTestBegin;

    for(; (anInputIt != iInputEnd) && (aTestIt != iTestEnd); ++anInputIt,++aTestIt)
    {
        if(*anInputIt != *aTestIt)
        {
            return false;
        }
    }

    return aTestIt == iTestEnd;
}

template<typename Iterator>
inline bool ends_with(Iterator iInputBegin, Iterator iInputEnd, Iterator iTestBegin, Iterator iTestEnd)
{
	Iterator anInputIt = iInputEnd;
	Iterator aTestIt = iTestEnd;

    for(;(anInputIt != iInputBegin) && (aTestIt != iTestBegin);)
    {
        if(*(--anInputIt) != *(--aTestIt))
        {
            return false;
        }
    }

    return aTestIt == iTestBegin;
}

}  // namespace detail
}  // namespace diff
}  // namespace izi

#endif /* DIFF_ALGORITHM_H_ */
