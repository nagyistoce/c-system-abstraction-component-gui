#define FIX_RELEASE_COM_COLLISION
#define MAKE_RCOORD_SINGLE
#define USE_IMAGE_INTERFACE l.pii
#define USE_IMAGE_3D_INTERFACE l.pi3i
#define USE_RENDER3D_INTERFACE l.pr3i

#define DEFINES_DEKWARE_INTERFACE
#define PLUGIN_MODULE

#include <plugin.h>
#include <vectlib.h>
#include <virtuality.h>
#include <render3d.h>
#include <image3d.h>

#include <brain.hpp>
#include <brainshell.hpp>


static struct local_virtuality_interface_tag
{
   INDEX extension;
   PIMAGE_INTERFACE pii;
   PIMAGE_3D_INTERFACE pi3i;
   PRENDER3D_INTERFACE pr3i;
   PTRANSFORM transform;
   PLIST objects;
   POBJECT root_object;
} l;

struct virtuality_object
{
	//VECTOR o;// objects actually origins already
	Image label;  // objects actually have labels already
	POBJECT object;
	PBRAIN brain;
	PBRAINBOARD brain_board;
	VECTOR speed;
	VECTOR rotation_speed;
};

PRELOAD( Init )
{
	l.pr3i = GetRender3dInterface();
	l.pii = GetImageInterface();
	l.pi3i = GetImage3dInterface();

	l.root_object = Virtuality_MakeCube( 1000 );

}

static Image tmp ;
static PTRSZVAL OnInit3d( WIDE( "Virtuality interface" ) )( PMatrix projection, PTRANSFORM camera, RCOORD *identity_depth, RCOORD *aspect )
{
	l.transform = camera;
	{
		//Image 
		tmp = LoadImageFile( "%resources%/images/AN00236511_001_l.jpg" );
		SetImageTransformRelation( tmp, IMAGE_TRANSFORM_RELATIVE_CENTER, NULL );
		SetObjectColor( l.root_object, BASE_COLOR_BLUE );
		SetRootObject( l.root_object );

		POBJECT floor_plane = CreateObject();
		PFACET floor_plane_facet = AddPlane( floor_plane, _0, _Y, 0 );
		floor_plane_facet->color = BASE_COLOR_BROWN;
		floor_plane_facet->image = tmp;
		PutIn( floor_plane, l.root_object );

		PutOn( floor_plane = Virtuality_MakeCube( 10 ), l.root_object );
		floor_plane->color = BASE_COLOR_NICE_ORANGE;
	}
	return 1;
}

static void OnFirstDraw3d( WIDE( "Terrain View" ) )( PTRSZVAL psvInit )
{
}

static LOGICAL OnUpdate3d( WIDE( "Virtuality interface" ) )( PTRANSFORM origin )
{
	l.transform = origin;
	
	INDEX idx;
	struct virtuality_object *vobj;
	LIST_FORALL( l.objects, idx, struct virtuality_object *, vobj )
	{
		VECTOR tmp;
		scale( tmp, vobj->speed, 1/256.0 );
		SetSpeed( vobj->object->Ti, tmp );
		scale( tmp, vobj->rotation_speed, 1/256.0 );
		SetRotation( vobj->object->Ti, tmp );
	}
	return TRUE;
}

static void DrawLabel( struct virtuality_object *vobj )
{
	//btTransform trans;
	//patch->fallRigidBody->getMotionState()->getWorldTransform( trans );
	//btVector3 r = trans.getOrigin();

	{
		PTRANSFORM t = GetImageTransformation( vobj->label );
		_POINT p;
		_POINT p2;
		p[0] = 0;
		p[1] = 10/*PLANET_RADIUS*/ * 1.4f;
		p[2] = 0;
		//ApplyRotation( l.transform, p2, p );
		PCVECTOR o = GetOrigin( vobj->object->Ti );
		Translate( t, o[0], o[1], o[2] );		
		Render3dImage( vobj->label, TRUE );
	}
}


static void OnDraw3d( WIDE( "Virtuality interface" ) )( PTRSZVAL psvInit )
{
	INDEX idx;
	struct virtuality_object *vobj;
	LIST_FORALL( l.objects, idx, struct virtuality_object *, vobj )
	{
		DrawLabel( vobj );
	}
   // virtuality doesn't have its own hooks...

}

static void UpdateLabel( struct virtuality_object *vobj, CTEXTSTR text )
{
	_32 w, h;
	GetStringSize( text, &w, &h );
	ResizeImage( vobj->label, w, h );
	SetImageTransformRelation( vobj->label, IMAGE_TRANSFORM_RELATIVE_CENTER, NULL );
	ClearImage( vobj->label );
	PutString( vobj->label, 0, 0, BASE_COLOR_WHITE, 0, text );
}
static int GetVector( VECTOR pos, PSENTIENT ps, PTEXT *parameters, LOGICAL log_error )
{
	PTEXT original = parameters?parameters[0]:NULL;
	S_64 iNumber;
	double fNumber;
	int bInt;
	PTEXT pResult;
	PTEXT temp;
	int n;
	for( n = 0; n < 3; n++ )
	{
		temp = GetParam( ps, parameters ); // does variable substitution etc...
		if( !temp )
		{
			if( log_error )
				S_MSG( ps, "vector part %d is missing", n + 1 );
			parameters[0] = original;
			return 0;
		}

		if( IsSegAnyNumber( &temp, &fNumber, &iNumber, &bInt ) )
		{
			if( bInt )
				pos[n] = (RCOORD)iNumber;
			else
				pos[n] = (RCOORD)fNumber;
		}
		else
		{
			if( log_error )
				S_MSG( ps, "vector part %d is not a number", n + 1 );
			parameters[0] = original;
			return 0;
		}
		log_error = 1;
	}
	return 1;
}


