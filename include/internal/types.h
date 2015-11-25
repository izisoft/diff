#ifndef DIFF_TYPES_H_
#define DIFF_TYPES_H_

#include <map>
#include <vector>

#include <string>

namespace izi {
namespace diff {

typedef unsigned long line_index;

template<typename Iterator>
struct range
{
	range(Iterator iBegin, Iterator iEnd): _begin(iBegin), _end(iEnd) {}

	bool operator<(const range iRange) const
	{
		return std::lexicographical_compare(_begin, _end, iRange._begin, iRange._end);
	}

	Iterator _begin;
	Iterator _end;
};

template<typename Iterator>
struct range_vector
{
	typedef std::vector<range<Iterator> > type;
};

typedef std::vector<line_index> line_vector;

template<typename Iterator>
struct line_map
{
	typedef std::map<range<Iterator>, line_index> type;
};

}  // namespace diff
}  // namespace izi


#endif /* DIFF_TYPES_H_ */
