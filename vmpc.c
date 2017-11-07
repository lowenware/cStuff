#include <stdio.h>
#include <string.h>
#include "vmpc.h"
/* Implementation of the VMPC
 * Author of the algorithms: Bartosz Zoltak (www.vmpcfunction.com)
 *
 * Implementation by Ilja Kartaschoff <ik@lowenware.com>
 * */



/* init context ------------------------------------------------------------- */

/* Key initialization sub routine for keys and vectors of random length */
static void
vmpc_init_round(vmpc_context_t ctx, unsigned char rounds, unsigned char * ptr, unsigned char len)
{
  unsigned char i=0,
                t,
                r;
  unsigned int  x;

  for (r=1; r<=rounds; r++)
  { 
     for (x=0; x<256; x++)
     {
        ctx->A=(ctx->P[ (ctx->A + ctx->F + ptr[i]) & 255 ] + i) & 255;
        if (++i==len) i=0;
        ctx->B=(ctx->S[ (ctx->B + ctx->A + ptr[i]) & 255 ] + i) & 255;
        if (++i==len) i=0;
        ctx->C=(ctx->P[ (ctx->C + ctx->B + ptr[i]) & 255 ] + i) & 255;
        if (++i==len) i=0;
        ctx->D=(ctx->S[ (ctx->D + ctx->C + ptr[i]) & 255 ] + i) & 255;
        if (++i==len) i=0;
        ctx->E=(ctx->P[ (ctx->E + ctx->D + ptr[i]) & 255 ] + i) & 255;
        if (++i==len) i=0;
        ctx->F=(ctx->S[ (ctx->F + ctx->E + ptr[i]) & 255 ] + i) & 255;  
        if (++i==len) i=0;

        t=ctx->P[x];       ctx->P[x]     =ctx->P[ctx->B];  ctx->P[ctx->B]=t;
        t=ctx->S[x];       ctx->S[x]     =ctx->S[ctx->E];  ctx->S[ctx->E]=t;
        t=ctx->P[ctx->D];  ctx->P[ctx->D]=ctx->P[ctx->F];  ctx->P[ctx->F]=t;
        t=ctx->S[ctx->A];  ctx->S[ctx->A]=ctx->S[ctx->C];  ctx->S[ctx->C]=t;
     }
  }
}

void
vmpc_init_context( vmpc_context_t ctx, unsigned char *key,
                                       unsigned char *vec,
                                       int            k_len,
                                       int            v_len )
{
  unsigned int  i;
  unsigned char r;
  for (i=0; i<256; i++)
  {
    ctx->P[i]=i;
  }
  memcpy(ctx->S, ctx->P, 256);
  ctx->A=0;
  ctx->B=0;
  ctx->C=0;
  ctx->D=0;
  ctx->E=0;
  ctx->F=0;
  ctx->G=0;

  i = (k_len * k_len / (6*256));
  if (k_len * k_len % (6*256)) i++;

  r = (v_len * v_len / (6*256));
  if (v_len * v_len % (6*256)) r++;

  vmpc_init_round(ctx, i, key, k_len);
  vmpc_init_round(ctx, r, vec, v_len);
  vmpc_init_round(ctx, i, key, k_len);
  
  ctx->G=ctx->S[(ctx->S[ctx->S[ (ctx->C + ctx->D) & 255 ]]+1) & 255];

  for (i=0; i<256; i++)
  {
    ctx->A=ctx->P[ (ctx->A + ctx->C + ctx->S[ctx->G]) & 255 ];
    ctx->B=ctx->P[ (ctx->B + ctx->A) & 255 ];
    ctx->C=ctx->P[ (ctx->C + ctx->B) & 255 ];
    ctx->D=ctx->S[ (ctx->D + ctx->F + ctx->P[ctx->G]) & 255 ];
    ctx->E=ctx->S[ (ctx->E + ctx->D) & 255 ];
    ctx->F=ctx->S[ (ctx->F + ctx->E) & 255 ];
    r=ctx->P[ctx->G];  ctx->P[ctx->G]=ctx->P[ctx->F];  ctx->P[ctx->F]=r;
    r=ctx->S[ctx->G];  ctx->S[ctx->G]=ctx->S[ctx->A];  ctx->S[ctx->A]=r;
    ctx->G++;
  }
}



void
vmpc_xcrypt(vmpc_context_t ctx, unsigned char *data, unsigned int d_len)
{
  unsigned char t;
  unsigned int  i;
  for (i=0; i<d_len; i++)
  {
    ctx->A=ctx->P[ (ctx->A + ctx->C + ctx->S[ctx->G]) & 255 ];
    ctx->B=ctx->P[ (ctx->B + ctx->A) & 255 ];
    ctx->C=ctx->P[ (ctx->C + ctx->B) & 255 ];
    ctx->D=ctx->S[ (ctx->D + ctx->F + ctx->P[ctx->G]) & 255 ];
    ctx->E=ctx->S[ (ctx->E + ctx->D) & 255 ];
    ctx->F=ctx->S[ (ctx->F + ctx->E) & 255 ];

    //Data[x]=S[(S[S[ (c + d) & 255 ]]+1) & 255];      //pseudo-random number generation /**/
    data[i]^=ctx->S[(ctx->S[ctx->S[ (ctx->C + ctx->D) & 255 ]]+1) & 255];   //encryption / decryption         /**/

    t=ctx->P[ctx->G];  ctx->P[ctx->G]=ctx->P[ctx->F];  ctx->P[ctx->F]=t;
    t=ctx->S[ctx->G];  ctx->S[ctx->G]=ctx->S[ctx->A];  ctx->S[ctx->A]=t;

    ctx->G++;
  }
}