static int OnCreateObject( WIDE("Point Label"), WIDE( "This is a point in space that has text") )(PSENTIENT ps,PENTITY pe_created,PTEXT parameters)
{
	if( !l.extension )
		l.extension = RegisterExtension( WIDE( "Point Label" ) );
	{
		struct virtuality_object *vobj = New( struct virtuality_object );
		SetLink( &pe_created->pPlugin, l.extension, vobj );
	}
	{
		struct virtuality_object *vobj = (struct virtuality_object *)GetLink( &pe_created->pPlugin, l.extension );
		vobj->label = MakeImageFile( 0, 0 );
		UpdateLabel( vobj, "Label" );

		vobj->brain = new BRAIN();
		PBRAIN_STEM pbs = new BRAIN_STEM( WIDE("Object Motion") );
		vobj->brain->AddBrainStem( pbs );

		pbs->AddOutput( new value(&vobj->speed[vForward]), WIDE("Forward -Backwards") );
		pbs->AddOutput( new value(&vobj->speed[vRight]), WIDE("Right -Left") );
		pbs->AddOutput( new value(&vobj->speed[vUp]), WIDE("Up -Down") );

		pbs = new BRAIN_STEM( WIDE("Object Rotation") );
		vobj->brain->AddBrainStem( pbs );

		pbs->AddOutput( new value(&vobj->rotation_speed[vForward]), WIDE("around Forward axis") );
		pbs->AddOutput( new value(&vobj->rotation_speed[vRight]), WIDE("around Right axis") );
		pbs->AddOutput( new value(&vobj->rotation_speed[vUp]), WIDE("around Up axis") );

		vobj->brain_board = CreateBrainBoard( vobj->brain );

		AddLink( &l.objects, vobj );


		PutIn( vobj->object = Virtuality_MakeCube( 10 ), l.root_object );
		CreateTransformMotionEx( vobj->object->Ti, 0 );
		vobj->object->color = BASE_COLOR_DARKBLUE;
		{
			VECTOR pos;
			if( GetVector( pos, ps, &parameters, 0 ) )
				TranslateV( vobj->object->Ti, pos );
		}

	}
	return 0;
}

static PTEXT ObjectVolatileVariableSet( WIDE("Point Label"), WIDE("text"), WIDE( "Set label text") )(PENTITY pe, PTEXT value )
{
	struct virtuality_object *vobj = (struct virtuality_object *)GetLink( &pe->pPlugin, l.extension );
	PTEXT text = BuildLine( value );
	
	UpdateLabel( vobj, GetText( text ) );
	LineRelease( text );
	return NULL;
}

static PTEXT ObjectVolatileVariableGet( WIDE("Point Label"), WIDE("position"), WIDE( "get current position") )(PENTITY pe, PTEXT *prior )
{
	struct virtuality_object *vobj = (struct virtuality_object *)GetLink( &pe->pPlugin, l.extension );
	PVARTEXT pvt = VarTextCreate();
	PTEXT result;
		PCVECTOR o = GetOrigin( vobj->object->Ti );
	vtprintf( pvt, "%g %g %g", o[0], o[1], o[2] );
	result = VarTextGet( pvt );
	VarTextDestroy( &pvt );
	return result;
}

static PTEXT ObjectVolatileVariableSet( WIDE("Point Label"), WIDE("position"), WIDE( "get current position") )(PENTITY pe, PTEXT value )
{
	struct virtuality_object *vobj = (struct virtuality_object *)GetLink( &pe->pPlugin, l.extension );
	PTEXT line = BuildLine( value );
	float vals[3];
	if( sscanf( GetText( line ), "%g %g %g", vals+0, vals+1, vals+2 ) == 3 )
	{
		// float precision may mismatch, so pass as discrete values...
		Translate( vobj->object->Ti,vals[0],vals[1], vals[2] );
	}
	LineRelease( line );
	return NULL;
}

static int ObjectMethod( WIDE("Point Label"), WIDE("move"), WIDE( "Friendly description") )(PSENTIENT ps, PENTITY pe_object, PTEXT parameters)
{
	VECTOR pos;
	struct virtuality_object *vobj = (struct virtuality_object *)GetLink( &pe_object->pPlugin, l.extension );
	if( vobj )
	{
		if( GetVector( pos, ps, &parameters, 1 ) )
			TranslateV( vobj->object->Ti, pos );
	}
   return 0;
}
