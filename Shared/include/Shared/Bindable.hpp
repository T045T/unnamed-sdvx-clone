#pragma once

template <typename R, typename... A>
struct IFunctionBinding
{
	virtual ~IFunctionBinding() = default;
	virtual R Call(A ... args) = 0;
};

/* Member function binding */
template <typename Class, typename R, typename... A>
struct ObjectBinding : IFunctionBinding<R, A...>
{
	ObjectBinding(Class* object, R (Class::*func)(A ...))
		: object(object), func(func)
	{};

	R Call(A ... args) override
	{
		return ((object)->*func)(args...);
	}

	Class* object;
	R (Class::*func)(A ...);
};

/* Static function binding */
template <typename R, typename... A>
struct StaticBinding : IFunctionBinding<R, A...>
{
	StaticBinding(R (*func)(A ...))
		: func(func)
	{};

	R Call(A ... args) override
	{
		return (*func)(args...);
	}

	R (*func)(A ...);
};

/* Lambda function binding */
template <typename T, typename R, typename... A>
struct LambdaBinding : IFunctionBinding<R, A...>
{
	// Copies the given lambda
	LambdaBinding(T&& lambda)
		: lambda(std::forward<T>(lambda))
	{};

	R Call(A ... args) override
	{
		return lambda(args...);
	}

	T lambda;
};

/* Constant return value binding */
template <typename R, typename... A>
struct ConstantBinding : IFunctionBinding<R, A...>
{
	ConstantBinding(const R& value)
		: value(value)
	{};

	R Call(A ...) override
	{
		return value;
	}

	R value;
};
