#define NO_UNICODE_C
#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#include <sack_types.h>
#include <procreg.h>
#include <stdio.h>
#include <string.h>
#if defined( _WIN32 ) && !defined( __TURBOC__ )
#include <stdhdrs.h> // windows lean_and_mean
#ifndef UNDER_CE
#include <io.h>  // findfirst,findnext, fileinfo
#endif
#else
#include <dirent.h> // opendir etc..
#include <sys/stat.h>
#endif
#include <filesys.h>

FILESYS_NAMESPACE

#include "filesys_local.h"

#define MAX_PATH_NAME 512

// DEBUG_COMPARE 1 == full debug
// DEBUG_COMPARE 2 == quieter debug
#ifdef _DEBUG
#define DEBUG_COMPARE 3
#else
#define DEBUG_COMPARE 999
#endif
//--------------------------------------------------------------------------

 int  CompareMask ( CTEXTSTR mask, CTEXTSTR name, int keepcase )
{
	int m = 0, n = 0;
	int anymatch;
	int wasanymatch, wasmaskmatch;
	int matchone;
	TEXTCHAR namech, maskch;
	if( !mask )
		return 1;
	if( !name )
		return 0;
try_mask:
	anymatch = 0;
	wasanymatch = 0;
	wasmaskmatch = 0;
	matchone = 0;
#if ( DEBUG_COMPARE < 3 )
	lprintf( WIDE("Check %s vs %s"), mask + m, name );
#endif
	do
	{
		if( mask[m] == '\t' || mask[m] == '|' )
		{
			//Log1( WIDE("Found mask seperator - skipping to next mask :%s"), name );
			n = 0;
			m++;
         continue;
		}
		while( mask[m] == '*' )
		{
			anymatch = 1;
			m++;
		}
		while( mask[m] == '?' )
		{
#if ( DEBUG_COMPARE < 2 )
         //Log( WIDE("Match any one character") );
#endif
			matchone++;
         m++;
		}
		if( !keepcase && name[n]>= 'a' && name[n] <= 'z' )
			namech = name[n] - ('a' - 'A');
		else
			namech = name[n];

		if( !keepcase && mask[m]>= 'a' && mask[m] <= 'z' )
			maskch = mask[m] - ('a' - 'A');
		else
			maskch = mask[m];

		if( mask[m] == '/' )
			maskch = '\\';
		if( name[n] == '/' )
         namech = '\\';

      if( matchone )
		{
			matchone--;
			n++;
		}
		else if(
#if ( DEBUG_COMPARE < 2 )
				  lprintf( WIDE("Check %c == %c?"), maskch, namech ),
#endif
				  maskch == namech )
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" yes.") );
#endif
		 	if( anymatch )
		 	{
			 	wasanymatch = n;
			 	wasmaskmatch = m;
			 	anymatch = 0;
			}
		 	n++;
	 		m++;
		}
		else if(
#if ( DEBUG_COMPARE < 2 )
				  lprintf( WIDE(" no. Any match?") ),
#endif
				  anymatch )
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" yes"));
#endif
			n++;
		}
		else if(
#if ( DEBUG_COMPARE < 2 )
				  lprintf( WIDE(" No. wasanymatch?") ),
#endif
				  wasanymatch )
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" yes. reset to anymatch.") );
#endif
			n = wasanymatch;
			m = wasmaskmatch;
			anymatch = 1;
			n++;
		}
		else
		{
#if ( DEBUG_COMPARE < 2 )
			lprintf( WIDE(" No. match failed.") );
#endif
			break;
		}
	}while( name[n] );
	// 0 or more match a *
   // so auto match remaining *
	while( mask[m] && mask[m] == '*' )
      m++;
#if ( DEBUG_COMPARE < 3 )
	lprintf( WIDE("Skipping to next mask") );
