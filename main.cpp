
/*
Daniel Shanks' Baby step-Giant step implementation.
For more information, see http://en.wikipedia.org/wiki/Baby-step_giant-step
*/

#include <NTL/ZZ.h>

/* x .. sqrt(2^40) = 2^20 = 1048576 */
#define B 1048576
#define TABLE_SIZE  116777261

NTL_CLIENT

typedef struct hashlist
{
    int value;
    hashlist *p_coll;
    hashlist *p_next;
} HASHLIST;

typedef struct hashtable
{
    int size;
    // pointer to a pointer to a linked list
    HASHLIST **table;
} HASHTABLE;

HASHTABLE *htCreate( size_t size )
{
    HASHTABLE *fresh_table;

    if( (fresh_table = (HASHTABLE*)malloc(sizeof(HASHTABLE))) == NULL )
    {
	   return NULL;
    }

    if( (fresh_table->table = (HASHLIST**)malloc(sizeof(HASHLIST*)*size)) == NULL )
    {
	   return NULL;
    }

    int i;
    for( i = 0; i < size; i++ ) {
	   fresh_table->table[i] = NULL;
    }

    fresh_table->size = size;

    return fresh_table;
}

int htHash( HASHTABLE *t, ZZ value )
{
    unsigned long int result = 0;
    /*
    discrete logarithm results in pseudo random number
    so I can just apply the modulus table size
    */
    result = to_int(MulMod(value, 1, to_ZZ(t->size)));

    return result;
}

HASHLIST *htNewExponent( int value )
{
    HASHLIST *newList;

    if( (newList = (HASHLIST*)malloc(sizeof(HASHLIST))) == NULL )
    {
	   return NULL;
    }
    
    if( ( newList->value = value ) == NULL ) {
	   return NULL;
    }
    
    newList->p_coll = NULL;
    newList->p_next = NULL;
    
    return newList;
}

void htInsert( HASHTABLE *t, ZZ key, int value )
{
    int index = 0;
    HASHLIST *newExponent;
    HASHLIST *next ;
    HASHLIST *prev;

    index = htHash(t, key);
    next = t->table[index];
    
    /* collision */
    if( next != NULL /*&& next->value != NULL*/ )
    {
	   
	   /* walk to the end */
	   while( next->p_coll != NULL )
	   {
		  printf("walking ");

		  prev = next;
		  next = next->p_coll;
	   }
	   
	   newExponent = htNewExponent(value);
	   next->p_coll = newExponent;
    }
    else
    {
	   newExponent = htNewExponent(value);

	   /* start */
	   if( next == t->table[ index ] )
	   {
		  newExponent->p_next = next;
		  t->table[index] = newExponent;
        
	   }
	   /* end */
	   else if ( next == NULL )
	   {
		  prev->p_next = newExponent;	  
	   }
	   /* walking */
	   else
	   {
		  newExponent->p_next = next;
		  prev->p_next = newExponent;
	   }
    }
}

/*
find our exponents
*/
int findExponents( HASHTABLE *t, ZZ key, int x0, ZZ g, ZZ h, ZZ p )
{
    ZZ x;
    int index = 0;
    HASHLIST *pair;

    index = htHash(t, key);
    pair = t->table[index];

    /* shit happened */
    if( pair == NULL ) {
	   return NULL;
    }
    else
    {
	   x = (to_ZZ(pair->value) * B) + to_ZZ(x0);
	   
	   if( PowerMod(g, x, p) == h )
	   {
		  //cout << PowerMod(g, x, p) << endl;
		  printf("%d * B + %d = x\n", pair->value, x0);
		  return 1;
	   }

	   // walk collisions
	   while( pair->p_coll != NULL )
	   {
		  x = (to_ZZ(pair->p_coll->value) * B) + to_ZZ(x0);

		  if( PowerMod( g, x, p ) == h )
		  {
			 //cout << PowerMod(g, x, p) << endl;
			 printf("%d * B + %d = x\n", pair->p_coll->value, x0);
			 return 1;
		  }
		  
		  pair = pair->p_coll;
	   }
	   
	   return pair->value;
    }
}

int main()
{
    HASHTABLE *tabulka = htCreate( TABLE_SIZE );

    if( tabulka == NULL )
    {
	   return -1;
    }
    
    /* time */
    double t;
    
    /*
    g = h^x mod p, find x!
    */
    ZZ p = to_ZZ("13407807929942597099574024998205846127479365820592393377723561443721764030073546976801874298166903427690031858186486050853753882811946569946433649006084171");
    ZZ g = to_ZZ("11717829880366207009516117596335367088558084999998952205599979459063929499736583746670572176471460312928594829675428279466566527115212748467589894601965568");
    ZZ h = to_ZZ("3239475104050450443565264378728065788649097520952449527834792452971981976143292558073856937958553180532878928001494706097394108577585732452307673444020333");

    int tbl;
    ZZ x;
    ZZ hgx1;
    ZZ gpowB;
    ZZ gx1  = h;
    ZZ gx0 = to_ZZ("1");

    /*
    precompute g^-B, there is no need 
    for computing it every iteration
    */
    PowerMod( gpowB, g, B, p );
    InvMod( gpowB, gpowB, p );
   
    t = GetTime();

    /* ~23s */
    printf( "Building hashtable...\n\n" );

    int x1 = 1;
    for( ; x1 < B; x1++ )
    {
	   /* compute x1*g^-B */
	   MulMod( gx1, gx1, gpowB, p );

	   /* insert into table */
	   htInsert( tabulka, gx1, x1 );  
    }

    t = GetTime() - t;
    printf( "\nTime: %f sec\n", t );

    /* ~29s */
    printf( "Searching for collision...\n" );

    int x0 = 1;
    for(; x0 < B; x0++)
    {
	   /* (g^B)^x0 */
	   MulMod(gx0, gx0, g, p);

	   if(findExponents( tabulka, gx0, x0, g, h, p ) == 1)
		  break;
    }

    t = GetTime() - t;
   
    printf( "\nTime: %f sec\n", t );

    getchar();
    return 0;
}
