#ifndef _MY_EXCEPTION
#define _MY_EXCEPTION
#include <exception>
#include <iostream>
#include <string>
using namespace std;
#define THROW_EXCEPTION(ExClass, args)       \
	do                                          \
	{											\
		ExClass e(args);						\
		e.Init(__FILE__, __FUNCTION__, __LINE__); \
		throw e;								\
	}while(0);

#define DEFINE_EXCEPTION(ExClass, Base)					\
	ExClass(const string& msg = "") throw():Base(msg){}	\
	~ExClass() throw(){}								\
	string GetClassName() const {						\
		return #ExClass;								\
	}													
class ExceptionBase: public exception
{
public:
	ExceptionBase(const string &msg = "") throw();
	virtual ~ExceptionBase(void) throw();
	void Init(const char* file, const char* func, int line);
	virtual string GetClassName() const;
	virtual string GetMessage() const;
	const char* what() const throw();
	const string& ToString() const;
	string GetStackTrace() const;
protected:
	string m_msg;
	const char *m_file;
	const char *m_func;
	int m_line;
private:
	enum {MAX_STACK_TRACE_SIZE = 50};
	void *m_stackTrace[MAX_STACK_TRACE_SIZE];
	int m_stackTraceSize;
	mutable string m_what;
};


class ExceptionDatabase:public ExceptionBase {
public:
	DEFINE_EXCEPTION(ExceptionDatabase, ExceptionBase);
};


class ExceptionParsejson:public ExceptionBase {
public:
	DEFINE_EXCEPTION(ExceptionParsejson, ExceptionBase);
};

class ExceptionSocket: public ExceptionBase {
public:
	DEFINE_EXCEPTION(ExceptionSocket, ExceptionBase);

};

class ExceptionMessageQueue: public ExceptionBase {
public:
    DEFINE_EXCEPTION(ExceptionMessageQueue, ExceptionBase);
};


#endif