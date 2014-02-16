#define SYSTEM_CORE_SOURCE
#define FIX_RELEASE_COM_COLLISION
#define TASK_INFO_DEFINED
#define NO_FILEOP_ALIAS
#include <stdhdrs.h>
#ifdef WIN32
#undef StrDup
#include <shlwapi.h>
#include <shellapi.h>
#undef StrRChr
#endif
#include <sack_types.h>
#include <deadstart.h>
#include <sharemem.h>
#include <sqlgetoption.h>
#include <timers.h>
#include <filesys.h>

#ifdef WIN32
#include <tlhelp32.h>
#endif

#ifdef __QNX__
#include <devctl.h>
#include <sys/procfs.h>
#endif

#ifdef __LINUX__
#include <sys/wait.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
extern char **environ;
#endif


#include <sack_system.h>

#ifdef __cplusplus
using namespace sack::timers;
#endif

//--------------------------------------------------------------------------
struct task_info_tag;

SACK_SYSTEM_NAMESPACE
//typedef void (CPROC*TaskEnd)(PTRSZVAL, struct task_info_tag *task_ended);

#include "taskinfo.h"

//-------------------------------------------------------------------------
//  Function/library manipulation routines...
//-------------------------------------------------------------------------


typedef struct loaded_function_tag
{
	_32 references;
	void (CPROC*function)(void );
	struct loaded_library_tag *library;
	DeclareLink( struct loaded_function_tag );
	TEXTCHAR name[];
} FUNCTION, *PFUNCTION;

#ifdef WIN32
typedef HMODULE HLIBRARY;
#else
typedef void* HLIBRARY;
#endif
typedef struct loaded_library_tag
{
	PTRSZVAL nLibrary; // when unloading...
	HLIBRARY library;
	PFUNCTION functions;
	DeclareLink( struct loaded_library_tag );
	TEXTCHAR *name;
	TEXTCHAR full_name[];
} LIBRARY, *PLIBRARY;

static PLIBRARY libraries;
static PTREEROOT pFunctionTree;

typedef struct task_info_tag TASK_INFO;

#ifdef HAVE_ENVIRONMENT
CTEXTSTR OSALOT_GetEnvironmentVariable(CTEXTSTR name)
{

#ifdef WIN32
	static int env_size;
	static TEXTCHAR *env;
	int size;
	if( size = GetEnvironmentVariable( name, NULL, 0 ) )
	{
		if( size > env_size )
		{
			if( env )
				ReleaseEx( (POINTER)env DBG_SRC );
			env = NewArray( TEXTCHAR, size + 10 );
			env_size = size + 10;
		}
		if( GetEnvironmentVariable( name, env, env_size ) )
			return env;
	}
	return NULL;
#else
	return getenv( name );
#endif
}

void OSALOT_SetEnvironmentVariable(CTEXTSTR name, CTEXTSTR value)
{
#if defined( WIN32 ) || defined( __CYGWIN__ )
	SetEnvironmentVariable( name, value );
#else
	setenv( name, value, TRUE );
#endif
}

void OSALOT_AppendEnvironmentVariable(CTEXTSTR name, CTEXTSTR value)
{
#if defined( WIN32 ) || defined( __CYGWIN__ )
	TEXTCHAR *oldpath;
	TEXTCHAR *newpath;
	_32 length;
	{
		int oldlen;
		oldpath = NewArray( TEXTCHAR, oldlen = ( GetEnvironmentVariable( name, NULL, 0 ) + 1 ) );
		GetEnvironmentVariable( name, oldpath, oldlen );
	}
	newpath = NewArray( TEXTCHAR, length = (_32)(strlen( oldpath ) + 2 + strlen(value)) );
	snprintf( newpath, length, WIDE("%s;%s"), oldpath, value );
	SetEnvironmentVariable( name, newpath );
	ReleaseEx( newpath DBG_SRC );
	ReleaseEx( oldpath DBG_SRC );
#else
	TEXTCHAR *oldpath = getenv( name );
	TEXTCHAR *newpath;
	size_t maxlen;
	newpath = NewArray( TEXTCHAR, maxlen = ( strlen( oldpath ) + strlen( value ) + 2 ) );
	snprintf( newpath, maxlen, WIDE("%s:%s"), oldpath, value );
	setenv( name, newpath, TRUE );
	ReleaseEx( newpath DBG_SRC );
#endif
}


void OSALOT_PrependEnvironmentVariable(CTEXTSTR name, CTEXTSTR value)
{
#if defined( WIN32 )|| defined( __CYGWIN__ )
	TEXTCHAR *oldpath;
	TEXTCHAR *newpath;
	int length;
	{
		int oldlen;
		oldpath = NewArray( TEXTCHAR, oldlen = ( GetEnvironmentVariable( name, NULL, 0 ) + 1 ) );
		GetEnvironmentVariable( name, oldpath, oldlen );
	}
	newpath = NewArray( TEXTCHAR, length = (_32)(strlen( oldpath ) + 2 + strlen(value)) );
	snprintf( newpath, length, WIDE("%s;%s"), value, oldpath );
	SetEnvironmentVariable( name, newpath );
	ReleaseEx( newpath DBG_SRC );
	ReleaseEx( oldpath DBG_SRC );
#else
	TEXTCHAR *oldpath = getenv( name );
	TEXTCHAR *newpath;
	int length;
	newpath = NewArray( TEXTCHAR, length = strlen( oldpath ) + strlen( value ) + 1 );
	snprintf( newpath, length, WIDE("%s:%s"), value, oldpath );
	setenv( name, newpath, TRUE );
	ReleaseEx( newpath DBG_SRC );
#endif
}
#endif

