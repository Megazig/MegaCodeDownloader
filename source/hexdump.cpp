#include "hexdump.h"

char ascii( char character )
{
  if ( character < 0x20 ) return '.';
  if ( character > 0x7E ) return '.';
  return character;
}

void hexdump( void * d , int len )
{
	u8 *data;
	int i, off;
	data = (u8*)d;
	for ( off = 0 ; off < len ; off += 16 )
	{
		dbgprintf( "%08x  " , (u32)(off + data) );
		for( i = 0 ; i < 16 ; i++ )
			if(( i + off ) >= len ) printf("  ");
			else dbgprintf( "%02x " , data[off+i]);

		dbgprintf(" ");
		for(i=0; i<16; i++)
			if((i+off)>=len) printf(" ");
			else dbgprintf("%c", ascii(data[off+i]));
		dbgprintf("\n");
	}
}

