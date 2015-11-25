#ifndef DIFF_BISECT_H_
#define DIFF_BISECT_H_

#include <iostream>

#include "algorithm.h"
#include "range_traits.h"
#include "operation.h"

namespace izi {
namespace diff {
namespace detail {

template<typename Traits, typename Iterator, typename Result>
void calculate(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult);

template<typename Iterator, typename Result>
inline void bisect_split(Iterator iBegin1, Iterator iMid1, Iterator iEnd1,
		Iterator iBegin2, Iterator iMid2, Iterator iEnd2,
		Result& oResult)
{
	calculate<void_traits>(iBegin1, iMid1, iBegin2, iMid2, oResult);
	calculate<void_traits>(iMid1, iEnd1, iMid2, iEnd2, oResult);
}

template<typename Iterator, typename Result>
void bisect(Iterator iBegin1, Iterator iEnd1, Iterator iBegin2, Iterator iEnd2, Result& oResult)
{
	typedef typename Result::value_type::second_type range_type;

	// Cache the text lengths to prevent multiple calls.
	const size_t aRng1Size = std::distance(iBegin1, iEnd1);
	const size_t aRng2Size = std::distance(iBegin2, iEnd2);
	const size_t max_d = (aRng1Size + aRng2Size + 1) / 2;
	const int v_offset = max_d;
	const int v_length = 2 * max_d;
	int *v1 = new int[v_length];
	int *v2 = new int[v_length];
	for (int x = 0; x < v_length; x++)
	{
		v1[x] = -1;
		v2[x] = -1;
	}
	v1[v_offset + 1] = 0;
	v2[v_offset + 1] = 0;
	const int delta = aRng1Size - aRng2Size;
	// If the total number of characters is odd, then the front path will
	// collide with the reverse path.
	const bool front = (delta % 2 != 0);
	// Offsets for start and end of k loop.
	// Prevents mapping of space beyond the grid.
	int k1start = 0;
	int k1end = 0;
	int k2start = 0;
	int k2end = 0;
	for (int d = 0; d < max_d; d++)
	{
		// Walk the front path one step.
		for (int k1 = -d + k1start; k1 <= d - k1end; k1 += 2)
		{
			const int k1_offset = v_offset + k1;
			int x1;
			if (k1 == -d || (k1 != d && v1[k1_offset - 1] < v1[k1_offset + 1]))
			{
				x1 = v1[k1_offset + 1];
			}
			else
			{
				x1 = v1[k1_offset - 1] + 1;
			}
			int y1 = x1 - k1;
			while (x1 < aRng1Size && y1 < aRng2Size && *(iBegin1 + x1) == *(iBegin2 + y1))
			{
				x1++;
				y1++;
			}
			v1[k1_offset] = x1;
			if (x1 > aRng1Size)
			{
				// Ran off the right of the graph.
				k1end += 2;
			}
			else if (y1 > aRng2Size)
			{
				// Ran off the bottom of the graph.
				k1start += 2;
			}
			else if (front)
			{
				int k2_offset = v_offset + delta - k1;
				if (k2_offset >= 0 && k2_offset < v_length && v2[k2_offset] != -1)
				{
					// Mirror x2 onto top-left coordinate system.
					int x2 = aRng1Size - v2[k2_offset];
					if (x1 >= x2)
					{
						// Overlap detected.
						delete [] v1;
						delete [] v2;
						bisect_split(iBegin1, iBegin1+x1, iEnd1, iBegin2, iBegin2+y1, iEnd2, oResult);
						return;
					}
				}
			}
		}

		// Walk the reverse path one step.
		for (int k2 = -d + k2start; k2 <= d - k2end; k2 += 2)
		{
			const int k2_offset = v_offset + k2;
			int x2;
			if (k2 == -d || (k2 != d && v2[k2_offset - 1] < v2[k2_offset + 1]))
			{
				x2 = v2[k2_offset + 1];
			}
			else
			{
				x2 = v2[k2_offset - 1] + 1;
			}
			int y2 = x2 - k2;
			while (x2 < aRng1Size && y2 < aRng2Size && *(iBegin1 + aRng1Size - x2 - 1) == *(iBegin2 + aRng2Size - y2 - 1))
			{
				x2++;
				y2++;
			}
			v2[k2_offset] = x2;
			if (x2 > aRng1Size)
			{
				// Ran off the left of the graph.
				k2end += 2;
			}
			else if (y2 > aRng2Size)
			{
				// Ran off the top of the graph.
				k2start += 2;
			}
			else if (!front)
			{
				int k1_offset = v_offset + delta - k2;
				if (k1_offset >= 0 && k1_offset < v_length && v1[k1_offset] != -1)
				{
					int x1 = v1[k1_offset];
					int y1 = v_offset + x1 - k1_offset;
					// Mirror x2 onto top-left coordinate system.
					x2 = aRng1Size - x2;
					if (x1 >= x2)
					{
						// Overlap detected.
						delete [] v1;
						delete [] v2;
						bisect_split(iBegin1, iBegin1+x1, iEnd1, iBegin2, iBegin2+y1, iEnd2, oResult);
						return;
					}
				}
			}
		}
	}
	delete [] v1;
	delete [] v2;

	oResult.push_back(std::make_pair(operation::remove(), range_type(iBegin1, iEnd1)));
	oResult.push_back(std::make_pair(operation::insert(), range_type(iBegin2, iEnd2)));
}

}  // namespace detail
}  // namespace diff
}  // namespace izi



#endif /* DIFF_BISECT_H_ */
