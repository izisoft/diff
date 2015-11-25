#ifndef DIFF_LINE_TRANSFORMATION_H_
#define DIFF_LINE_TRANSFORMATION_H_

#include "types.h"

namespace izi {
namespace diff {
namespace detail {

template<typename Traits, typename Iterator, typename Result>
void calculate(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult);

template<typename Traits, typename Iterator>
void line_transform(Iterator iBegin, Iterator iEnd,
		line_vector& oRange,
		typename range_vector<Iterator>::type& oLines,
		typename line_map<Iterator>::type& oLinesMap)
{
	Iterator aEndl = std::find(iBegin, iEnd, Traits::endl());

	while(iBegin != iEnd)
	{
		if(aEndl != iEnd)
		{
			++aEndl;
		}
		range<Iterator> aLine(iBegin, aEndl);

		typename line_map<Iterator>::type::const_iterator aLineIt = oLinesMap.find(aLine);
		if(aLineIt != oLinesMap.end())
		{
			oRange.push_back(aLineIt->second);
		}
		else
		{
			oLines.push_back(aLine);
			oLinesMap.insert(std::make_pair(aLine, oLines.size() - 1));
			oRange.push_back(oLines.size() - 1);
		}

		iBegin = aEndl;
		aEndl = std::find(iBegin, iEnd, Traits::endl());
	}
}

template<typename Traits, typename Iterator>
void line_transform(Iterator iBegin1, Iterator iEnd1,
		Iterator iBegin2, Iterator iEnd2,
		line_vector& oRange1, line_vector& oRange2,
		typename range_vector<Iterator>::type& oLines)
{
	typename line_map<Iterator>::type aLinesMap;
	line_transform<Traits>(iBegin1, iEnd1, oRange1, oLines, aLinesMap);
	line_transform<Traits>(iBegin2, iEnd2, oRange2, oLines, aLinesMap);
}

template<typename TrResult, typename Result, typename LineCont>
void reverse_transform(const TrResult iTrResult, Result& oResult, const LineCont& iLines)
{
	typedef typename Result::value_type::second_type range_type;

	// Perform reverse transformation of the line diff result
	typename TrResult::const_iterator aTrEnd = iTrResult.end();
	for(typename TrResult::const_iterator aTrIt = iTrResult.begin(); aTrIt != aTrEnd; ++aTrIt)
	{
		oResult.push_back(std::make_pair(aTrIt->first, range_type()));
		range_type& aRange = oResult.back().second;

		line_vector::const_iterator aLineEnd = aTrIt->second.end();
		for(line_vector::const_iterator aLineIt = aTrIt->second.begin(); aLineIt != aLineEnd; ++aLineIt)
		{
			const typename LineCont::value_type& aLine = iLines[*aLineIt];
			std::copy(aLine._begin, aLine._end, std::back_inserter(aRange));
		}
	}
}

template<typename Result>
void cleanup_transformation(Result& oResult)
{
	typedef typename Result::value_type::second_type range_type;

	oResult.push_back(std::make_pair(operation::equal(), range_type()));
	range_type aInserted;
	range_type aRemoved;
	unsigned long aInsertedCnt(0);
	unsigned long aRemovedCnt(0);
	typename Result::iterator aResultIt = oResult.begin();
	while(aResultIt != oResult.end())
	{
		if(aResultIt->first.isInsert())
		{
			++aInsertedCnt;
			std::copy(aResultIt->second.begin(), aResultIt->second.end(), std::back_inserter(aInserted));
		}
		else if(aResultIt->first.isRemove())
		{
			++aRemovedCnt;
			std::copy(aResultIt->second.begin(), aResultIt->second.end(), std::back_inserter(aRemoved));
		}
		else if(aResultIt->first.isEqual())
		{
			if((aInsertedCnt > 0) && (aRemovedCnt > 0))
			{
				oResult.erase(next(aResultIt, - (aInsertedCnt + aRemovedCnt)), aResultIt);
				Result aResult;
				calculate<void_traits>(aRemoved.begin(), aRemoved.end(), aInserted.begin(), aInserted.end(), aResult);
				oResult.splice(aResultIt, aResult);
			}
			aInsertedCnt = 0;
			aRemovedCnt = 0;
			aInserted.clear();
			aRemoved.clear();
		}
		++aResultIt;
	}
	if(oResult.back().second.empty())
	{
		oResult.pop_back();
	}
}

template<typename Traits, typename Iterator, typename Result>
void line_diff(Iterator iBegin1, Iterator iEnd1,
		Iterator iBegin2, Iterator iEnd2, Result& oResult)
{
	line_vector aTransform1;
	line_vector aTransform2;
	typename range_vector<Iterator>::type aLines;

	// Transform to lines
	line_transform<Traits>(iBegin1, iEnd1, iBegin2, iEnd2, aTransform1, aTransform2, aLines);

	// Calculate diff on lines
	std::list<std::pair<operation, line_vector> > aTrResult;
	calculate<void_traits>(aTransform1.begin(), aTransform1.end(), aTransform2.begin(), aTransform2.end(), aTrResult);

	// Perform reverse transformation of the line diff result
	reverse_transform(aTrResult, oResult, aLines);

	cleanup_transformation(oResult);
}

}  // namespace detail
}  // namespace diff
}  // namespace izi

#endif /* DIFF_LINE_TRANSFORMATION_H_ */
