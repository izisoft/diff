#ifndef IZI_DIFF_OPERATION_H_
#define IZI_DIFF_OPERATION_H_

namespace izi {
namespace diff {
namespace detail {

enum OPERATION
{
	EQUAL = 0,
	INSERT,
	REMOVE
};

}  // namespace detail

class operation
{
public:
	operation(detail::OPERATION iValue): _value(iValue) {}

	static const operation& equal()
	{
		static const operation anOperation(detail::EQUAL);
		return anOperation;
	}

	static const operation& insert()
	{
		static const operation anOperation(detail::INSERT);
		return anOperation;
	}

	static const operation& remove()
	{
		static const operation anOperation(detail::REMOVE);
		return anOperation;
	}

	bool isEqual() const
	{
		return _value == detail::EQUAL;
	}

	bool isInsert() const
	{
		return _value == detail::INSERT;
	}

	bool isRemove() const
	{
		return _value == detail::REMOVE;
	}

	bool isChange() const
	{
		return _value != detail::EQUAL;
	}

	detail::OPERATION value() const
	{
		return _value;
	}

private:
	const detail::OPERATION _value;
};

}  // namespace diff
}  // namespace izi


#endif /* IZI_DIFF_OPERATION_H_ */
