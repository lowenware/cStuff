#ifndef _STANDARD_VMPC_H_
#define _STANDARD_VMPC_H_


/* Implementation of the VMPC
 * Author of the algorithms: Bartosz Zoltak (www.vmpcfunction.com)
 *
 * Implementation by Ilja Kartaschoff <ik@lowenware.com>
 * */

/* VMPC Context ------------------------------------------------------------- */

struct vmpc_context
{
  unsigned char P[256];
  unsigned char S[256];
  unsigned char A;
  unsigned char B;
  unsigned char C;
  unsigned char D;
  unsigned char E;
  unsigned char F;
  unsigned char G;
};

typedef struct vmpc_context  _vmpc_context_t;
typedef struct vmpc_context * vmpc_context_t;


void
vmpc_init_context( vmpc_context_t ctx, unsigned char *key,
                                       unsigned char *vec,
                                       int            k_len,
                                       int            v_len );

void
vmpc_xcrypt(vmpc_context_t ctx, unsigned char *data, unsigned int d_len);

#endif
