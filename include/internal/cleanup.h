#ifndef IZI_DIFF_CLEANUP_H_
#define IZI_DIFF_CLEANUP_H_

#include <algorithm>

#include "algorithm.h"
#include "types.h"

namespace izi {
namespace diff {
namespace detail {

template<typename Result>
void cleanup(Result& ioResult);

template<typename Result>
void cleanup_first_pass(Result& ioResult)
{
	typedef typename Result::value_type::second_type range_type;

	// Put dummy equality to the end, so we trigger cleanup at least once
	ioResult.push_back(std::make_pair(operation::equal(), range_type()));

	size_t aInsertedCnt(0);
	range_type aInserted;
	size_t aRemovedCnt(0);
	range_type aRemoved;

	typename Result::iterator aResultIt = ioResult.begin();
	while(aResultIt != ioResult.end())
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
			// We encountered both insert and delete operations.
			if((aInsertedCnt > 0) && (aRemovedCnt > 0))
			{
				ioResult.erase(prior(aResultIt, aInsertedCnt + aRemovedCnt), aResultIt);
				range_type aCommonPfx, aCommonSfx;
				partition(aRemoved, aInserted, aCommonPfx, aCommonSfx);
				if(!aCommonPfx.empty())
				{
					if(aResultIt != ioResult.begin())
					{
						std::copy(aCommonPfx.begin(), aCommonPfx.end(), std::back_inserter(prior(aResultIt)->second));
					}
					else
					{
						ioResult.push_front(std::make_pair(operation::equal(), aCommonPfx));
					}
				}
				if(!aCommonSfx.empty())
				{
					std::copy(aResultIt->second.begin(), aResultIt->second.end(), std::back_inserter(aCommonSfx));
					std::swap(aResultIt->second, aCommonSfx);
				}
				if(!aRemoved.empty())
				{
					ioResult.insert(aResultIt, std::make_pair(operation::remove(), aRemoved));
				}
				if(!aInserted.empty())
				{
					ioResult.insert(aResultIt, std::make_pair(operation::insert(), aInserted));
				}
			}
			// Multiple consecutive inserts before equality, merge them together
			else if(aInsertedCnt > 1)
			{
				ioResult.erase(prior(aResultIt, aInsertedCnt), aResultIt);
				ioResult.insert(aResultIt, std::make_pair(operation::insert(), aInserted));
			}
			// Multiple consecutive removes before equality, merge them together
			else if(aRemovedCnt > 1)
			{
				ioResult.erase(prior(aResultIt, aRemovedCnt), aResultIt);
				ioResult.insert(aResultIt, std::make_pair(operation::remove(), aRemoved));
			}
			else if((aResultIt != ioResult.begin()) && prior(aResultIt)->first.isEqual())
			{
				std::copy(aResultIt->second.begin(), aResultIt->second.end(), std::back_inserter(prior(aResultIt)->second));
				aResultIt->second = prior(aResultIt)->second;
				ioResult.erase(prior(aResultIt));
			}
			aInsertedCnt = 0;
			aInserted.clear();
			aRemovedCnt = 0;
			aRemoved.clear();
		}
		++aResultIt;
	}
	if(ioResult.back().second.empty())
	{
		ioResult.pop_back();
	}
}

template<typename Result>
void cleanup_second_pass(Result& ioResult)
{
	typedef typename Result::value_type::second_type range_type;

	/* Second pass: look for single edits surrounded on both sides by equalities
	 * which can be shifted sideways to eliminate an equality.
	 * e.g: A<ins>BA</ins>C -> <ins>AB</ins>AC (ABAC)
	 * e.g: AX<ins>BAX</ins>C -> <ins>AXB</ins>AXC (AXBAXC)
	 * e.g: GH<del>KOP</del>KO -> GHKO<del>PKO</del> (GHKO)
	 */
	const size_t aResultSize = ioResult.size();
	if(aResultSize < 3)
	{
		return;
	}
	typename Result::iterator aPrevIt = ioResult.begin();
	typename Result::iterator aResultIt = next(aPrevIt);
	typename Result::iterator aNextIt = next(aResultIt);
	while(aNextIt != ioResult.end())
	{
		if(aPrevIt->first.isEqual() && aNextIt->first.isEqual())
		{
			if(ends_with(aResultIt->second.begin(), aResultIt->second.end(), aPrevIt->second.begin(), aPrevIt->second.end()))
			{
				const size_t aSize = aResultIt->second.size();
				range_type aTmp(aPrevIt->second);
				aTmp.insert(aTmp.end(), aResultIt->second.begin(), aResultIt->second.end());
				aResultIt->second = aTmp;
				aResultIt->second.resize(aSize);
				aPrevIt->second.insert(aPrevIt->second.end(), aNextIt->second.begin(), aNextIt->second.end());
				aNextIt->second = aPrevIt->second;
				ioResult.erase(aPrevIt);
			}
			else if(starts_with(aResultIt->second.begin(), aResultIt->second.end(), aNextIt->second.begin(), aNextIt->second.end()))
			{
				aPrevIt->second.insert(aPrevIt->second.end(), aNextIt->second.begin(), aNextIt->second.end());
				aResultIt->second.erase(aResultIt->second.begin(), next(aResultIt->second.begin(), aNextIt->second.size()));
				aResultIt->second.insert(aResultIt->second.end(), aNextIt->second.begin(), aNextIt->second.end());
				ioResult.erase(aNextIt);
			}
		}
		aPrevIt = aResultIt;
		++aResultIt;
		aNextIt = (aResultIt != ioResult.end()) ? next(aResultIt) : aResultIt;
	}
	if(aResultSize != ioResult.size())
	{
		cleanup(ioResult);
	}
}

template<typename Result>
void cleanup(Result& ioResult)
{
	cleanup_first_pass(ioResult);
	cleanup_second_pass(ioResult);
}

}  // namespace detail
}  // namespace diff
}  // namespace izi

#endif /* IZI_CLEANUP_H_ */