static void SetupSystemServices( void )
{
#ifdef _WIN32
	{
		TEXTCHAR filepath[256];
		TEXTCHAR *ext, *e1;
		GetModuleFileName( NULL, filepath, sizeof( filepath ) );
		ext = (TEXTSTR)StrRChr( (CTEXTSTR)filepath, '.' );
		if( ext )
			ext[0] = 0;
		e1 = (TEXTSTR)pathrchr( filepath );
		if( e1 )
		{
			e1[0] = 0;
			l.filename = StrDupEx( e1 + 1 DBG_SRC );
			l.load_path = StrDupEx( filepath DBG_SRC );
		}
		else
		{
			l.filename = StrDupEx( filepath DBG_SRC );
			l.load_path = StrDupEx( WIDE("") DBG_SRC );
		}

#ifdef HAVE_ENVIRONMENT
		OSALOT_SetEnvironmentVariable( WIDE("MY_LOAD_PATH"), filepath );
#endif

		GetCurrentPath( filepath, sizeof( filepath ) );
		l.work_path = StrDupEx( filepath DBG_SRC );
		SetDefaultFilePath( l.work_path );

#ifdef HAVE_ENVIRONMENT
		OSALOT_SetEnvironmentVariable( WIDE( "MY_WORK_PATH" ), filepath );
#endif
	}
#else
#  if defined( __QNX__ )
	{
		struct dinfo_s {
			procfs_debuginfo info;
			char pathbuffer[_POSIX_PATH_MAX];
		};

		struct dinfo_s dinfo;
		char buf[256], *pb;
		int proc_fd;
		proc_fd = open("/proc/self/as",O_RDONLY);
		if( proc_fd >= 0 )
		{
			int status;

			status = devctl( proc_fd, DCMD_PROC_MAPDEBUG_BASE, &dinfo, sizeof(dinfo),
								 0 );
			if( status != EOK )
			{
				lprintf( "Error in devctl() call. %s",
						  strerror(status) );
				l.filename = "FailedToReadFilenaem";
				l.load_path = ".";
            l.work_path = ".";
				return;
			}
			close(proc_fd);
		}
		snprintf( buf, 256, "/%s", dinfo.info.path );
		pb = (char*)pathrchr(buf);
		if( pb )
		{
			pb[0]=0;
			l.filename = StrDupEx( pb + 1 DBG_SRC );
		}
		else
		{
			l.filename = StrDupEx( buf DBG_SRC );
			buf[0] = '.';
			buf[1] = 0;
		}

		if( StrCmp( buf, "/." ) == 0 )
			GetCurrentPath( buf, 256 );
		//lprintf( WIDE("My execution: %s"), buf);
		l.load_path = StrDupEx( buf DBG_SRC );
		setenv( WIDE("MY_LOAD_PATH"), l.load_path, TRUE );
		//strcpy( pMyPath, buf );

		GetCurrentPath( buf, sizeof( buf ) );
		setenv( WIDE( "MY_WORK_PATH" ), buf, TRUE );
		l.work_path = StrDupEx( buf DBG_SRC );
		SetDefaultFilePath( l.work_path );
	}
#  else
	// this might be clever to do, auto export the LD_LIBRARY_PATH
	// but if we loaded this library, then didn't we already have a good path?
	// use /proc/self to get to cmdline
	// which has the whole invokation of this process.
	{
		/* #include unistd.h, stdio.h, string.h */
#    ifdef __ANDROID__
			l.filename = GetProgramName();
			l.load_path = GetProgramPath();
#    else
		{
			char buf[256], *pb;
			int n;
			n = readlink("/proc/self/exe",buf,256);
			if( n >= 0 )
			{
				buf[n]=0; //linux
				if( !n )
				{
					strcpy( buf, WIDE(".") );
					buf[ n = readlink( WIDE("/proc/curproc/"),buf,256)]=0; // fbsd
				}
			}
			else
				strcpy( buf, WIDE(".")) ;
			pb = strrchr(buf,'/');
			if( pb )
				pb[0]=0;
			else
				pb = buf - 1;
			//lprintf( WIDE("My execution: %s"), buf);
			l.filename = StrDupEx( pb + 1 DBG_SRC );
			l.load_path = StrDupEx( buf DBG_SRC );
			setenv( WIDE("MY_LOAD_PATH"), buf, TRUE );
			//strcpy( pMyPath, buf );

			GetCurrentPath( buf, sizeof( buf ) );
			setenv( WIDE( "MY_WORK_PATH" ), buf, TRUE );
			l.work_path = StrDupEx( buf DBG_SRC );
         SetDefaultFilePath( l.work_path );
		}
	{
		TEXTCHAR *oldpath;
		TEXTCHAR *newpath;
		oldpath = getenv( "LD_LIBRARY_PATH" );
		if( oldpath )
		{
			newpath = NewArray( char, (_32)((oldpath?strlen( oldpath ):0) + 2 + strlen(l.load_path)) );
			sprintf( newpath, WIDE("%s:%s"), l.load_path
					 , oldpath );
			setenv( WIDE("LD_LIBRARY_PATH"), newpath, 1 );
			ReleaseEx( newpath DBG_SRC );
		}
	}
	{
		TEXTCHAR *oldpath;
		TEXTCHAR *newpath;
		oldpath = getenv( "PATH" );
		if( oldpath )
		{
			newpath = NewArray( char, (_32)((oldpath?strlen( oldpath ):0) + 2 + strlen(l.load_path)) );
			sprintf( newpath, WIDE("%s:%s"), l.load_path
					 , oldpath );
			setenv( WIDE("PATH"), newpath, 1 );
			ReleaseEx( newpath DBG_SRC );
		}
	}
#    endif
		//<x`int> rathar: main() { char buf[1<<7]; buf[readlink("/proc/self/exe",buf,1<<7)]=0; puts(buf); }
		//<x`int> main() {  }
		//<x`int>
	}
#  endif
#endif
}

PRIORITY_PRELOAD( SetupPath, OSALOT_PRELOAD_PRIORITY )
{
#ifndef __LINUX__
	extern void InitCo( void );
#endif
	SetupSystemServices();
#ifndef __LINUX__
	InitCo();
#endif
}

#ifndef __NO_OPTIONS__
PRELOAD( SetupSystemOptions )
{
	l.flags.bLog = SACK_GetProfileIntEx( GetProgramName(), WIDE( "SACK/System/Enable Logging" ), 0, TRUE );
	if( SACK_GetProfileIntEx( GetProgramName(), WIDE( "SACK/System/Auto prepend program location to PATH environment" ), 0, TRUE ) )
		OSALOT_PrependEnvironmentVariable( WIDE("PATH"), l.load_path );

}
#endif

//--------------------------------------------------------------------------

#ifdef WIN32
#ifndef _M_CEE_PURE
static BOOL CALLBACK CheckWindowAndSendKill( HWND hWnd, LPARAM lParam )
{
	_32 idThread, idProcess;
	PTASK_INFO task = (PTASK_INFO)lParam;
	idThread = GetWindowThreadProcessId( hWnd, (LPDWORD)&idProcess );

   /*
	{
		TEXTCHAR title[256];
		GetWindowText( hWnd, title, sizeof( title ) );
		lprintf( "Window [%s] = %d %d", title, idProcess, idThread );
	}
   */
	if( task->pi.dwProcessId == idProcess )
	{
		// found the window to kill...
		PostThreadMessage( idThread, WM_QUIT, 0xD1E, 0 );
		return FALSE;
	}
	return TRUE;
}
#endif

