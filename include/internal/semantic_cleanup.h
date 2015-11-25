#ifndef IZI_SEMANTIC_CLEANUP_H_
#define IZI_SEMANTIC_CLEANUP_H_

#include <cctype>
#include <stack>

#include "algorithm.h"
#include "cleanup.h"

namespace izi {
namespace diff {
namespace detail {

template<typename Traits, typename Range>
int semantic_score(const Range& iRange1, const Range& iRange2)
{
	if (iRange1.empty() || iRange2.empty())
	{
		// Edges are the best.
		return 6;
	}

	typedef typename Range::value_type value_type;

	static const Range kDoubleEndl(Traits::endl(), 2);

	// Each port of this function behaves slightly differently due to
	// subtle differences in each language's definition of things like
	// 'whitespace'.  Since this function's purpose is largely cosmetic,
	// the choice has been made to use each language's native features
	// rather than force total conformity.
	const value_type aValue1 = iRange1.back();
	const value_type aValue2 = iRange2.front();
	const bool aNonAlphaNumeric1(std::isalnum(aValue1) == 0);
	const bool aNonAlphaNumeric2(std::isalnum(aValue2) == 0);;
	const bool aWhitespace1 = aNonAlphaNumeric1 && (std::isspace(aValue1) != 0);
	const bool aWhitespace2 = aNonAlphaNumeric2 && (std::isspace(aValue2) != 0);
	const bool aLineBreak1 = aWhitespace1 && (std::iscntrl(aValue1) != 0);
	const bool aLineBreak2 = aWhitespace2 && (std::iscntrl(aValue2) != 0);
	const bool aBlankLine1 = aLineBreak1 && ends_with(iRange1.begin(), iRange1.end(), kDoubleEndl.begin(), kDoubleEndl.end());
	const bool aBlankLine2 = aLineBreak2 && starts_with(iRange2.begin(), iRange2.end(), kDoubleEndl.begin(), kDoubleEndl.end());

	if (aBlankLine1 || aBlankLine2)
	{
		// Five points for blank lines.
		return 5;
	}
	else if (aLineBreak1 || aLineBreak2)
	{
		// Four points for line breaks.
		return 4;
	}
	else if (aNonAlphaNumeric1 && !aWhitespace1 && aWhitespace2)
	{
		// Three points for end of sentences.
		return 3;
	}
	else if (aWhitespace1 || aWhitespace2)
	{
		// Two points for whitespace.
		return 2;
	}
	else if (aNonAlphaNumeric1 || aNonAlphaNumeric2)
	{
		// One point for non-alphanumeric.
		return 1;
	}
	return 0;
}

template<typename Traits, typename Result>
void cleanup_small_equalities(Result& ioResult)
{
	const size_t aResultSize = ioResult.size();
	std::stack<typename Result::iterator> aEqualities;

	size_t aPreInsertSize(0);
	size_t aPreDeleteSize(0);
	size_t aPostInsertSize(0);
	size_t aPostDeleteSize(0);

	typename Result::iterator aResultIt = ioResult.begin();
	while(aResultIt != ioResult.end())
	{
		if(aResultIt->first.isEqual())
		{
			aEqualities.push(aResultIt);
			aPreInsertSize = aPostInsertSize;
			aPreDeleteSize = aPostInsertSize;
			aPostInsertSize = 0;
			aPostDeleteSize = 0;
		}
		else
		{
			if(aResultIt->first.isInsert())
			{
				aPostInsertSize += aResultIt->second.size();
			}
			else if(aResultIt->first.isRemove())
			{
				aPostDeleteSize += aResultIt->second.size();
			}

			// Eliminate an equality that is smaller or equal to the edits on both
			// sides of it.
			if(!aEqualities.empty() &&
					(aEqualities.top()->second.size() <= std::max(aPreInsertSize, aPreDeleteSize)) &&
					(aEqualities.top()->second.size() <= std::max(aPostInsertSize, aPostDeleteSize)))
			{
				aResultIt = aEqualities.top();
				aResultIt->first.isRemove();
				ioResult.insert(aResultIt, std::make_pair(operation::insert(), aResultIt->second));

				aEqualities.pop();
				if(!aEqualities.empty())
				{
					aEqualities.pop();
				}
				if(aEqualities.empty())
				{
					aResultIt = ioResult.begin();
				}
				else
				{
					aResultIt = aEqualities.top();
				}
				aPreInsertSize = 0;
				aPreDeleteSize = 0;
				aPostInsertSize = 0;
				aPostDeleteSize = 0;
			}
		}
		++aResultIt;
	}

	// Normalize the diff.
	if(aResultSize != ioResult.size())
	{
		cleanup(ioResult);
	}
}

template<typename Result>
void cleanup_isolated_changes(Result& ioResult)
{
	typedef typename Result::value_type::second_type range_type;

	if(ioResult.size() < 3)
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
			// ABCDEF<ins>WTFDEF</ins>GHIJK
			// commonString -> DEF
			// eq1 = ABC
			// change = WTF
			// eq2 = WTFGHIJK
			range_type anEquality1 = aPrevIt->second;
			range_type aChange = aResultIt->second;
			range_type anEquality2 = aNextIt->second;
			typename range_type::iterator aSfxIt = common_suffix(anEquality1.begin(), anEquality1.end(), aChange.begin(), aChange.end());
			if(aSfxIt != anEquality1.end())
			{
				const range_type aCommonStr(aSfxIt, anEquality1.end());
				anEquality1.erase(aSfxIt, anEquality1.end());
				aChange.resize(aChange.size() - aCommonStr.size());
				aChange = aCommonStr+aChange;
				anEquality2 = aCommonStr + anEquality2;
			}

			// Second, step character by character right, looking for the best fit.
			range_type aBestEquality1 = anEquality1;
			range_type aBestChange = aChange;
			range_type aBestEquality2 = anEquality2;
			int aBestScore = semantic_score(anEquality1, aChange) + semantic_score(aChange, anEquality2);
			while (!aChange.empty() && !anEquality2.empty() && aChange[0] == anEquality2[0])
			{
				anEquality1 += aChange[0];
				aChange = aChange.substr(1) + anEquality2[0];
				anEquality2 = anEquality2.substr(1);
				int aScore = semantic_score(anEquality1, aChange) + semantic_score(aChange, anEquality2);
				// The >= encourages trailing rather than leading whitespace on edits.
				if (aScore >= aBestScore)
				{
					aBestScore = aScore;
					aBestEquality1 = anEquality1;
					aBestChange = aChange;
					aBestEquality2 = anEquality2;
				}
			}

			if (aPrevIt->second != aBestEquality1)
			{
				// We have an improvement, save it back to the diff.
				if (!aBestEquality1.empty())
				{
					aPrevIt->second = aBestEquality1;
				}
				else
				{
					ioResult.erase(aPrevIt);
				}
				aResultIt->second = aBestChange;

				if (!aBestEquality2.empty())
				{
					aNextIt->second = aBestEquality2;
				}
				else
				{
					ioResult.erase(aNextIt);
					aResultIt = aPrevIt;
				}
			}
		}
		aPrevIt = aResultIt;
		++aResultIt;
		aNextIt = (aResultIt != ioResult.end()) ? next(aResultIt) : aResultIt;
	}
}

template<typename Result>
void cleanup_change_overlaps(Result& ioResult)
{
	typedef typename Result::value_type::second_type range_type;

	// Find any overlaps between deletions and insertions.
	// e.g: <del>abcxxx</del><ins>xxxdef</ins>
	//   -> <del>abc</del>xxx<ins>def</ins>
	// e.g: <del>xxxabc</del><ins>defxxx</ins>
	//   -> <ins>def</ins>xxx<del>abc</del>
	// Only extract an overlap if it is as big as the edit ahead or behind it.
	if(ioResult.size() < 2)
	{
		return;
	}
	typename Result::iterator aPrevIt = ioResult.begin();
	typename Result::iterator aResultIt = next(aPrevIt);
	while(aResultIt != ioResult.end())
	{
		if(aPrevIt->first.isRemove() && aResultIt->first.isInsert())
		{
			range_type& aDeletion = aPrevIt->second;
			range_type& anInsertion = aResultIt->second;
			typename range_type::iterator aSfxIt1 = common_suffix(aDeletion.begin(), aDeletion.end(), anInsertion.rbegin(), anInsertion.rend());
			typename range_type::iterator aSfxIt2 = common_suffix(anInsertion.begin(), anInsertion.end(), aDeletion.rbegin(), aDeletion.rend());
			const size_t aSize1(std::distance(aSfxIt1, aDeletion.end()));
			const size_t aSize2(std::distance(aSfxIt2, anInsertion.end()));
			if(aSize1 >= aSize2)
			{
				if((aSize1 >= (aDeletion.size() / 2.0)) || (aSize1 >= (anInsertion.size() / 2.0)))
				{
					ioResult.insert(aResultIt, std::make_pair(operation::equal(), range_type(aSfxIt1, aDeletion.end())));
					aDeletion.resize(aSize1);
					anInsertion = anInsertion.substr(aSize1);
				}
			}
			else
			{
				if((aSize2 >= (aDeletion.size() / 2.0)) || (aSize2 >= (anInsertion.size() / 2.0)))
				{
					// Reverse overlap found.
					// Insert an equality and swap and trim the surrounding edits.
					ioResult.insert(aResultIt, std::make_pair(operation::equal(), range_type(aSfxIt2, anInsertion.end())));
					anInsertion.resize(aSize2);
					aDeletion = aDeletion.substr(aSize2);
					anInsertion.swap(aDeletion);
					aPrevIt->first = operation::insert();
					aResultIt->first = operation::remove();
				}
			}
		}
		aPrevIt = aResultIt;
		++aResultIt;
	}
}

template<typename Result>
void semantic_cleanup(Result& ioResult)
{
	cleanup_small_equalities(ioResult);
	cleanup_isolated_changes(ioResult);
	cleanup_change_overlaps(ioResult);
}


}  // namespace detail
}  // namespace diff
}  // namespace izi

#endif /* SEMANTIC_CLEANUP_H_ */
