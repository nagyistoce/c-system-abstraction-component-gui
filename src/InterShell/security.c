
#include <sack_types.h>
#include <deadstart.h>
#include <pssql.h>
#include "intershell_local.h"
#include "intershell_registry.h"
#include "intershell_security.h"
#include "menu_real_button.h"
#include "resource.h"
#include "fonts.h"


/*
 * Menu Buttons extended security attributes.
 *
 *  When a button is pressed the flag bSecurity is check, and if set,
 *  then a security context by permission_context table index.
 *  The program_id will be set for
 *
 */



/* result with either INVALID_INDEX - permission denied
 or 0 - no token created
 or some other value that is passed to CloseSecurityContext when the button press is complete
 */
PTRSZVAL CreateSecurityContext( PTRSZVAL button )
{
      // add additional security plugin stuff...
      if( button )
		{
         PTRSZVAL psv_context;
			CTEXTSTR name;
			PCLASSROOT data = NULL;
			for( name = GetFirstRegisteredName( TASK_PREFIX WIDE( "/common/Test Security" ), &data );
				 name;
				  name = GetNextRegisteredName( &data ) )
			{
				PTRSZVAL (CPROC*f)(PTRSZVAL);
				//snprintf( rootname, sizeof( rootname ), TASK_PREFIX WIDE( "/common/save common/%s" ), name );
				f = GetRegisteredProcedure2( (CTEXTSTR)data, PTRSZVAL, name, (PTRSZVAL) );
				if( f )
				{
					psv_context = f( button );
					if( psv_context )
					{
						/* should probably record on per-plugin basis this value...... */
						return psv_context;
					}
				}
			}
		}
   return 0;
}

void CloseSecurityContext( PTRSZVAL button, PTRSZVAL psvSecurity )
{
      if( button )
		{
			//PTRSZVAL psv_context;
			CTEXTSTR name;
			PCLASSROOT data = NULL;
			for( name = GetFirstRegisteredName( TASK_PREFIX WIDE( "/common/Close Security" ), &data );
				 name;
				  name = GetNextRegisteredName( &data ) )
			{
				void (CPROC*f)(PTRSZVAL,PTRSZVAL);
				//snprintf( rootname, sizeof( rootname ), TASK_PREFIX WIDE( "/common/save common/%s" ), name );
				f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PTRSZVAL,PTRSZVAL) );
				if( f )
				{
					f( button, psvSecurity );
				}
			}
		}
}

void AddSecurityContextToken( PTRSZVAL button, CTEXTSTR module, CTEXTSTR token )
{
	void (CPROC*f)(PTRSZVAL,CTEXTSTR);
	f = GetRegisteredProcedure2( (CTEXTSTR)WIDE( "intershell/common/Add Security Token" ), void, module, (PTRSZVAL,CTEXTSTR) );
	if( f )
		f( button, token );
}

void GetSecurityContextTokens( PTRSZVAL button, CTEXTSTR module, PLIST *ppList )
{
	void (CPROC*f)(PTRSZVAL,PLIST*);
	f = GetRegisteredProcedure2( (CTEXTSTR)WIDE( "intershell/common/Get Security Tokens" ), void, module, (PTRSZVAL,PLIST*) );
	if( f )
		f( button, ppList );
}

void CPROC SelectEditSecurity( PTRSZVAL psv, PSI_CONTROL listbox, PLISTITEM pli )
{
	{
		CTEXTSTR name;
		//TEXTCHAR invoke[256];
		void (CPROC*f)(PTRSZVAL);
		name = (CTEXTSTR)GetItemData( pli );
		f = GetRegisteredProcedure2( (CTEXTSTR)WIDE( "intershell/common/Edit Security" ), void, name, (PTRSZVAL) );
		if( f )
			f( (PTRSZVAL)psv );
	}
}


void CPROC EditSecurity( PTRSZVAL psv, PSI_CONTROL button )
{
	PSI_CONTROL pc_list;
   	PLISTITEM pli;
	pc_list = GetNearControl( button, LISTBOX_SECURITY_MODULE );
	pli = GetSelectedItem( pc_list );
	if( pli )
	{
      		SelectEditSecurity( psv, NULL, pli );
	}
	else
	{
      		SimpleMessageBox( button, WIDE( "No selected security module" ), WIDE( "No Selection" ) );
	}
}