//--------------------------------------------------------------------------

int CPROC EndTaskWindow( PTASK_INFO task )
{
	return EnumWindows( CheckWindowAndSendKill, (LPARAM)task );
}
#endif

//--------------------------------------------------------------------------

LOGICAL CPROC StopProgram( PTASK_INFO task )
{
#ifdef WIN32
#ifndef UNDER_CE
	int error;
	if( !GenerateConsoleCtrlEvent( CTRL_C_EVENT, task->pi.dwProcessId ) )
	{
      error = GetLastError();
      lprintf( WIDE( "Failed to send CTRL_C_EVENT %d" ), error );
		if( !GenerateConsoleCtrlEvent( CTRL_BREAK_EVENT, task->pi.dwProcessId ) )
		{
         error = GetLastError();
			lprintf( WIDE( "Failed to send CTRL_BREAK_EVENT %d" ), error );
		}
	}
#endif
#else
        // kill( ) ?
#endif


    return TRUE;
}

PTRSZVAL CPROC TerminateProgram( PTASK_INFO task )
{
	if( task )
	{
#if defined( WIN32 )
		int bDontCloseProcess = 0;
#endif
		if( !task->flags.closed )
		{
			int nowait = 0;
			task->flags.closed = 1;

			//lprintf( WIDE( "%ld, %ld %p %p" ), task->pi.dwProcessId, task->pi.dwThreadId, task->pi.hProcess, task->pi.hThread );

#if defined( WIN32 )
			if( WaitForSingleObject( task->pi.hProcess, 0 ) != WAIT_OBJECT_0 )
			{
				// try using ctrl-c, ctrl-break to end process...
				if( !StopProgram( task ) )
				{
					xlprintf(LOG_DEBUG+1)( WIDE("Program did not respond to ctrl-c or ctrl-break...") );
					// if ctrl-c fails, try finding the window, and sending exit (systray close)
					if( EndTaskWindow( task ) )
					{
						xlprintf(LOG_DEBUG+1)( WIDE("failed to find task window to send postquitmessage...") );
						// didn't find the window - result was continue_enum with no more (1)
                  // so didn't find the window - nothing to wait for, fall through
						nowait = 1;
					}
				}
				if( nowait || ( WaitForSingleObject( task->pi.hProcess, 500 ) != WAIT_OBJECT_0 ) )
				{
					xlprintf(LOG_DEBUG+1)( WIDE("Terminating process....") );
					bDontCloseProcess = 1;
					if( !TerminateProcess( task->pi.hProcess, 0xD1E ) )
					{
						HANDLE hTmp;
						lprintf( WIDE("Failed to terminate process... %p %ld : %d (will try again with OpenProcess)"), task->pi.hProcess, task->pi.dwProcessId, GetLastError() );
						hTmp = OpenProcess( SYNCHRONIZE|PROCESS_TERMINATE, FALSE, task->pi.dwProcessId);
						if( !TerminateProcess( hTmp, 0xD1E ) )
						{
							lprintf( WIDE("Failed to terminate process... %p %ld : %d"), task->pi.hProcess, task->pi.dwProcessId, GetLastError() );
						}
						CloseHandle( hTmp );
					}
				}
			}
			if( !task->EndNotice )
			{
				lprintf( WIDE( "Closing handle (no end notification)" ) );
				// task end notice - will get the event and close these...
				CloseHandle( task->pi.hThread );
				task->pi.hThread = 0;
				if( !bDontCloseProcess )
				{
					lprintf( WIDE( "And close process handle" ) );
					CloseHandle( task->pi.hProcess );
					task->pi.hProcess = 0;
				}
				else
					lprintf( WIDE( "Keeping process handle" ) );
			}
//			else
//				lprintf( WIDE( "Would have close handles rudely." ) );
#else
         kill( task->pid, SIGTERM );
			// wait a moment for it to die...
#endif
		}
		//if( !task->EndNotice )
		//{
		//	Release( task );
		//}
		//task = NULL;
	}
	return 0;
}

//--------------------------------------------------------------------------

SYSTEM_PROC( void, SetProgramUserData )( PTASK_INFO task, PTRSZVAL psv )
{
	if( task )
		task->psvEnd = psv;
}

//--------------------------------------------------------------------------

_32 GetTaskExitCode( PTASK_INFO task )
{
	if( task )
		return task->exitcode;
	return 0;
}


