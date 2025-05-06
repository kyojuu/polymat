#ifndef NUMBERGENERATOR_H
#define NUMBERGENERATOR_H

#include <random>

class NumberGenerator
{
protected:
	std::random_device rd_;
	std::mt19937 gen_;

	NumberGenerator() : gen_(1)
	{ }
};

template<typename T>
class RealNumberGenerator : public NumberGenerator
{
private:
	std::uniform_real_distribution<T> dis_;

public:
	RealNumberGenerator() 
		: NumberGenerator(), dis_(0.0f, 1.0f)
	{ }

	// random_device is not copyable 
	RealNumberGenerator(const RealNumberGenerator<T>& right)
		: NumberGenerator(), dis_(right.dis_)
	{ }

	float get()
	{
		return dis_(gen_);
	}

	float getUnder(T max)
	{
		return get() * max;
	}

	float getRange(T min, T max)
	{
		return min + get() * (max - min);
	}

	float getRange(T width)
	{
		return getRange(-width * 0.5f, width * 0.5f);
	}
};

template<typename T>
class RNG
{
private:
	static RealNumberGenerator<T> gen_;

public:
	static T get()
	{
		return gen_.get();
	}

	static float getUnder(T max)
	{
		return gen_.getUnder(max);
	}

	static uint64_t getUintUnder(uint64_t max)
	{
		return static_cast<uint64_t>(gen_.getUnder(static_cast<float>(max) + 1.0f));
	}

	static float getRange(T min, T max)
	{
		return gen_.getRange(min, max);
	}

	static float getRange(T width)
	{
		return gen_.getRange(width);
	}

	static float getFullRange(T width)
	{
		return gen_.getRange(static_cast<T>(2.0f) * width);
	}

	static bool proba(float threshold)
	{
		return get() < threshold;
	}
};

using RNGf = RNG<float>;

template<typename T>
class IntegerNumberGenerator : public NumberGenerator
{
public:
	IntegerNumberGenerator()
		: NumberGenerator()
	{ }

	// randmom_device is not copyable
	IntegerNumberGenerator(const IntegerNumberGenerator<T>&)
		: NumberGenerator()
	{ }

	T getUnder(T max)
	{
		std::uniform_int_distribution<std::mt19937::result_type> dist(0, max);
		return dist(gen_);
	}

	T getRange(T min, T max)
	{
		std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
		return dist(gen_);
	}
};

template<typename T>
class RNGi
{
private:
	static IntegerNumberGenerator<T> gen_;

public:
	static T getUnder(T max)
	{
		return gen_.getUnder(max);
	}

	static T getRange(T min, T max)
	{
		return gen_.getRange(min, max);
	}
};

template<typename T>
IntegerNumberGenerator<T> RNGi<T>::gen_;

using RNGi32 = RNGi<int32_t>;
using RNGi64 = RNGi<int64_t>;
using RNGu32 = RNGi<uint32_t>;
using RNGu64 = RNGi<uint64_t>;

#endif // !NUMBERGENERATOR_H
