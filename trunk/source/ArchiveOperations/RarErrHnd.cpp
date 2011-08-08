#define BIG_ENDIAN
#include <libunrar/rar.hpp>
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "Tools/tools.h"
#include "sys.h"

ErrorHandler::ErrorHandler()
{
	Clean();
}

void ErrorHandler::Clean()
{
	ExitCode=SUCCESS;
	ErrCount=0;
	EnableBreak=true;
	Silent=false;
	DoShutdown=false;
}


void ErrorHandler::MemoryError()
{
	StopProgress();
	ShowError("Not enought memory.");
	ExitCode = MEMORY_ERROR;
}


void ErrorHandler::OpenError(const char *FileName)
{
	StopProgress();
	ShowError("Cannot open file %s", FileName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::CloseError(const char *FileName)
{
	StopProgress();
	ShowError("Cannot close file %s", FileName);
	ExitCode = FATAL_ERROR;
}

void ErrorHandler::ReadError(const char *FileName)
{
	StopProgress();
	ShowError("Read Error in %s", FileName);
	ExitCode = FATAL_ERROR;
}

bool ErrorHandler::AskRepeatRead(const char *FileName)
{
	StopProgress();
	char output[200];
	snprintf(output, sizeof(output), "Read Error in file: %s", FileName);

	int choice = WindowPrompt(output, "Retry?", "Yes", "Cancel");

	return (choice != 0);
}

void ErrorHandler::WriteError(const char *ArcName,const char *FileName)
{
	StopProgress();
	ShowError("Write Error from %s to %s", ArcName, FileName);
	ExitCode = FATAL_ERROR;
}

bool ErrorHandler::AskRepeatWrite(const char *FileName,bool DiskFull)
{
	StopProgress();
	char output[200];
	if(DiskFull)
		snprintf(output, sizeof(output), "Write Error in file: %s. Disk is Full", FileName);
	else
		snprintf(output, sizeof(output), "Write Error in file: %s", FileName);

	int choice = WindowPrompt(output, "Retry?", "Yes", "Cancel");

	return (choice != 0);
}


void ErrorHandler::SeekError(const char *FileName)
{
	StopProgress();
	ShowError("Seek Error in %s", FileName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::GeneralErrMsg(const char *Msg)
{
	StopProgress();
	ShowError(Msg);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::MemoryErrorMsg()
{
	ExitCode = MEMORY_ERROR;
}


void ErrorHandler::OpenErrorMsg(const char *FileName)
{
	StopProgress();
	ShowError("Error opening %s", FileName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::OpenErrorMsg(const char *ArcName,const char *FileName)
{
	StopProgress();
	ShowError("Error opening %s in %s", FileName, ArcName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::CreateErrorMsg(const char *FileName)
{
	CreateErrorMsg(NULL,FileName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::CreateErrorMsg(const char *ArcName,const char *FileName)
{
	StopProgress();
	ShowError("Error received from %s in %s", FileName, ArcName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::ReadErrorMsg(const char *ArcName,const char *FileName)
{
	StopProgress();
	ShowError("Read error from %s in %s", FileName, ArcName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::WriteErrorMsg(const char *ArcName,const char *FileName)
{
	StopProgress();
	ShowError("Write error to %s in %s", FileName, ArcName);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::Exit(int ExitCode)
{
	Throw(ExitCode);
}

void ErrorHandler::ErrMsg(const char *ArcName,const char *fmt,...)
{
	safebuf char Msg[NM+1024];
	va_list argptr;
	va_start(argptr,fmt);
	vsprintf(Msg,fmt,argptr);
	va_end(argptr);
	if (*Msg)
	{
		StopProgress();
		ShowError("%s: %s", ArcName, Msg);
	}
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::SetErrorCode(int Code)
{
  switch(Code)
  {
	case WARNING:
	case USER_BREAK:
	  if (ExitCode==SUCCESS)
		ExitCode=Code;
	  break;
	case FATAL_ERROR:
	  if (ExitCode==SUCCESS || ExitCode==WARNING)
		ExitCode=FATAL_ERROR;
	  break;
	default:
	  ExitCode=Code;
	  break;
  }
  ErrCount++;
}


void ErrorHandler::SetSignalHandlers(bool Enable UNUSED)
{
//  EnableBreak=Enable;
//#if !defined(GUI) && !defined(_SFX_RTL_)
//#ifdef _WIN_32
//  SetConsoleCtrlHandler(Enable ? ProcessSignal:NULL,TRUE);
////  signal(SIGBREAK,Enable ? ProcessSignal:SIG_IGN);
//#else
//  signal(SIGINT,Enable ? ProcessSignal:SIG_IGN);
//  signal(SIGTERM,Enable ? ProcessSignal:SIG_IGN);
//#endif
//#endif
}

void ErrorHandler::Throw(int Code)
{
	if (Code==USER_BREAK && !EnableBreak)
		return;

	StopProgress();
	ShowError("Fatal error: %i. Must shutdown app.", Code);
	ExitCode = FATAL_ERROR;
}


void ErrorHandler::SysErrMsg()
{
	StopProgress();
	ShowError("System error received");
	ExitCode = FATAL_ERROR;
}

