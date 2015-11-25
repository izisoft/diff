#ifndef DIFF_CALCULATION_H_
#define DIFF_CALCULATION_H_

#include <iostream>

#include "bisect.h"
#include "cleanup.h"
#include "line_transformation.h"
#include "operation.h"

namespace izi {
namespace diff {
namespace detail {

template<typename Traits, typename Iterator, typename Result>
void calculate(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult, const non_line_range&)
{
	bisect(iBegin1, iEnd1, iBegin2, iEnd2, oResult);
}

template<typename Traits, typename Iterator, typename Result>
void calculate(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult, const line_range&)
{
	if((std::distance(iBegin1, iEnd1) > Traits::min_size()) && (std::distance(iBegin2, iEnd2) > Traits::min_size()))
	{
		line_diff<Traits>(iBegin1, iEnd1, iBegin2, iEnd2, oResult);
	}
	else
	{
		bisect(iBegin1, iEnd1, iBegin2, iEnd2, oResult);
	}
}

template<typename Iterator, typename Result>
bool check_empty(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult)
{
	typedef typename Result::value_type::second_type range_type;
	if(iBegin1 == iEnd1)
	{
		oResult.push_back(std::make_pair(operation::insert(), range_type(iBegin2, iEnd2)));
		return true;
	}
	if(iBegin2 == iEnd2)
	{
		oResult.push_back(std::make_pair(operation::remove(), range_type(iBegin1, iEnd1)));
		return true;
	}
	return false;
}

/*! Checks if shorter range is part of the longer one.
 *
 * @param iBegin1
 * @param iEnd1
 * @param iBegin2
 * @param iEnd2
 * @param oResult
 * @return
 */
template<typename Iterator, typename Result>
bool check_subrange(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult)
{
	typedef typename Result::value_type::second_type range_type;
	const bool aFirstLonger(std::distance(iBegin1, iEnd1) > std::distance(iBegin2, iEnd2));

	Iterator aShortBegin(aFirstLonger ? iBegin2 : iBegin1);
	Iterator aShortEnd(aFirstLonger ? iEnd2 : iEnd1);
	Iterator aLongBegin(aFirstLonger ? iBegin1 : iBegin2);
	Iterator aLongEnd(aFirstLonger ? iEnd1 : iEnd2);
	const size_t aShortSize(std::distance(aShortBegin, aShortEnd));

	Iterator anIt = std::search(aLongBegin, aLongEnd, aShortBegin, aShortEnd);
	if(anIt != aLongEnd)
	{
		const operation anOperation(aFirstLonger ? operation::remove() : operation::insert());
		if(anIt != aLongBegin)
		{
			oResult.push_back(std::make_pair(anOperation, range_type(aLongBegin, anIt)));
		}
		oResult.push_back(std::make_pair(operation::equal(), range_type(aShortBegin, aShortEnd)));
		if(std::distance(anIt, aLongEnd) > aShortSize)
		{
			std::advance(anIt, aShortSize);
			oResult.push_back(std::make_pair(anOperation, range_type(anIt, aLongEnd)));
		}
		return true;
	}
	if(aShortSize == 1)
	{
		// Single character string.
		// After the previous speedup, the character can't be an equality.
		oResult.push_back(std::make_pair(operation::remove(), range_type(iBegin1, iEnd1)));
		oResult.push_back(std::make_pair(operation::insert(), range_type(iBegin2, iEnd2)));
		return true;
	}
	return false;
}

template<typename Iterator, typename Result>
Iterator check_pfx_sfx(Iterator& ioBegin1, Iterator& ioEnd1, Iterator& ioBegin2, Iterator& ioEnd2, Result& oResult)
{
	typedef typename Result::value_type::second_type range_type;

	Iterator aPfxIt = common_prefix(ioBegin1, ioEnd1, ioBegin2, ioEnd2);
	if(aPfxIt != ioBegin1)
	{
		oResult.push_back(std::make_pair(operation::equal(), range_type(ioBegin1, aPfxIt)));
		std::advance(ioBegin2, std::distance(ioBegin1, aPfxIt));
		ioBegin1 = aPfxIt;
	}

	Iterator aSfxIt = common_suffix(ioBegin1, ioEnd1, ioBegin2, ioEnd2);
	if(aSfxIt != ioEnd1)
	{
		std::advance(ioEnd2, -std::distance(aSfxIt, ioEnd1));
		ioEnd1 = aSfxIt;
	}
	return aSfxIt;
}

template<typename Traits, typename Iterator, typename Result>
void calculate(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult)
{
	typedef typename Result::value_type::second_type range_type;

	if((iBegin1 != iEnd1) && (iBegin2 != iEnd2))
	{
		if(equal(iBegin1, iEnd1, iBegin2, iEnd2))
		{
			oResult.push_back(std::make_pair(operation::equal(), range_type(iBegin1, iEnd1)));
			return;
		}

		Iterator aBegin1 = iBegin1;
		Iterator aEnd1 = iEnd1;
		Iterator aBegin2 = iBegin2;
		Iterator aEnd2 = iEnd2;

		// Check if ranges have common prefix and/or suffix
		Iterator aSfxIt = check_pfx_sfx(aBegin1, aEnd1, aBegin2, aEnd2, oResult);

		if(!check_empty(aBegin1, aEnd1, aBegin2, aEnd2, oResult) &&
				!check_subrange(aBegin1, aEnd1, aBegin2, aEnd2, oResult))
		{
			// Perform a real diff.
			calculate<Traits>(aBegin1, aEnd1, aBegin2, aEnd2, oResult, typename Traits::range_type());
		}

		// Push the common suffix to the result
		if(aSfxIt != iEnd1)
		{
			oResult.push_back(std::make_pair(operation::equal(), range_type(aSfxIt, iEnd1)));
		}

		cleanup(oResult);
	}
	else
	{
		check_empty(iBegin1, iEnd1, iBegin2, iEnd2, oResult);
	}
}

}  // namespace detail
}  // namespace diff
}  // namespace izi

#endif /* CALCULATION_H_ */
