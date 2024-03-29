#ifndef BRAIN_TYPES_DEFINED
#define BRAIN_TYPES_DEFINED

#include <stdhdrs.h> // ptrszval
#include "global.h"

// note this value reflects the type defined by VAL_NATIVE

typedef float NATIVE, *PNATIVE; // precision which the brain 'thinks' in
#define VAL_NATIVE VAL_FLOAT
#define NATIVE_FORMAT "%g"
#pragma pack(1)
enum type {
     VAL_NULL // 0 - never set to a value.
	, VAL_BOOL                 , VAL_b               = VAL_BOOL
   , VAL_FLOAT                , VAL_f               = VAL_FLOAT
   , VAL_DOUBLE					, VAL_d     			 = VAL_DOUBLE
   , VAL_CHAR						, VAL_c   				 = VAL_CHAR
   , VAL_SHORT						, VAL_s    				 = VAL_SHORT
   , VAL_LONG						, VAL_l   				 = VAL_LONG
   , VAL_LONGLONG					, VAL_ll      			 = VAL_LONGLONG
   , VAL_UCHAR						, VAL_uc   				 = VAL_UCHAR
   , VAL_USHORT					, VAL_us    			 = VAL_USHORT
   , VAL_ULONG						, VAL_ul   				 = VAL_ULONG
   , VAL_ULONGLONG				, VAL_ull      		 = VAL_ULONGLONG
	, VAL_PTRBOOL              , VAL_pb              = VAL_PTRBOOL
   , VAL_PTRFLOAT					, VAL_pf      			 = VAL_PTRFLOAT
   , VAL_PTRDOUBLE				, VAL_pd       		 = VAL_PTRDOUBLE
   , VAL_PTRCHAR					, VAL_pc     			 = VAL_PTRCHAR
   , VAL_PTRSHORT					, VAL_ps      			 = VAL_PTRSHORT
   , VAL_PTRLONG					, VAL_pl					 = VAL_PTRLONG
   , VAL_PTRLONGLONG				, VAL_pll        		 = VAL_PTRLONGLONG
   , VAL_PTRUCHAR					, VAL_puc     			 = VAL_PTRUCHAR
   , VAL_PTRUSHORT				, VAL_pus      		 = VAL_PTRUSHORT
   , VAL_PTRULONG					, VAL_pul     			 = VAL_PTRULONG
   , VAL_PTRULONGLONG			, VAL_pull        	 = VAL_PTRULONGLONG
   , VAL_EXTERNINPUT          , VAL_Input           = VAL_EXTERNINPUT
   , VAL_EXTERNOUTPUT         , VAL_Output          = VAL_EXTERNOUTPUT
   , VAL_EXTERNTHRUPUT
			 , VAL_EXTERNTRIGGER
          , VAL_PANYVALUE
	, VAL_UNDEFINED
};


typedef NATIVE (*InputFunction)( PTRSZVAL );
typedef void   (*OutputFunction)( PTRSZVAL, NATIVE );
typedef NATIVE (*ThruputFunction) ( PTRSZVAL, NATIVE );
typedef void   (*TriggerFunction) ( PTRSZVAL );

struct value {
   enum type type;
   union {
		bool                   b;
      float                  f;
      double                 d;
      S_8                    c;
		short                  s;
		S_32                   l;
      S_64                  ll;
      _8                    uc;
      unsigned short        us;
      _32                   ul;
		_64                  ull;
      void                 *pv;
      bool                 *pb;
      float                *pf;
      double               *pd;
      S_8                  *pc;
      short                *ps;
		S_32                 *pl;
      S_64                *pll;
      _8                  *puc;
      unsigned short      *pus;
      _32                 *pul;
      _64                *pull;
		InputFunction       Input;
		OutputFunction      Output;
		ThruputFunction     Thruput;
		TriggerFunction     Trigger;
      struct value       *pany;
	} data;
#define VALUE_CONSTRUCTOR( val_type, name ) value(val_type *name){ data.name=name; value::type=VAL_##name; }
#define FVALUE_CONSTRUCTOR( val_type, name ) value(val_type name, PTRSZVAL psv){ data.name=name; value::type=VAL_##name; value::aux=psv; }
	VALUE_CONSTRUCTOR( bool            ,   pb );
	VALUE_CONSTRUCTOR( float           ,   pf );
	VALUE_CONSTRUCTOR( double          ,   pd );
	VALUE_CONSTRUCTOR( S_8             ,   pc );
	VALUE_CONSTRUCTOR( S_16            ,   ps );
	VALUE_CONSTRUCTOR( S_32            ,   pl );
	VALUE_CONSTRUCTOR( S_64            ,  pll );
	VALUE_CONSTRUCTOR( _8              ,  puc );
	VALUE_CONSTRUCTOR( _16             ,  pus );
	VALUE_CONSTRUCTOR( _32             ,  pul );
	VALUE_CONSTRUCTOR( _64             , pull );
	FVALUE_CONSTRUCTOR( InputFunction   , Input );
	FVALUE_CONSTRUCTOR( OutputFunction  , Output );
		//InputFunction       Input;
		//OutputFunction      Output;
      //ThruputFunction     Thruput;
		//TriggerFunction     Trigger;
      //struct value       *pany;
	PTRSZVAL aux; // value passed to input/output function
	int null( void )
	{ return( type == VAL_NULL ); }
   ;
	IMPORT NATIVE get();
	IMPORT int get_int();
	IMPORT void set( NATIVE native );
	IMPORT void set( enum type type, ... );
	//void set( struct value value );
	IMPORT void set( struct value &value );
	IMPORT void set( struct value *value );
	value()	{ set(VAL_UNDEFINED); }
	IMPORT value( enum type settype, ... );
	IMPORT value( struct value *clonevalue );
	//IMPORT _32 save(FILE*);
	//IMPORT _32 load(FILE*);
};
#pragma pack()
typedef struct value ANYVALUE, *PANYVALUE;

inline NATIVE GetNative( NATIVE n ) { return n; }
inline NATIVE GetNative( PNATIVE n ) { if(n) return *n; else return 0; }
inline NATIVE GetNative( ANYVALUE av ) { return av.get(); }
inline NATIVE GetNative( PANYVALUE av ) { return av->get(); }


#endif