#endif
	if( mask[m] &&
	    ( mask[m] != '\t' &&
		   mask[m] != '|' ) )
	{
		int mask_m = m;
		while( mask[m] )
		{
			if( mask[m] == '\t' || mask[m] == '|' )
			{
				n = 0;
				m++;
				break;
			}
			m++;
		}
		if( mask[m] )
			goto try_mask;
      m = mask_m;
	}
   //lprintf( WIDE("Result: %d %c %d"), matchone, mask[m], name[n] );
	// match ???? will not match abc 
	// a??? abc not match
	if( !matchone && (!mask[m] || mask[m] == '\t' || mask[m] == '|' ) && !name[n] )
		return 1;
	return  0;
}

//--------------------------------------------------------------------------
//---------------------------------------------------------------------------

typedef struct result_buffer
{
	TEXTSTR buffer;
	int len;
	int result_len;
} RESULT_BUFFER, *PRESULT_BUFFER;

static void CPROC MatchFile( PTRSZVAL psvUser, CTEXTSTR name, int flags )
{
	PRESULT_BUFFER buffer = (PRESULT_BUFFER)psvUser;
	buffer->result_len = tnprintf( buffer->buffer, buffer->len*sizeof(TEXTCHAR), WIDE("%s"), name );
}

int  GetMatchingFileName ( CTEXTSTR filemask, int flags, TEXTSTR pResult, int nResult )
{
	void *info = NULL;
	RESULT_BUFFER result_buf;
	result_buf.buffer = pResult;
	result_buf.len = nResult;
	result_buf.result_len = 0;
	// may need a while loop here...
	// but I'm just going to result the first matching anyhow.
	while( ScanFiles( NULL, filemask, &info, MatchFile, flags, (PTRSZVAL)&result_buf ) );
	return result_buf.result_len;
}

//---------------------------------------------------------------------------

#if defined( _WIN32 ) && !defined( __TURBOC__ )

#ifdef UNDER_CE
#define finddata_t WIN32_FIND_DATA
#define findfirst FindFirstFile
#define findnext  FindNextFile
#define findclose FindClose
#else
#  ifdef UNICODE
#define finddata_t _wfinddata_t
#define findfirst _wfindfirst
#define findnext  _wfindnext
#define findclose _findclose
#  else
#define finddata_t _finddata_t
#define findfirst _findfirst
#define findnext  _findnext
#define findclose _findclose
#  endif
#endif

#else


#endif

typedef struct myfinddata {
# ifdef _MSC_VER
#define HANDLECAST (HANDLE)
	intptr_t
# else
#define HANDLECAST
	int 
# endif
		handle;
# ifdef UNDER_CE
	WIN32_FIND_DATA fd;
# else
#  ifdef UNICODE
	struct _wfinddata_t fd;
#  else
   struct finddata_t fd;
#  endif
# endif
	struct find_cursor *cursor;
	INDEX scanning_interface_index;
	LOGICAL new_mount;
	LOGICAL single_mount;
	struct file_system_mounted_interface *scanning_mount;
	TEXTCHAR buffer[MAX_PATH_NAME];
	TEXTCHAR basename[MAX_PATH_NAME];
	TEXTCHAR findmask[MAX_PATH_NAME];
	struct myfinddata *current;
	struct myfinddata *prior;
	struct myfinddata **root_info;
} MFD, *PMFD;