void CPROC EditSecurityNoList( PTRSZVAL psv, PSI_CONTROL button )
{
	SimpleMessageBox( button, WIDE( "No listbox to select security module" ), WIDE( "NO SECURITY LIST" ) );

}

void SetupSecurityEdit( PSI_CONTROL frame, PTRSZVAL object_to_secure )
		{
			PSI_CONTROL listbox;
         //lprintf( "Setting up security edit for %p", object_to_secure );
			listbox = GetControl( frame, LISTBOX_SECURITY_MODULE );
			if( listbox )
			{
				INDEX idx;
				CTEXTSTR name;
				LIST_FORALL( g.security_property_names, idx, CTEXTSTR, name )
				{
					SetItemData( AddListItem( listbox, name ), (PTRSZVAL)name );
				}
				SetDoubleClickHandler( listbox, SelectEditSecurity, object_to_secure );
				SetButtonPushMethod( GetControl( frame, EDIT_SECURITY ), EditSecurity, object_to_secure );
			}
			else
			{
				SetButtonPushMethod( GetControl( frame, EDIT_SECURITY ), EditSecurityNoList, 0 );
			}
		}


OnGlobalPropertyEdit( WIDE( "Set Edit Permissions" ) )( PSI_CONTROL parent )
{
	PSI_CONTROL pc = LoadXMLFrameOver( parent, WIDE("SetEditPermissions.isFrame") );
	if( pc )
	{
		int okay = 0;
		int done = 0;
		SetupSecurityEdit( pc, (PTRSZVAL)parent );
		SetCommonButtons( pc, &done, &okay );
		DisplayFrameOver( pc, parent );
		CommonWait( pc );
		DestroyFrame( &pc );
	}
}

void InterShell_ReloadSecurityInformation( PCONFIG_HANDLER pch )
{
	CTEXTSTR name;
	PCLASSROOT data = NULL;
	//lprintf( WIDE( "Gave control a chance to register additional security methods on current config..." ) );
	for( name = GetFirstRegisteredName( TASK_PREFIX WIDE( "/common/Load Security" ), &data );
		 name;
		  name = GetNextRegisteredName( &data ) )
	{
		void (CPROC*f)(PCONFIG_HANDLER);
		//snprintf( rootname, sizeof( rootname ), TASK_PREFIX WIDE( "/common/save common/%s" ), name );
		f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (PCONFIG_HANDLER) );
		if( f )
			f( pch );
	}
}

void InterShell_SaveSecurityInformation( FILE *file, PTRSZVAL psv )
{
	// add additional security plugin stuff...
	CTEXTSTR name;
	PCLASSROOT data = NULL;
   //lprintf( "Save existing config for %p", psv );
	for( name = GetFirstRegisteredName( TASK_PREFIX WIDE( "/common/Save Security" ), &data );
		 name;
		  name = GetNextRegisteredName( &data ) )
	{
		void (CPROC*f)(FILE*,PTRSZVAL);
		//snprintf( rootname, sizeof( rootname ), TASK_PREFIX "/common/save common/%s", name );
		f = GetRegisteredProcedure2( (CTEXTSTR)data, void, name, (FILE*,PTRSZVAL) );
		if( f )
			f( file, psv );
	}
}



OnSaveCommon( WIDE( "@10 EditSecurity" ) )( FILE *file )
{
	fprintf( file, WIDE( "Begin Edit Permissions\n" ) );
	InterShell_SaveSecurityInformation( file, (PTRSZVAL)g.single_frame );
}

static PTRSZVAL CPROC BeginGlobalEditPerms( PTRSZVAL psv, arg_list args )
{
	lprintf( "Setting psv to single_frame %p, adding security plugin rules", g.single_frame );
	InterShell_ReloadSecurityInformation( InterShell_GetCurrentConfigHandler() );
   // change/set what the psv is for global paramters.
   return (PTRSZVAL)g.single_frame;
}

OnLoadCommon( WIDE( "@10 EditSecurity" ) )( PCONFIG_HANDLER pch )
{
	AddConfigurationMethod( pch, WIDE( "Begin Edit Permissions" ), BeginGlobalEditPerms );
}

