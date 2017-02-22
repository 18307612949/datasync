#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/cac49598/Commfun.o \
	${OBJECTDIR}/_ext/cac49598/LogData.o \
	${OBJECTDIR}/_ext/cac49598/LogError.o \
	${OBJECTDIR}/_ext/cac49598/MyException.o \
	${OBJECTDIR}/_ext/cac49598/lock.o \
	${OBJECTDIR}/_ext/1b0e3895/AliMessageQueue.o \
	${OBJECTDIR}/_ext/1b0e3895/Config.o \
	${OBJECTDIR}/_ext/1b0e3895/MyMessageListener.o \
	${OBJECTDIR}/_ext/1b0e3895/__fdelt_chk.o \
	${OBJECTDIR}/_ext/1b0e3895/base64.o \
	${OBJECTDIR}/_ext/1b0e3895/comm.o \
	${OBJECTDIR}/_ext/1b0e3895/memcpy.o \
	${OBJECTDIR}/_ext/1b0e3895/urlencode.o \
	${OBJECTDIR}/CDownloadBinlog.o \
	${OBJECTDIR}/CProcessMqData.o \
	${OBJECTDIR}/HandlerClientSocket.o \
	${OBJECTDIR}/SocketServer.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../lib/mq

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/datasync

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/datasync: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/datasync ${OBJECTFILES} ${LDLIBSOPTIONS} -lpthread -lonsclient4cpp -ldl -Wl,-wrap,memcpy

${OBJECTDIR}/_ext/cac49598/Commfun.o: ../public/Commfun.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cac49598
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cac49598/Commfun.o ../public/Commfun.cpp

${OBJECTDIR}/_ext/cac49598/LogData.o: ../public/LogData.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cac49598
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cac49598/LogData.o ../public/LogData.cpp

${OBJECTDIR}/_ext/cac49598/LogError.o: ../public/LogError.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cac49598
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cac49598/LogError.o ../public/LogError.cpp

${OBJECTDIR}/_ext/cac49598/MyException.o: ../public/MyException.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cac49598
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cac49598/MyException.o ../public/MyException.cpp

${OBJECTDIR}/_ext/cac49598/lock.o: ../public/lock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cac49598
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cac49598/lock.o ../public/lock.cpp

${OBJECTDIR}/_ext/1b0e3895/AliMessageQueue.o: ../syncshare/AliMessageQueue.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/AliMessageQueue.o ../syncshare/AliMessageQueue.cpp

${OBJECTDIR}/_ext/1b0e3895/Config.o: ../syncshare/Config.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/Config.o ../syncshare/Config.cpp

${OBJECTDIR}/_ext/1b0e3895/MyMessageListener.o: ../syncshare/MyMessageListener.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/MyMessageListener.o ../syncshare/MyMessageListener.cpp

${OBJECTDIR}/_ext/1b0e3895/__fdelt_chk.o: ../syncshare/__fdelt_chk.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.c) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/__fdelt_chk.o ../syncshare/__fdelt_chk.c

${OBJECTDIR}/_ext/1b0e3895/base64.o: ../syncshare/base64.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/base64.o ../syncshare/base64.cpp

${OBJECTDIR}/_ext/1b0e3895/comm.o: ../syncshare/comm.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/comm.o ../syncshare/comm.cpp

${OBJECTDIR}/_ext/1b0e3895/memcpy.o: ../syncshare/memcpy.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.c) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/memcpy.o ../syncshare/memcpy.c

${OBJECTDIR}/_ext/1b0e3895/urlencode.o: ../syncshare/urlencode.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1b0e3895
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1b0e3895/urlencode.o ../syncshare/urlencode.cpp

${OBJECTDIR}/CDownloadBinlog.o: CDownloadBinlog.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CDownloadBinlog.o CDownloadBinlog.cpp

${OBJECTDIR}/CProcessMqData.o: CProcessMqData.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CProcessMqData.o CProcessMqData.cpp

${OBJECTDIR}/HandlerClientSocket.o: HandlerClientSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HandlerClientSocket.o HandlerClientSocket.cpp

${OBJECTDIR}/SocketServer.o: SocketServer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SocketServer.o SocketServer.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DLOG_EXCEPTION -I. -I../syncshare -I../include -I../public -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/datasync

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