#define findhandle(pInfo) ( ((PMFD)(*pInfo))->handle)
#define finddata(pInfo) ( &((PMFD)(*pInfo))->fd)
#define findbuffer(pInfo) ( ((PMFD)(*pInfo))->buffer)
#define findbasename(pInfo) ( ((PMFD)(*pInfo))->basename)
#define findmask(pInfo)     ( ((PMFD)(*pInfo))->findmask)
#define findinfo(pInfo)     (((PMFD)(*pInfo)))
#define findcursor(pInfo)     ( ((PMFD)(*pInfo))->cursor)

 int  ScanFilesEx ( CTEXTSTR base
           , CTEXTSTR mask
           , void **pInfo
           , void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
           , int flags 
           , PTRSZVAL psvUser 
		   , LOGICAL begin_sub_path 
		   , struct file_system_mounted_interface *mount
		   )
{
	PMFD pDataCurrent = (PMFD)(pInfo);
	PMFD pData = (PMFD)(*pInfo);
	int sendflags;
	int processed = 0;
#ifndef WIN32
	struct dirent *de;
#endif
	if( begin_sub_path )
	{
		pInfo = (void**)&(pDataCurrent->current);
	}
	else
		pDataCurrent = NULL;

	lprintf( "Search in %s for %s", base?base:"(NULL)", mask?mask:"(*)" );
	if( !*pInfo || begin_sub_path || ((PMFD)*pInfo)->new_mount )
	{
		TEXTCHAR findmask[256];
		pData = (PMFD)(*pInfo);
		if( !pData )
		{
			*pInfo = Allocate( sizeof( MFD ) );
			pData = (PMFD)(*pInfo);
			if( !( pData->scanning_mount = mount ) )
			{
				if( !winfile_local )
					SimpleRegisterAndCreateGlobal( winfile_local );

				//lprintf( "... %p", winfile_local );
				pData->single_mount = FALSE;
				pData->scanning_mount = (*winfile_local).mounted_file_systems;
			}
			else
				pData->single_mount = TRUE;

			if( !pData->scanning_mount )
			{
				Deallocate( PMFD, pData );
				return 0;
			}
			if( pData->scanning_mount->fsi )
			{
				//lprintf( "create cursor" );
				pData->cursor = pData->scanning_mount->fsi->find_create_cursor( pData->scanning_mount->psvInstance, base, mask );
			}
			else
			{
				//lprintf( "no cursor" );
				pData->cursor = NULL;
			}
		}
		else
		{
			if( pData->new_mount )
			{
				if( pData->scanning_mount->fsi )
				{
					//lprintf( "create cursor (new mount)" );
					pData->cursor = pData->scanning_mount->fsi->find_create_cursor( pData->scanning_mount->psvInstance, base, mask );
				}
				else
					pData->cursor = NULL;
			}
		}
		pData->new_mount = FALSE;
		pData->current = NULL;
		pData->prior = pDataCurrent;

		if( pDataCurrent )
		{
			pData->root_info = pDataCurrent->root_info;
			pInfo = (void**)pData->root_info;
		}
		else
		{
			pData->root_info = (struct myfinddata**)pInfo;
		}

		(*pData->root_info) = pData;
		if( base )
		{
			TEXTSTR tmp;
			StrCpyEx( findbasename(pInfo), tmp = ExpandPathEx( base, pData->scanning_mount->fsi ), MAX_PATH_NAME );
			Release( tmp );
			StrCpyEx( findmask(pInfo), mask, MAX_PATH_NAME );
		}
		else
		{
			CTEXTSTR p = pathrchr( mask );
			if( p )
			{
				StrCpyEx( findbasename(pInfo), mask, p - mask + 1 );
				StrCpyEx( findmask(pInfo), p + 1, MAX_PATH_NAME );
				//mask = p + 1;
			}
			else
			{
				StrCpyEx( findbasename(pInfo), WIDE("."), 2 );
				StrCpyEx( findmask(pInfo), mask, MAX_PATH_NAME );
			}
		}
#ifdef UNICODE
		snwprintf( findmask, sizeof(findmask), WIDE("%s/*"), findbasename(pInfo) );
#else
		snprintf( findmask, sizeof(findmask), WIDE("%s/*"), findbasename(pInfo) );
#endif
		if( pData->scanning_mount->fsi )
			if( pData->scanning_mount->fsi->find_first( findcursor(pInfo) ) )
				findhandle(pInfo) = 0;
			else
				findhandle(pInfo) = -1;
		else
		{
#if WIN32
			findhandle(pInfo) = findfirst( findmask, finddata(pInfo) );
#else
			lprintf( "opendir %s", findbasename(pInfo) );
			findhandle( pInfo ) = (int)opendir( findbasename(pInfo) );
			if( !findhandle(pInfo ) )
				findhandle(pInfo) = -1;
			else
				de = readdir( (DIR*)findhandle( pInfo ) );
#endif
		}
		if( findhandle(pInfo) == -1 )
		{
			PMFD prior = pData->prior;
			if( pData->scanning_mount->fsi )
				pData->scanning_mount->fsi->find_close( findcursor(pInfo) );
			else
			{
#ifdef WIN32
				findclose( findhandle(pInfo) );
#else
				// but it failed... so ... don't close
				//closedir( findhandle( pInfo ) );
#endif
			}
			pData->scanning_mount = NextThing( pData->scanning_mount );
			if( !pData->scanning_mount || pData->single_mount )
			{
				(*pData->root_info) = pData->prior;
				Release( pData );
				return prior?processed:0;
			}
			pData->new_mount = TRUE;
			return 1;
		}
	}
	else
	{
		int r;
getnext:
		if( pData->scanning_mount->fsi )
			r = !pData->scanning_mount->fsi->find_next( findcursor( pInfo ) );
		else
		{
#ifdef _WIN32
			r = findnext( findhandle(pInfo), finddata( pInfo ) );
#else
			de = readdir( (DIR*)findhandle( pInfo ) );
#endif
		}
		if( r )
		{
			PMFD prior = pData->prior;
			if( pData->scanning_mount->fsi )
				pData->scanning_mount->fsi->find_close( findcursor(pInfo) );
			else
			{
#ifdef WIN32
				findclose( findhandle(pInfo) );
#else
				closedir( (DIR*)findhandle(pInfo));
#endif
			}
			pData->scanning_mount = NextThing( pData->scanning_mount );
			if( !pData->scanning_mount || pData->single_mount )
			{
				(*pData->root_info) = pData->prior;
				Release( pData );
				if( prior )
					prior->current = NULL;
				if( !processed && !begin_sub_path )
				{
					//pInfo = (void**)&(prior->prior->current);
					pData = prior;
					if( pData )
						goto getnext;
				}
				return (*pInfo)?processed:0;
			}
			pData->new_mount = TRUE;
			return 1;
		}
	}
	if( pData->scanning_mount->fsi )
	{
		CTEXTSTR path = pData->scanning_mount->fsi->find_get_name( findcursor(pInfo) );
		//lprintf( "... %s", path );
		if( !StrCmp( WIDE("."), path ) ||
		    !StrCmp( WIDE(".."), path ) )
		goto getnext;
	}
	else
	{
#if WIN32 
		//lprintf( "... %s", finddata(pInfo)->name );
#  ifdef UNDER_CE
		if( !StrCmp( WIDE("."), finddata(pInfo)->cFileName ) ||
		    !StrCmp( WIDE(".."), finddata(pInfo)->cFileName ) )
#  else
		if( !StrCmp( WIDE("."), finddata(pInfo)->name ) ||
		    !StrCmp( WIDE(".."), finddata(pInfo)->name ) )
#  endif
#else
		if( !StrCmp( WIDE("."), de->d_name ) ||
		    !StrCmp( WIDE(".."), de->d_name ) )
#endif
			goto getnext;
	}
	if( !(flags & SFF_NAMEONLY) ) // if nameonly - have to rebuild the correct name.
	{
		if( pData->scanning_mount->fsi )
		{
			snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s/%s"), findbasename(pInfo), pData->scanning_mount->fsi->find_get_name( findcursor(pInfo) ) );
		}
		else
		{
#ifdef WIN32
#  ifdef UNDER_CE
			snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s/%s"), findbasename(pInfo), finddata(pInfo)->cFileName );
#  else
#    ifdef UNICODE
			snwprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s/%s"), findbasename(pInfo), finddata(pInfo)->name );
#    else
			snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s/%s"), findbasename(pInfo), finddata(pInfo)->name );
#    endif
#  endif
#else
			snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s/%s"), findbasename(pInfo), de->d_name );
#endif
		}
	}
	else
	{
		if( flags & SFF_SUBCURSE )
		{
			if( pData->scanning_mount->fsi )
			{
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s%s%s")
					  , pData->prior?pData->prior->buffer:WIDE( "" )
					  , pData->prior?WIDE( "/" ):WIDE( "" )
					, pData->scanning_mount->fsi->find_get_name( findcursor(pInfo) ) 
					);
			}
			else
			{
#if WIN32
#  ifdef UNDER_CE
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s%s%s")
						  , pData->prior?pData->prior->buffer:WIDE( "" )
						  , pData->prior?WIDE( "/" ):WIDE( "" )
						  , finddata(pInfo)->cFileName );
#  else
#    ifdef UNICODE
				snwprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s%s%s")
						  , pData->prior?pData->prior->buffer:WIDE( "" )
						  , pData->prior?WIDE( "/" ):WIDE( "" )
						  , finddata(pInfo)->name );
#    else
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s%s%s")
						  , pData->prior?pData->prior->buffer:WIDE( "" )
						  , pData->prior?WIDE( "/" ):WIDE( "" )
						  , finddata(pInfo)->name );