PTRSZVAL CPROC WaitForTaskEnd( PTHREAD pThread )
{
	PTASK_INFO task = (PTASK_INFO)GetThreadParam( pThread );
#ifdef __LINUX__
	while( !task->pid ) {
		Relinquish();
	}
#endif
	// this should be considered the owner of this.
	if( !task->EndNotice )
	{
		// application is dumb, hold the task for him; otherwise 
		// the application is aware that after EndNotification the task is no longer valid.
		Hold( task );
	}
	//if( task->EndNotice )
	{
		// allow other people to delete it...
		//Hold( task );
#if defined( WIN32 )
		WaitForSingleObject( task->pi.hProcess, INFINITE );
		GetExitCodeProcess( task->pi.hProcess, &task->exitcode );
#elif defined( __LINUX__ )
		waitpid( task->pid, NULL, 0 );
#endif
		task->flags.process_ended = 1;

		if( task->hStdOut.hThread )
		{
#ifdef _WIN32
         // vista++ so this won't work for XP support...
			static BOOL (WINAPI *MyCancelSynchronousIo)( HANDLE hThread ) = (BOOL(WINAPI*)(HANDLE))-1;
			if( (int)MyCancelSynchronousIo == -1 )
				MyCancelSynchronousIo = (BOOL(WINAPI*)(HANDLE))LoadFunction( WIDE( "kernel32.dll" ), WIDE( "CancelSynchronousIo" ) );
			if( MyCancelSynchronousIo )
			{
				if( !MyCancelSynchronousIo( GetThreadHandle( task->hStdOut.hThread ) ) )
				{
					// maybe the read wasn't queued yet....
					//lprintf( "Failed to cancel IO on thread %d %d", GetThreadHandle( task->hStdOut.hThread ), GetLastError() );
				}
			}
			else
			{
				static BOOL (WINAPI *MyCancelIoEx)( HANDLE hFile,LPOVERLAPPED ) = (BOOL(WINAPI*)(HANDLE,LPOVERLAPPED))-1;
				if( (int)MyCancelIoEx == -1 )
					MyCancelIoEx = (BOOL(WINAPI*)(HANDLE,LPOVERLAPPED))LoadFunction( WIDE( "kernel32.dll" ), WIDE( "CancelIoEx" ) );
				if( MyCancelIoEx )
					MyCancelIoEx( task->hStdOut.handle, NULL );
				else
				{
					DWORD written;
					//lprintf( WIDE( "really? You're still using xp or less?" ) );
					task->flags.bSentIoTerminator = 1;
					if( !WriteFile( task->hWriteOut, WIDE( "\x04" ), 1, &written, NULL ) )
					lprintf( WIDE( "write pipe failed! %d" ), GetLastError() );
					//lprintf( "Pipe write was %d", written );
				}
			}
#endif
		}

		// wait for task last output before notification of end of task.
		while( task->pOutputThread )
			Relinquish();

		if( task->EndNotice )
			task->EndNotice( task->psvEnd, task );
#if defined( WIN32 )
		lprintf( WIDE( "Closing process and thread handles." ) );
		if( task->pi.hProcess )
		{
			CloseHandle( task->pi.hProcess );
			task->pi.hProcess = 0;
		}
		if( task->pi.hThread )
		{
			CloseHandle( task->pi.hThread );
			task->pi.hThread = 0;
		}
#endif
		ReleaseEx( task DBG_SRC );
	}
	//TerminateProgram( task );
	return 0;
}


//--------------------------------------------------------------------------

#ifdef WIN32
static int DumpError( void )
{
	lprintf( WIDE("Failed create process:%d"), GetLastError() );
	return 0;
}
#endif

#ifdef WIN32

static BOOL CALLBACK EnumDesktopProc( LPTSTR lpszDesktop,
												  LPARAM lParam
												)
{
	lprintf( WIDE( "Desktop found [%s]" ), lpszDesktop );
	return 1;
}


void EnumDesktop( void )
{
   // I'm running on some windows station, right?
   //HWINSTA GetProcessWindowStation();
	if( EnumDesktops( NULL, EnumDesktopProc, (LPARAM)(PTRSZVAL)0 ) )
	{
      // returned non-zero value from enumdesktopproc?
      // failed to find?
	}

}

static BOOL CALLBACK EnumStationProc( LPTSTR lpszWindowStation, LPARAM lParam )
{
	lprintf( WIDE( "station found [%s]" ), lpszWindowStation );
   return 1;
}

void EnumStations( void )
{
	if( EnumWindowStations( EnumStationProc, 0 ) )
	{
	}
}

void SetDefaultDesktop( void )
{
	//return;
	{
	HDESK lngDefaultDesktop;
	HWINSTA lngWinSta0;
   HWINSTA station = GetProcessWindowStation();
	HDESK desk = GetThreadDesktop( GetCurrentThreadId() );
	DWORD length;
   char buffer[256];

	lprintf( WIDE( "Desktop this is %p %p" ), station, desk );

	GetUserObjectInformation( desk, UOI_NAME, buffer, sizeof( buffer ), &length );
   lprintf( WIDE( "desktop is %s" ), buffer );
	GetUserObjectInformation( station, UOI_NAME, buffer, sizeof( buffer ), &length );
   lprintf( WIDE( "station is %s" ), buffer );

	EnumDesktop();
   EnumStations();

   // these should be const strings, but they're not... add typecast for GCC
	lngWinSta0 = OpenWindowStation( (LPTSTR)WIDE( "WinSta0" ), FALSE, WINSTA_ALL_ACCESS );
	//lngWinSta0 = OpenWindowStation(WIDE( "msswindowstation" ), FALSE, WINSTA_ALL_ACCESS );
   lprintf( WIDE( "sta = %p %d" ), lngWinSta0, GetLastError() );
	if( !SetProcessWindowStation(lngWinSta0) )
      lprintf( WIDE( "Failed station set?" ) );

   // these should be const strings, but they're not... add typecast for GCC
	lngDefaultDesktop = OpenDesktop( (LPTSTR)WIDE( "Default" ), 0, FALSE, 0x10000000);
	//lngDefaultDesktop = OpenDesktop(WIDE( "WinSta0" ), 0, FALSE, 0x10000000);
   lprintf( WIDE( "defa = %p" ), lngDefaultDesktop );
	if( !SetThreadDesktop(lngDefaultDesktop) )
		lprintf( WIDE( "Failed desktop set?" ) );
	}
}
		/*
 HDESK WINAPI OpenInputDesktop(
  __in  DWORD dwFlags,
  __in  BOOL fInherit,
  __in  ACCESS_MASK dwDesiredAccess
);

*/

DWORD GetExplorerProcessID()
{
   static TEXTCHAR process_find[128];
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	DWORD temp = 0;
	ZeroMemory(&pe32,sizeof(pe32));

	if( !process_find[0] )
	{
#ifndef __NO_OPTIONS__
		SACK_GetProfileStringEx( GetProgramName(), WIDE( "SACK/System/Impersonate Process" ), WIDE( "explorer.exe" ), process_find, sizeof( process_find ), TRUE );
#endif
	}
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if(Process32First(hSnapshot,&pe32))
	{
		do
		{
			//lprintf( WIDE( "Thing is %s" ), pe32.szExeFile );
			if(!strcmp(pe32.szExeFile,process_find))
			{
				//MessageBox(0,pe32.szExeFile,WIDE( "test" ),0);
				temp = pe32.th32ProcessID;
				break;
			}

		}while(Process32Next(hSnapshot,&pe32));
	}
	return temp;
}

