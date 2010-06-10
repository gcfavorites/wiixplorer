#include "rar.hpp"

int RarErrorCode = 0;

//!Quick and dirty error handle for WiiXplorer by Dimok
extern "C"
{
    extern void ShowError(const char * format, ...);
    void Sys_BackToLoader();
}
extern void StopProgress();
extern int WindowPrompt(const char *a, const char *b, const char *c, const char *d, const char *e, const char *f, bool t);


static bool UserBreak;

ErrorHandler::ErrorHandler()
{
  Clean();
}


void ErrorHandler::Clean()
{
  ExitCode=SUCCESS;
  RarErrorCode=0;
  ErrCount=0;
  EnableBreak=true;
  Silent=false;
  DoShutdown=false;
}


void ErrorHandler::MemoryError()
{
  StopProgress();
  ShowError("Not enought memory.");
  RarErrorCode = MEMORY_ERROR;
}


void ErrorHandler::OpenError(const char *FileName)
{
  StopProgress();
  ShowError("Cannot open file %s", FileName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::CloseError(const char *FileName)
{
  StopProgress();
  ShowError("Cannot close file %s", FileName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::ReadError(const char *FileName)
{
  StopProgress();
  ShowError("Read Error in %s", FileName);
  RarErrorCode = FATAL_ERROR;
}


bool ErrorHandler::AskRepeatRead(const char *FileName)
{
    StopProgress();
    char output[200];
    snprintf(output, sizeof(output), "Read Error in file: %s", FileName);

    int choice = WindowPrompt(output, "Retry?", "Yes", "Cancel", 0, 0, true);

    return (choice != 0);
}


void ErrorHandler::WriteError(const char *ArcName,const char *FileName)
{
  StopProgress();
  ShowError("Write Error from %s to %s", ArcName, FileName);
  RarErrorCode = FATAL_ERROR;
}


#ifdef _WIN_32
void ErrorHandler::WriteErrorFAT(const char *FileName)
{
#if !defined(SILENT) && !defined(SFX_MODULE)
  SysErrMsg();
  ErrMsg(NULL,St(MNTFSRequired),FileName);
#endif
#if !defined(SILENT) && !defined(SFX_MODULE) || defined(RARDLL)
  Throw(WRITE_ERROR);
#endif
}
#endif


bool ErrorHandler::AskRepeatWrite(const char *FileName,bool DiskFull)
{
    StopProgress();
    char output[200];
    if(DiskFull)
        snprintf(output, sizeof(output), "Write Error in file: %s. Disk is Full", FileName);
    else
        snprintf(output, sizeof(output), "Write Error in file: %s", FileName);

    int choice = WindowPrompt(output, "Retry?", "Yes", "Cancel", 0, 0, true);

    return (choice != 0);
}


void ErrorHandler::SeekError(const char *FileName)
{
  StopProgress();
  ShowError("Seek Error in %s", FileName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::GeneralErrMsg(const char *Msg)
{
  StopProgress();
  ShowError(Msg);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::MemoryErrorMsg()
{
  RarErrorCode = MEMORY_ERROR;
}


void ErrorHandler::OpenErrorMsg(const char *FileName)
{
  StopProgress();
  ShowError("Error opening %s", FileName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::OpenErrorMsg(const char *ArcName,const char *FileName)
{
  StopProgress();
  ShowError("Error opening %s in %s", FileName, ArcName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::CreateErrorMsg(const char *FileName)
{
  CreateErrorMsg(NULL,FileName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::CreateErrorMsg(const char *ArcName,const char *FileName)
{
  StopProgress();
  ShowError("Error received from %s in %s", FileName, ArcName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::ReadErrorMsg(const char *ArcName,const char *FileName)
{
  StopProgress();
  ShowError("Read error from %s in %s", FileName, ArcName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::WriteErrorMsg(const char *ArcName,const char *FileName)
{
  StopProgress();
  ShowError("Write error to %s in %s", FileName, ArcName);
  RarErrorCode = FATAL_ERROR;
}


void ErrorHandler::Exit(int ExitCode)
{
#ifndef SFX_MODULE
  Alarm();
#endif
  Throw(ExitCode);
}


#ifndef GUI
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
    ShowError("Msg");
  }
  RarErrorCode = FATAL_ERROR;
}
#endif


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


//#if !defined(GUI) && !defined(_SFX_RTL_)
//#ifdef _WIN_32
//BOOL __stdcall ProcessSignal(DWORD SigType)
//#else
//#if defined(__sun)
//extern "C"
//#endif
//void _stdfunction ProcessSignal(int SigType)
//#endif
//{
//#ifdef _WIN_32
//  if (SigType==CTRL_LOGOFF_EVENT)
//    return(TRUE);
//#endif
//  UserBreak=true;
//  mprintf(St(MBreak));
//  for (int I=0;!File::RemoveCreated() && I<3;I++)
//  {
//#ifdef _WIN_32
//    Sleep(100);
//#endif
//  }
//#if defined(USE_RC) && !defined(SFX_MODULE) && !defined(_WIN_CE)
//  ExtRes.UnloadDLL();
//#endif
//  exit(USER_BREAK);
//#if defined(_WIN_32) && !defined(_MSC_VER)
//  // never reached, just to avoid a compiler warning
//  return(TRUE);
//#endif
//}
//#endif


void ErrorHandler::SetSignalHandlers(bool Enable)
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
    RarErrorCode = FATAL_ERROR;
    return;
#ifdef ALLOW_EXCEPTIONS
  throw Code;
#else
  File::RemoveCreated();
  exit(Code);
#endif
}


void ErrorHandler::SysErrMsg()
{
    StopProgress();
    ShowError("System error received");
  RarErrorCode = FATAL_ERROR;
}

