#include "StdAfx.h"
#include "MyException.h"
#include <sstream>
#include <LogError.h>
ExceptionBase::ExceptionBase(const string &msg) throw()
	:m_msg(msg), 
	m_file("<unknown file>"),
	m_func("<unknown func>"),
	m_line(-1),
	m_stackTraceSize(0){
	
}


ExceptionBase::~ExceptionBase(void) throw()
{
}

void ExceptionBase::Init(const char* file, const char* func, int line) {
	m_file = file;
	m_func = func;
	m_line = line;
#ifdef LOG_EXCEPTION
	CLogError::getInstance()->WriteErrorLog(0, ToString().c_str());
#endif
//	m_stackTraceSize = backtrace(m_stackTrace,MAX_STACK_TRACE_SIZE);
}


string ExceptionBase::GetClassName() const {
	return "ExceptionBase";
}


const char* ExceptionBase::what() const throw() {
	return ToString().c_str();
}

const string& ExceptionBase::ToString() const {
	if(m_what.empty()) {
		stringstream sstr("");
		if(m_line > 0) {
			sstr<< "file["<< m_file<< "]&line["<< m_line<< "]";
		}
		sstr<< "&func["<< m_func<< "]";
		sstr<< "&exception["<< GetClassName()<< "]";
		if(!GetMessage().empty()) {
			sstr<< "&msg["<< GetMessage()<< "]";
		}
		m_what = sstr.str();
	}
	return m_what;
}

string ExceptionBase::GetMessage() const {
	return m_msg;
}

string ExceptionBase::GetStackTrace() const {
	return "";
}