void ImpersonateInteractiveUser( void )
{
	HANDLE hToken = NULL;
	HANDLE hProcess = NULL;

	DWORD processID;
	SetDefaultDesktop();
	processID = GetExplorerProcessID();
	//lprintf( WIDE( "Enum EDesktops..." ) );
	//EnumDesktop();
	//lprintf( WIDE( "explorer is %p" ), processID );
	if( processID)
	{
		hProcess =
			OpenProcess(
							PROCESS_ALL_ACCESS,
							TRUE,
							processID );

		if( hProcess)
		{
			//lprintf( WIDE( "Success getting process %p" ), hProcess );
			if( OpenProcessToken(
										hProcess,
										TOKEN_EXECUTE |
										TOKEN_READ |
										TOKEN_QUERY |
										TOKEN_ASSIGN_PRIMARY |
										TOKEN_QUERY_SOURCE |
										TOKEN_WRITE |
										TOKEN_DUPLICATE |
									   TOKEN_IMPERSONATE,
										&hToken))
			{
            //lprintf( WIDE( "Sucess opening token" ) );
				if( ImpersonateLoggedOnUser( hToken ) )
					;
				else
					lprintf( WIDE( "Fail impersonate %d" ), GetLastError() );
				CloseHandle( hToken );
			}
			else
				lprintf( WIDE( "Failed opening token %d" ), GetLastError() );
			CloseHandle( hProcess );
		}
		else
			lprintf( WIDE( "Failed open process: %d" ), GetLastError() );
	}
	else
		lprintf( WIDE( "Failed get explorer process: %d" ), GetLastError() );
}

HANDLE GetImpersonationToken( void )
{
	HANDLE hToken = NULL;
	HANDLE hProcess = NULL;

	DWORD processID;
	processID = GetExplorerProcessID();
   //lprintf( WIDE( "Enum EDesktops..." ) );
	//EnumDesktop();
   //lprintf( WIDE( "explorer is %p" ), processID );
	if( processID)
	{
		hProcess =
			OpenProcess(
							PROCESS_ALL_ACCESS,
							TRUE,
							processID );

		if( hProcess)
		{
         //lprintf( WIDE( "Success getting process %p" ), hProcess );
			if( OpenProcessToken(
										hProcess,
										TOKEN_EXECUTE |
										TOKEN_READ |
										TOKEN_QUERY |
										TOKEN_ASSIGN_PRIMARY |
										TOKEN_QUERY_SOURCE |
										TOKEN_WRITE |
										TOKEN_DUPLICATE |
									   TOKEN_IMPERSONATE,
										&hToken))
			{
            //lprintf( WIDE( "Sucess opening token" ) );
				//if( ImpersonateLoggedOnUser( hToken ) )
            //   ;
				//else
            //   lprintf( WIDE( "Fail impersonate %d" ), GetLastError() );
				//CloseHandle( hToken );
			}
			else
            lprintf( WIDE( "Failed opening token %d" ), GetLastError() );
			CloseHandle( hProcess );
		}
		else
			lprintf( WIDE( "Failed open process: %d" ), GetLastError() );
	}
   else
		lprintf( WIDE( "Failed get explorer process: %d" ), GetLastError() );
   return hToken;
}

void EndImpersonation( void )
{
	RevertToSelf();
}
#endif

//--------------------------------------------------------------------------
#ifdef WIN32
int TryShellExecute( PTASK_INFO task, CTEXTSTR path, CTEXTSTR program, PTEXT cmdline )
{
#if 0
#if defined( OLD_MINGW_SUX ) || defined( __WATCOMC__ )
	typedef struct _SHELLEXECUTEINFO {
		DWORD     cbSize;
		ULONG     fMask;
		HWND      hwnd;
		LPCTSTR   lpVerb; // null default
		LPCTSTR   lpFile;
		LPCTSTR   lpParameters;
		LPCTSTR   lpDirectory;
		int       nShow;
		HINSTANCE hInstApp;
		LPVOID    lpIDList;
		LPCTSTR   lpClass;
		HKEY      hkeyClass;
		DWORD     dwHotKey;
		union {
			HANDLE hIcon;
			HANDLE hMonitor;
		} DUMMYUNIONNAME;
		HANDLE    hProcess;
	} SHELLEXECUTEINFO, *LPSHELLEXECUTEINFO;
#endif
#endif
	SHELLEXECUTEINFO execinfo;
	MemSet( &execinfo, 0, sizeof( execinfo ) );
	execinfo.cbSize = sizeof( SHELLEXECUTEINFO );
	execinfo.fMask = SEE_MASK_NOCLOSEPROCESS  // need this to get process handle back for terminate later
		| SEE_MASK_FLAG_NO_UI
		| SEE_MASK_NO_CONSOLE
		//| SEE_MASK_NOASYNC
		;
	execinfo.lpFile = program;
	execinfo.lpDirectory = path;
	{
		TEXTCHAR *params;
		for( params = GetText( cmdline ); params[0] && params[0] != ' '; params++ );
		if( params[0] )
		{
			//lprintf( WIDE( "adding extra parames [%s]" ), params );
			execinfo.lpParameters = params;
		}
	}
	execinfo.nShow = SW_SHOWNORMAL;
	if( ShellExecuteEx( &execinfo ) )
	{
		if( (int)execinfo.hInstApp > 32)
		{
			switch( (int)execinfo.hInstApp )
			{
			case 42:
				lprintf( WIDE( "No association picked : %d (gle:%d)" ), (int)execinfo.hInstApp , GetLastError() );
				break;
			}
			lprintf( WIDE( "sucess with shellexecute of(%d) %s " ), execinfo.hInstApp, program );
			task->pi.hProcess = execinfo.hProcess;
			task->pi.hThread = 0;
			return TRUE;
		}
		else
		{
			switch( (int)execinfo.hInstApp )
			{
			default:
				lprintf( WIDE( "Shell exec error : %d (gle:%d)" ), (int)execinfo.hInstApp , GetLastError() );
				break;
			}
			return FALSE;
		}
	}
	else
		lprintf( WIDE( "Shellexec error %d" ), GetLastError() );
	return FALSE;

}
#endif
//--------------------------------------------------------------------------

