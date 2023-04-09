#pragma once
#include <system_error>
#include <crtdbg.h>
#include <Windows.h>
#include <assert.h>

#undef min
#undef max

#if defined(DEBUG) | defined(_DEBUG)
#define _NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define _NEW new
#endif

#define SAFE_DELETE(ptr)\
if(ptr != nullptr)\
{\
	delete ptr;\
	ptr = nullptr; \
}

#define SAFE_DELETE_ARRAY(ptr)\
if(ptr != nullptr)\
{\
	delete[] ptr;\
	ptr = nullptr; \
}

#define SAFE_RELEASE(com)\
if(com != nullptr)\
{\
	com->Release();\
	com = nullptr; \
}

#define HRESULT_TRACE_STRING(hr) std::system_category().message((int)hr)

#if defined( DEBUG ) || defined( _DEBUG )
#define _ASSERT_EXPR_A(expr, msg) \
	(void)((!!(expr)) || \
	(1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, __FUNCDNAME__, "%s", msg)) || \
	(_CrtDbgBreak(), 0))
#else
#define  _ASSERT_EXPR_A(expr, expr_str) ((void)0)
#endif

class HrException
	: public std::runtime_error
{
public:
	explicit HrException(HRESULT hr)
		:std::runtime_error(HRESULT_TRACE_STRING(hr))
	{}

	virtual ~HrException() = default;
};

#define THROW_IF_FAILED(hr)	if(FAILED(hr)){ throw HrException(hr); }
#define HRESULT_ASSERT(hr)	_ASSERT_EXPR_A(SUCCEEDED(hr),HRESULT_TRACE_STRING(hr).c_str())
#define WIN32_ASSERT(x)		HRESULT_ASSERT(HRESULT_FROM_WIN32(x))