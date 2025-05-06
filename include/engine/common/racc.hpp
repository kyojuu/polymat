#ifndef RACC_H
#define RACC_H

#include <vector>

template<typename T>
struct RaccBase
{
	uint32_t max_values_count;
	std::vector<T> values;
	uint32_t current_index;
	T pop_value;

	RaccBase(uint32_t max_size=8) 
		: max_values_count(max_size), 
		values(max_size, 0.0f),
		current_index(0),
		pop_value(0.0f)
	{ }

	bool addValueBase(T val)
	{
		const bool pop = current_index >= max_values_count;
		const uint32_t i = getIndex();
		pop_value = values[i];
		values[i] = val;
		++current_index;
		return pop;
	}

	uint32_t getCount() const
	{
		return std::min(current_index + 1, max_values_count);
	}

	virtual T get() const = 0;

	operator T() const
	{
		return get();
	}

protected:
	uint32_t getIndex(int32_t offset = 0) const
	{
		return (current_index + offset) % max_values_count;
	}
};

template<typename T>
struct  RDiff : public RaccBase<T>
{
	RDiff(uint32_t max_size = 8) : RaccBase<T>(max_size)
	{ }

	void addValue(T v)
	{
		RaccBase<T>::addValueBase(v);
	}

	T get() const override
	{
		return RaccBase<T>::values[RaccBase<T>::getIndex(-1)] - RaccBase<T>::values[RaccBase<T>::getIndex()];
	}
};
#endif // !RACC_H