// Run a program completely detached from the current process
// it runs independantly.  Program does not suspend until it completes.
// No way at all to know if the program works or fails.
SYSTEM_PROC( PTASK_INFO, LaunchProgramEx )( CTEXTSTR program, CTEXTSTR path, PCTEXTSTR args, TaskEnd EndNotice, PTRSZVAL psv )
{
	PTASK_INFO task;
	if( program && program[0] )
	{
#ifdef WIN32
	PVARTEXT pvt = VarTextCreate();
	PTEXT cmdline;
	int first = TRUE;
	//TEXTCHAR saved_path[256];
	TEXTSTR expanded_path = ExpandPath( program );
	TEXTSTR expanded_working_path = path?ExpandPath( path ):ExpandPath( WIDE(".") );
	LOGICAL needs_quotes;
	task = New( TASK_INFO );
	MemSet( task, 0, sizeof( TASK_INFO ) );
	task->psvEnd = psv;
	task->EndNotice = EndNotice;
	if( StrCmp( path, WIDE( "." ) ) == 0 )
	{
		path = NULL;
		Deallocate( TEXTSTR, expanded_working_path );
		expanded_working_path = NULL;
	}

	if( expanded_path && StrChr( expanded_path, ' ' ) )
		needs_quotes = TRUE;
	else if( expanded_working_path && StrChr( expanded_working_path, ' ' ) )
		needs_quotes = TRUE;
	else
		needs_quotes = FALSE;

	xlprintf(LOG_DEBUG+1)( WIDE( "quotes?%s path[%s] program[%s]" ), needs_quotes?WIDE( "yes" ):WIDE( "no" ), expanded_working_path, expanded_path );

	if( needs_quotes )
		vtprintf( pvt, WIDE( "\"" ) );

	if( !IsAbsolutePath( expanded_path ) && expanded_working_path )
	{
		//lprintf( "needs working path too" );
		vtprintf( pvt, WIDE("%s/"), expanded_working_path );
	}
	vtprintf( pvt, WIDE("%s"), expanded_path );

	if( needs_quotes )
		vtprintf( pvt, WIDE( "\"" ) );

	if( args && args[0] )// arg[0] is passed with linux programs, and implied with windows.
		args++;
	while( args && args[0] )
	{
		/* restore quotes around parameters with spaces */
		if( args[0][0] == 0 )
			vtprintf( pvt, WIDE(" \"\"" ) );
		else if( strchr( args[0], ' ' ) )
			vtprintf( pvt, WIDE(" \"%s\"" ), args[0] );
		else
			vtprintf( pvt, WIDE(" %s"), args[0] );
		first = FALSE;
		args++;
	}
	cmdline = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	MemSet( &task->si, 0, sizeof( STARTUPINFO ) );
	task->si.cb = sizeof( STARTUPINFO );

	if( ( CreateProcess( NULL //program
						  , GetText( cmdline )
						  , NULL, NULL, FALSE
						  , 
#ifdef UNDER_CE 
						  0
#else
						  CREATE_NEW_PROCESS_GROUP|CREATE_NEW_CONSOLE
#endif
						  , NULL
						  , expanded_working_path
						  , &task->si
						  , &task->pi ) || DumpError() ) ||
		( CreateProcess( program
						 , GetText( cmdline )
						 , NULL, NULL, FALSE
						 , 
#ifdef UNDER_CE 
						  0
#else
						  CREATE_NEW_PROCESS_GROUP|CREATE_NEW_CONSOLE
#endif
						 , NULL
						 , expanded_working_path
						 , &task->si
						 , &task->pi ) || DumpError() ) ||
		( TryShellExecute( task, expanded_working_path, program, cmdline ) ) ||
		( CreateProcess( program
						 , NULL // GetText( cmdline )
						 , NULL, NULL, FALSE
						 , 
#ifdef UNDER_CE 
						  0
#else
						  CREATE_NEW_PROCESS_GROUP|CREATE_NEW_CONSOLE
#endif
						 , NULL
						 , expanded_working_path
						 , &task->si
						 , &task->pi ) || DumpError() ) ||
      0
	  )
	{
		xlprintf(LOG_DEBUG+1)( WIDE("Success running %s[%s] %p: %d"), program, GetText( cmdline ), task->pi.hProcess, GetLastError() );
		if( EndNotice )
			ThreadTo( WaitForTaskEnd, (PTRSZVAL)task );
	}
	else
	{
		xlprintf(LOG_DEBUG+1)( WIDE("Failed to run %s[%s]: %d"), program, GetText( cmdline ), GetLastError() );
		ReleaseEx( task DBG_SRC );
		task = NULL;
	}
	Deallocate( TEXTSTR, expanded_path );
	Deallocate( TEXTSTR, expanded_working_path );
	LineRelease( cmdline );
	return task;
#endif
#ifdef __LINUX__
	{
		pid_t newpid;
		task = New( TASK_INFO );
		MemSet( task, 0, sizeof( TASK_INFO ) );
		task->psvEnd = psv;
		task->EndNotice = EndNotice;
		//if( EndNotice )
		ThreadTo( WaitForTaskEnd, (PTRSZVAL)task );
		if( !( newpid = fork() ) )
		{
		// in case exec fails, we need to
		// drop any registered exit procs...
			DispelDeadstart();

                        chdir( path );

                        execve( program, (char *const*)args, environ );
                        {
                            char *tmp = StrDupEx( getenv( "PATH" ) DBG_SRC );
                            char *tok;
                            for( tok = strtok( tmp, ":" ); tok; tok = strtok( NULL, ":" ) )
                            {
                                char fullname[256];
                                snprintf( fullname, sizeof( fullname ), "%s/%s", tok, program );

                                lprintf( "program:[%s][%s][%s]", fullname,args[0], getenv("PATH") );
                                ((char**)args)[0] = fullname;
                                execve( fullname, (char*const*)args, environ );
                            }
                        }
			lprintf( "exec failed - and this is ALLL bad...[%s]%d", program, errno );
			//DebugBreak();
			// well as long as this runs before
			// the other all will be well...
			task = NULL;
			// shit - what can I do now?!
			exit(0); // just in case exec fails... need to fault this.
		}
		task->pid = newpid;
		//lprintf( WIDE("Forked, and set the pid..") );
		// how can I know if the command failed?
		// well I can't - but the user's callback will be invoked
		// when the above exits.
		return task;
	}
#endif
	}
	return FALSE;
}

SYSTEM_PROC( PTASK_INFO, LaunchProgram )( CTEXTSTR program, CTEXTSTR path, PCTEXTSTR args )
{
   return LaunchProgramEx( program, path, args, NULL, 0 );
}

//--------------------------------------------------------------------------

SYSTEM_PROC( int, CallProgram )( CTEXTSTR program, CTEXTSTR path, PCTEXTSTR args, ... )
{
	// should hook into stdin,stdout,stderr...
	// also keep track of process handles so that
	// the close of said program could be monitored.....
   return 0;
}

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

