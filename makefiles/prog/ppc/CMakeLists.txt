
PROJECT( Panthers_pre_cpp )

set( MYSOURCES 
	define.c 
        links.c 
        text.c 
        input.c 
        cppmain.c 
        mem.c
	fileio.c 
        expr.c )

add_executable( ppc ${MYSOURCES} )

