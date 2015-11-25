#ifndef IZI_DIFF_H_
#define IZI_DIFF_H_

#include <list>
#include <map>

#include "internal/calculation.h"
#include "internal/range_traits.h"
#include "internal/semantic_cleanup.h"

namespace izi {
namespace diff {

template<typename Range, typename Traits = detail::range_traits<Range> >
class result
{
public:
	typedef std::pair<operation, Range> value_type;
	typedef typename Range::value_type element_type;
	typedef std::list<value_type> container_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;

	void calculate(const Range& iRange1, const Range& iRange2)
	{
		detail::calculate<Traits>(iRange1.begin(), iRange1.end(), iRange2.begin(), iRange2.end(), _result);
	}

	void cleanup()
	{
		detail::semantic_cleanup<Traits>(_result);
	}

public:
	iterator begin()
	{
		return _result.begin();
	}

	const_iterator begin() const
	{
		return _result.begin();
	}

	iterator end()
	{
		return _result.end();
	}

	const_iterator end() const
	{
		return _result.end();
	}

	typename container_type::size_type size() const
	{
		return _result.size();
	}

	bool empty() const
	{
		return _result.empty();
	}

private:
	container_type _result;
};

}  // namespace diff
}  // namespace izi

#endif /* IZI_DIFF_H_ */