void InvokeLibraryLoad( void )
{
	void (CPROC *f)(void);
	PCLASSROOT data = NULL;
	PCLASSROOT event_root = GetClassRoot( WIDE("SACK/system/library/load_event") );
	CTEXTSTR name;
	for( name = GetFirstRegisteredName( event_root, &data );
		 name;
		  name = GetNextRegisteredName( &data ) )
	{
		f = GetRegisteredProcedureExx( data,(CTEXTSTR)NULL,void,name,(void));
		if( f )
		{
			f();
		}
	}
}



SYSTEM_PROC( generic_function, LoadFunctionExx )( CTEXTSTR libname, CTEXTSTR funcname, LOGICAL bPrivate  DBG_PASS )
{
	static int nLibrary;
	PLIBRARY library = libraries;

	while( library )
	{
		if( StrCmp( library->name, libname ) == 0 )
			break;
		library = library->next;
	}
	// don't really NEED anything else, in case we need to start before deadstart invokes.
	if( !l.load_path )
	{
		lprintf( WIDE( "Init Load Path" ) );
		SetupSystemServices();
	}
	if( !library )
	{
		size_t maxlen = strlen( l.load_path ) + 1 + strlen( libname ) + 1;
		library = NewPlus( LIBRARY, sizeof(TEXTCHAR)*((maxlen<0xFFFFFF)?(_32)maxlen:0) );
		if( !IsAbsolutePath( libname ) )
		{
			library->name = library->full_name
				+ snprintf( library->full_name, maxlen, WIDE("%s/"), l.load_path );
			snprintf( library->name
				, maxlen - (library->name-library->full_name)
				, WIDE("%s"), libname );
		}
		else
		{
			library->name = library->full_name;
			StrCpy( library->name, libname );
		}
		library->functions = NULL;
		SuspendDeadstart();
#ifdef _WIN32
		// with deadstart suspended, the library can safely register
		// all of its preloads.  Then invoke will release suspend
		// so final initializers in application can run.
		library->library = LoadLibrary( library->name );
		if( !library->library )
		{
			library->library = LoadLibrary( library->full_name );
			if( !library->library )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to load %s[%s](%s) failed: %d."), libname, library->full_name, funcname?funcname:WIDE("all"), GetLastError() );
				ReleaseEx( library DBG_SRC );
				ResumeDeadstart();
				return NULL;
			}
		}
#else
#  ifndef __ANDROID__
		// ANDROID This will always fail from the application manager.
		library->library = dlopen( library->name, RTLD_LAZY|(bPrivate?RTLD_LOCAL: RTLD_GLOBAL) );
		if( !library->library )
		{
			_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to load %s%s(%s) failed: %s."), bPrivate?"(local)":"(global)", libname, funcname?funcname:"all", dlerror() );
#  endif
			library->library = dlopen( library->full_name, RTLD_LAZY|(bPrivate?RTLD_LOCAL:RTLD_GLOBAL) );
			if( !library->library )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to load  %s%s(%s) failed: %s."), bPrivate?"(local)":"(global)", library->full_name, funcname?funcname:"all", dlerror() );
				ReleaseEx( library DBG_SRC );
				ResumeDeadstart();
				return NULL;
			}
#  ifndef __ANDROID__
		}
#  endif
#endif
#ifdef __cplusplus_cli
		{
			void (CPROC *f)( void );
			if( l.flags.bLog )
				lprintf( WIDE( "GetInvokePreloads" ) );
			f = (void(CPROC*)(void))GetProcAddress( library->library, "InvokePreloads" );
			if( f )
				f();
		}
#endif
		{
			//DebugBreak();
			ResumeDeadstart();
			// actually bInitialDone will not be done sometimes
			// and we need to force this here.
			InvokeDeadstart();
		}
		InvokeLibraryLoad();
		library->nLibrary = ++nLibrary;
		LinkThing( libraries, library );
	}
	if( funcname )
	{
		PFUNCTION function = library->functions;
		while( function )
		{
			if( strcmp( function->name, funcname ) == 0 )
				break;
			function = function->next;
		}
		if( !function )
		{
			int len;
			function = NewPlus( FUNCTION, (len=(sizeof(TEXTCHAR)*( (_32)StrLen( funcname ) + 1 ) ) ) );
			snprintf( function->name, len, WIDE( "%s" ), funcname );
			function->library = library;
			function->references = 0;
#ifdef _WIN32
#ifdef __cplusplus_cli
			char *procname = CStrDup( function->name );
			if( l.flags.bLog )
				lprintf( WIDE( "Get:%s" ), procname );
			if( !(function->function = (generic_function)GetProcAddress( library->library, procname )) )
#else
#ifdef _UNICODE
			{
			char *tmp;
#endif
  			if( l.flags.bLog )
  				lprintf( WIDE( "Get:%s" ), function->name );
			if( !(function->function = (generic_function)GetProcAddress( library->library
#ifdef _UNICODE
																						  , tmp = DupTextToChar( function->name )
#else
																						  , function->name
#endif
																						  )) )
#endif
			{
				TEXTCHAR tmpname[128];
				snprintf( tmpname, sizeof( tmpname ), WIDE("_%s"), funcname );
#ifdef __cplusplus_cli
				char *procname = CStrDup( tmpname );
				if( l.flags.bLog )
					lprintf( WIDE( "Get:%s" ), procname );
				function->function = (generic_function)GetProcAddress( library->library, procname );
				ReleaseEx( procname DBG_SRC );
#else
				if( l.flags.bLog )
					lprintf( WIDE( "Get:%s" ), function->name );
				function->function = (generic_function)GetProcAddress( library->library
#ifdef _UNICODE
																					  , WcharConvert( tmpname )
#else
																					  , tmpname
#endif
																					  );
#endif
			}
#ifdef __cplusplus_cli
			ReleaseEx( procname DBG_SRC );
#else
#ifdef _UNICODE
			Deallocate( char *, tmp );
			}
#endif
#endif
			if( !function->function )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to get function %s from %s failed. %d"), funcname, libname, GetLastError() );
				ReleaseEx( function DBG_SRC );
				return NULL;
			}
#else
			if( !(function->function = (generic_function)dlsym( library->library, function->name )) )
			{
				char tmpname[128];
				sprintf( tmpname, WIDE("_%s"), funcname );
				function->function = (generic_function)dlsym( library->library, tmpname );
			}
			if( !function->function )
			{
				_xlprintf( 2 DBG_RELAY)( WIDE("Attempt to get function %s from %s failed. %s"), funcname, libname, dlerror() );
				ReleaseEx( function DBG_SRC );
				return NULL;
			}
#endif
			if( !pFunctionTree )
				pFunctionTree = CreateBinaryTree();
			//lprintf( WIDE("Adding function %p"), function->function );
			AddBinaryNode( pFunctionTree, function, (PTRSZVAL)function->function );
			LinkThing( library->functions, function );
		}
		function->references++;
		return function->function;
	}
	else
	{
		return (generic_function)(/*extend precisionfirst*/(PTRSZVAL)library->nLibrary); // success, but no function possible.
	}
	return NULL;
}

