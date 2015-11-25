#ifndef IZI_DIFF_RANGE_TRAITS_H_
#define IZI_DIFF_RANGE_TRAITS_H_

#include <string>

namespace izi {
namespace diff {
namespace detail {

struct line_range {};
struct non_line_range {};

template<typename Range>
struct range_traits
{
	typedef non_line_range range_type;
};

template<>
struct range_traits<std::string>
{
	typedef line_range range_type;

	static std::string::size_type min_size()
	{
		return 1000;
	}

	static char endl()
	{
		return '\n';
	}
};

template<>
struct range_traits<std::wstring>
{
	typedef line_range range_type;

	static std::wstring::size_type min_size()
	{
		return 1000;
	}

	static wchar_t endl()
	{
		return L'\n';
	}
};

typedef range_traits<void> void_traits;

}  // namespace detail
}  // namespace diff
}  // namespace izi


#endif /* IZI_DIFF_RANGE_TRAITS_H_ */