#    endif
#  endif
#else
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s%s%s")
					  , pData->prior?pData->prior->buffer:WIDE( "" )
					  , pData->prior?WIDE( "/" ):WIDE( "" )
					  , de->d_name );
#endif
			}
		}
		else
		{
			if( pData->scanning_mount->fsi )
			{
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s"), pData->scanning_mount->fsi->find_get_name( findcursor(pInfo) ) );
			}
			else
			{
#if WIN32
#  ifdef UNDER_CE
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s"), finddata(pInfo)->cFileName );
#  else
#    ifdef UNICODE
				snwprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s"), finddata(pInfo)->name );
#    else
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s"), finddata(pInfo)->name );
#    endif
#  endif
#else
				snprintf( pData->buffer, MAX_PATH_NAME, WIDE("%s"), de->d_name );
#endif
			}
		}
	}
	pData->buffer[MAX_PATH_NAME-1] = 0; // force nul termination...

	lprintf( "Check if %s is a directory...", pData->buffer );
	if( ( flags & (SFF_DIRECTORIES|SFF_SUBCURSE) )
#ifdef WIN32
#  ifdef UNDER_CE
		&& finddata(pInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
#  else
		&& finddata(pInfo)->attrib & _A_SUBDIR
#  endif
#else
		&& IsPath( pData->buffer )
#endif
	  )
	{
		if( flags & SFF_DIRECTORIES )
		{
			if( Process != NULL )
			{
				Process( psvUser, pData->buffer, SFF_DIRECTORY );
				processed = 1;
			}
			//return 1;
		}
		if( flags & SFF_SUBCURSE )
		{
			void *data = NULL;
			int ofs = 0;
			TEXTCHAR tmpbuf[MAX_PATH_NAME];
			if( flags & SFF_NAMEONLY )
			{
				// even in name only - need to have this full buffer for subcurse.
				if( pData->scanning_mount->fsi )
				{
					ofs = snprintf( tmpbuf, sizeof( tmpbuf ), WIDE( "%s/%s" ), findbasename(pInfo), pData->scanning_mount->fsi->find_get_name( findcursor(pInfo) ) );
				}
				else
				{
#ifdef UNDER_CE
				ofs = snprintf( tmpbuf, sizeof( tmpbuf ), WIDE( "%s/%s" ), findbasename(pInfo), finddata(pInfo)->cFileName );
#else
#  ifdef UNICODE
				ofs = snwprintf( tmpbuf, sizeof( tmpbuf ), WIDE( "%s/%s" ), findbasename(pInfo), finddata(pInfo)->name );
#  else
				ofs = snprintf( tmpbuf, sizeof( tmpbuf ), WIDE( "%s/%s" ), findbasename(pInfo), finddata(pInfo)->name );
#  endif
#endif
				}
				lprintf( "process sub..." );
				processed |= ScanFilesEx( tmpbuf, findmask( pInfo ), (POINTER*)pData, Process, flags, psvUser, TRUE, pData->scanning_mount );
			}
			else
			{
				lprintf( "process sub..." );
				processed |= ScanFilesEx( pData->buffer, findmask( pInfo ), (POINTER*)pData, Process, flags, psvUser, TRUE, pData->scanning_mount );
			}
		}
		if( !processed )
			goto getnext;
		return (*pInfo)?1:0;
	}
	if( ( sendflags = SFF_DIRECTORY, ( ( flags & SFF_DIRECTORIES )
#ifdef UNDER_CE
												 && ( finddata(pInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
#else
												 && ( finddata(pInfo)->attrib & _A_SUBDIR )
#endif
												) ) || ( sendflags = 0, CompareMask( findmask( pInfo )
#ifdef UNDER_CE
																							  , finddata(pInfo)->cFileName
#else
																							  , finddata(pInfo)->name
#endif
																								// yes this is silly - but it's correct...
																							  , (flags & SFF_IGNORECASE)?0:0 ) ) )
	{
		if( Process != NULL )
			Process( psvUser, pData->buffer, sendflags );
		return (*pInfo)?1:0;
	}
	return (*pInfo)?1:0;
}
 int  ScanFiles ( CTEXTSTR base
           , CTEXTSTR mask
           , void **pInfo
           , void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
           , int flags 
           , PTRSZVAL psvUser )
 {
	 return ScanFilesEx( base, mask, pInfo, Process, flags, psvUser, FALSE, NULL );
 }


//---------------------------------------------------------------------------

 void  ScanDrives ( void (CPROC*Process)(PTRSZVAL user, CTEXTSTR letter, int flags)
									, PTRSZVAL user )
{
#ifdef UNDER_CE
	Process( user, WIDE(""), SFF_DRIVE );
#else
	_32 drives;
	int i;
	drives = GetLogicalDrives();
	for( i = 0; i < 26; i++ )
	{
		TEXTCHAR name[2];
		name[1] = 0;
		if( drives & ( 1 << i ) ) 
		{
			name[0] = 'A' + i;
			if( Process )
				Process( user, name, SFF_DRIVE );
		}
	}
#endif

}
#if 1
#else

//---------------------------------------------------------------------------
typedef struct myfinddata {
	int handle;
	DIR *dir;
	//struct finddata_t fd;
	char buffer[MAX_PATH_NAME];
	char basename[MAX_PATH_NAME];
	struct myfinddata *current;
	//struct myfinddata *prior;
} MFD, *PMFD;

#define finddir(pInfo) ( ((PMFD)(*pInfo))->dir)
#define findbasename(pInfo) ( ((PMFD)(*pInfo))->basename)

//---------------------------------------------------------------------------
int  ScanFilesEx ( CTEXTSTR base
					, CTEXTSTR mask
					, void **pInfo
					, void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
					, int flags
					, PTRSZVAL psvUser
		   , LOGICAL begin_sub_path 
		   , struct file_system_mounted_interface *mount
                  )
{
	//DIR *dir;
	char name[256];
	struct dirent *de;
	char *_base = CStrDup( base );
	char *_mask = CStrDup( mask );
	//char basename[256];
	// need to dup base - it might be in read-only space.
	if( !*pInfo || begin_sub_path || ((PMFD)*pInfo)->new_mount )
	{
		if( !(*pInfo) )
			(*pInfo) = New( MFD );
		if( base )
			strcpy( findbasename(pInfo), _base );
		else
		{
			CTEXTSTR p = pathrchr( mask );
			if( p )
			{
				strncpy( findbasename(pInfo), _mask, p - mask );
				findbasename(pInfo)[p-mask] = 0;
				mask = p + 1;
			}
			else
			{
				strcpy( findbasename(pInfo), "." );
			}
		}
		//lprintf( "Open directory for scanning... %s %s", base, mask );
		finddir( pInfo ) = opendir( findbasename(pInfo) );
		//lprintf( "result of opendir on %s = %d", findbasename(pInfo), finddir( pInfo ) );
		//*pInfo = (void*)dir;
	}
	else
	{
		//dir = (DIR*)*pInfo;
	}
	{
		TEXTCHAR *p;
		TEXTCHAR *tmppath = DupCStr( findbasename(pInfo) );
		// result from pathrchr is within findbasename(pInfo)
		// it's result si technically a CTEXTSTR since
		// that is what is passed to pathrchr
		if( ( p = (TEXTCHAR*)pathrchr( tmppath ) ) )
		{
			if( !p[1] )
			{
				p[0] = 0;
			}
		}
		Release( tmppath );
	}
	if( finddir( pInfo ) )
		while( ( de = readdir( finddir( pInfo ) ) ) )
		{
			char *de_d_name = de->d_name;
			TEXTCHAR *_de_d_name = DupCStr( de_d_name );
			struct stat filestat;
			//lprintf( WIDE("Check: %s"), de_d_name );
			// should figure a way to check file vs mask...
			if( !strcmp( ".", de_d_name ) ||
				!strcmp( "..", de_d_name ) )
				continue;
			sprintf( name, "%s/%s", findbasename(pInfo), de_d_name );
#ifdef BCC32
			if( stat( name, &filestat ) == -1 )
				continue;
#else
			if( lstat( name, &filestat ) == -1 )
			{
				//lprintf( WIDE("We got problems with stat! (%s)"), name );
				continue;
			}
			if( S_ISLNK( filestat.st_mode ) )
			{
				//lprintf( WIDE("A link: %s"), name );
				// not following links...
				continue;
			}
#endif
			if( S_ISDIR(filestat.st_mode) )
			{
				if( flags & SFF_SUBCURSE )
				{
					TEXTCHAR *tmpresult;
					void *data = NULL;
					if( S_ISBLK( filestat.st_mode ) ||
						S_ISCHR( filestat.st_mode ) )
						continue;
					tmpresult = NULL;
		   			if( flags & SFF_DIRECTORIES ) 
						if( Process )
							Process( psvUser
							       , tmpresult = DupCStr( (flags & SFF_NAMEONLY)?de_d_name:name )
									 , SFF_DIRECTORY );
					if( tmpresult )
						Release( tmpresult );
					while( ScanFiles( DupCStr( name ), mask, &data, Process, flags, psvUser ) );
				}
				continue;
			}
			if( CompareMask( mask, _de_d_name, (flags & SFF_IGNORECASE)?0:1 ) )
			{
				TEXTCHAR *tmpresult;
				tmpresult = NULL;
				if( Process )
					Process( psvUser
					       , tmpresult = DupCStr( (flags & SFF_NAMEONLY)?de_d_name:name )
							 , 0 );
				if( tmpresult )
					Release( tmpresult );
				return 1;
			}
		}
		Release( _base );
		Release( _mask );
		if( finddir( pInfo ) )
			closedir( finddir( pInfo ) );

	{
		Release( *pInfo );
		*pInfo = NULL;
	}
	return !((*pInfo)==NULL);
}

 int  ScanFiles ( CTEXTSTR base
           , CTEXTSTR mask
           , void **pInfo
           , void CPROC Process( PTRSZVAL psvUser, CTEXTSTR name, int flags )
           , int flags 
           , PTRSZVAL psvUser )
 {
	 return ScanFilesEx( base, mask, pInfo, Process, flags, psvUser, FALSE, NULL );
 }

//---------------------------------------------------------------------------


 void  ScanDrives ( void(*Process)(PTRSZVAL user, CTEXTSTR letter, int flags)
                                 , PTRSZVAL psv )
{

}
//---------------------------------------------------------------------------
#endif

FILESYS_NAMESPACE_END