SYSTEM_PROC( generic_function, LoadPrivateFunctionEx )( CTEXTSTR libname, CTEXTSTR funcname DBG_PASS )
{
   return LoadFunctionExx( libname, funcname, TRUE DBG_RELAY );
}

SYSTEM_PROC( generic_function, LoadFunctionEx )( CTEXTSTR libname, CTEXTSTR funcname DBG_PASS )
{
   return LoadFunctionExx( libname, funcname, FALSE DBG_RELAY );
}
#undef LoadFunction
SYSTEM_PROC( generic_function, LoadFunction )( CTEXTSTR libname, CTEXTSTR funcname )
{
	return LoadFunctionEx( libname,funcname DBG_SRC);
}

//-------------------------------------------------------------------------

// pass the address of the function pointer - this
// will gracefully erase that reference also.
SYSTEM_PROC( int, UnloadFunctionEx )( generic_function *f DBG_PASS )
{
	if( !f  )
		return 0;
	_xlprintf( 1 DBG_RELAY )( WIDE("Unloading function %p"), *f );
	if( (PTRSZVAL)(*f) < 1000 )
	{
		// unload library only...
		if( !(*f) )  // invalid result...
			return 0;
		{
			PLIBRARY library;
			PTRSZVAL nFind = (PTRSZVAL)(*f);
			for( library = libraries; library; library = NextLink( library ) )
			{
				if( nFind == library->nLibrary )
				{
#ifdef _WIN32
               // should make sure noone has loaded a specific function.
					FreeLibrary ( library->library );
					UnlinkThing( library );
					ReleaseEx( library DBG_SRC );
#else
#endif
				}
			}
		}
	}
	{
		PFUNCTION function = (PFUNCTION)FindInBinaryTree( pFunctionTree, (PTRSZVAL)(*f) );
		PLIBRARY library;
		if( function &&
		    !(--function->references) )
		{
			UnlinkThing( function );
			lprintf( WIDE( "Should remove the node from the tree here... but it crashes intermittantly. (tree list is corrupted)" ) );
			//RemoveLastFoundNode( pFunctionTree );
			library = function->library;
			if( !library->functions )
			{
#ifdef _WIN32
				FreeLibrary( library->library );
#else
				dlclose( library->library );
#endif
				UnlinkThing( library );
				ReleaseEx( library DBG_SRC );
			}
			ReleaseEx( function DBG_SRC );
			*f = NULL;
		}
		else
		{
			lprintf( WIDE("function was not found - or ref count = %") _32f WIDE(" (5566 means no function)"), function?function->references:5566 );
		}
	}
	return FALSE;
}

//-------------------------------------------------------------------------

#ifndef __ANDROID__
SYSTEM_PROC( PTHREAD, SpawnProcess )( CTEXTSTR filename, va_list args )
{
	PTRSZVAL (CPROC *newmain)( PTHREAD pThread );
	newmain = (PTRSZVAL(CPROC*)(PTHREAD))LoadFunction( filename, WIDE("main") );
	if( newmain )
	{
		// hmm... suppose I should even thread through my own little header here
      // then when the thread exits I can get a positive acknowledgement?
      return ThreadTo( newmain, (PTRSZVAL)args );
	}
	return NULL;
}
#endif

//---------------------------------------------------------------------------

TEXTSTR GetArgsString( PCTEXTSTR pArgs )
{
	static TEXTCHAR args[256];
	int len = 0, n;
	args[0] = 0;
	// arg[0] should be the same as program name...
	for( n = 1; pArgs && pArgs[n]; n++ )
	{
		int space = (strchr( pArgs[n], ' ' )!=NULL);
		len += snprintf( args + len, sizeof( args ) - len * sizeof( TEXTCHAR ), WIDE("%s%s%s%s")
							, n>1?WIDE(" "):WIDE("")
							, space?WIDE("\""):WIDE("")
							, pArgs[n]
							, space?WIDE("\""):WIDE("")
							);
	}
	return args;
}


#ifdef __ANDROID__
static char *program_name;
static char *program_path;
static char *working_path;

void SACKSystemSetProgramPath( char *path )
{
   program_path = strdup( path );
}
void SACKSystemSetProgramName( char *name )
{
   program_name = strdup( name );
}
void SACKSystemSetWorkingPath( char *name )
{
   working_path = strdup( name );
}
#endif



CTEXTSTR GetProgramName( void )
{
#ifdef __ANDROID__
	return program_name;
#else
	if( !l.filename )
	{
		SetupSystemServices();
		if( !l.filename )
		{
			DebugBreak();
			return NULL;
		}
	}
	return l.filename;
#endif
}

CTEXTSTR GetProgramPath( void )
{
#ifdef __ANDROID__
	return program_path;
#else
	if( !l.load_path )
	{
		SetupSystemServices();
		if( !l.load_path )
		{
			DebugBreak();
			return NULL;
		}
	}
	return l.load_path;
#endif
}

CTEXTSTR GetStartupPath( void )
{
#ifdef __ANDROID__
	return working_path;
#else
	if( !l.work_path )
	{
		SetupSystemServices();
		if( !l.work_path )
		{
			DebugBreak();
			return NULL;
		}
	}
	return l.work_path;
#endif
}

LOGICAL IsSystemShuttingDown( void )
{
#ifdef WIN32
	static HANDLE h = INVALID_HANDLE_VALUE;
	if( h == INVALID_HANDLE_VALUE )
      h = CreateEvent( NULL, TRUE, FALSE, WIDE( "Windows Is Shutting Down" ) );
	if( h != INVALID_HANDLE_VALUE )
		if( WaitForSingleObject( h, 0 ) == WAIT_OBJECT_0 )
			return TRUE;
#endif
   return FALSE;
}


SACK_SYSTEM_NAMESPACE_END


