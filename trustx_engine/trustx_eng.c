/**
* MIT License
*
* Copyright (c) 2019 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE

*/
#include "trustx_eng.h"
#include "trustx_eng_common.h"

//#include "ifx_optiga_command_library.h"
#include "trustx.h"

#include <string.h>

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

/* PROTOTYPES */

static ECDSA_SIG* trexe_ecdsa_signSig(
  const unsigned char  *dgst,
  int                   dgst_len,
  const BIGNUM         *inv,
  const BIGNUM         *rp,
  EC_KEY               *eckey
);

static int trexe_ecdsa_sign(
  int                   type,
  const unsigned char  *dgst,
  int                   dlen,
  unsigned char        *sig,
  unsigned int         *siglen,
  const BIGNUM         *kinv,
  const BIGNUM         *r,
  EC_KEY               *eckey
);

EC_KEY_METHOD *trexe_ec_get_ec_key_method(void);

/* Global engine context; to be extended if necessary */

typedef struct trexe_ctx_str
{
  char              key[KEY_CONTEXT_MAX_LEN];
  uint16_t			key_oid;
  char				pubkeyfilename[256];
  EC_KEY_METHOD     *ec_key_method;
} trexe_ctx_t;

static trexe_ctx_t trexe_ctx;

// Engine Helper
/*
static void _hexdump(uint8_t *data, uint16_t len)
{
	uint16_t j,k;

	printf("\t");
	k=0;
	for (j=0;j<len;j++)
	{
		printf("%.2X ", data[j]);
		if(k < 15)
		{
			k++;
		}	
		else
		{
			printf("\n\t");
			k=0;
		}
	}
	printf("\n");
}
*/
static uint32_t parseKeyParams(const char *aArg)
{
	uint32_t value;
	char in[1024];

	char *token[5];
	int   i;
	  
	DBGFN(">");
    
    strncpy(in, aArg,1024);
    
	if (aArg == NULL)
	{
		ERRFN("No input key parameters present. (key_oid:<pubkeyfile>)");
		return EVP_FAIL;
	}
	  
	i = 0;
	token[0] = strtok((char *)aArg, ":");
	
	if (token[0] == NULL)
	{
	  ERRFN("Too few parameters in key parameters list. (key_oid:<pubkeyfile>)");
	  return EVP_FAIL;
	}

	while (token[i] != NULL)
	{
		i++;
		token[i] = strtok(NULL, ":");
	}

	if (i > 2)
	{
	  ERRFN("Too many parameters in key parameters list. (key_oid:<pubkeyfile>)");
	  return EVP_FAIL;
	}
	
	if (strncmp(token[0], "0x",2) == 0)
		sscanf(token[0],"%x",&value);
	else
		value = 0;

	trexe_ctx.key_oid = value;
	if (token[1] != NULL)
		strncpy(trexe_ctx.pubkeyfilename, token[1], 256);
	else
		trexe_ctx.pubkeyfilename[0]='\0';

	if (((value < 0xE0F0) || (value > 0xE0F3)) &&
		((value < 0xE100) || (value > 0xE103)))
	{
	  ERRFN("Invalid Key OID");
	  return EVP_FAIL;
	}

	DBGFN("<");

	return value;
}

/*
 * Initializes the global engine context.
 * Return 1 on success, otherwise 0.
 */
int trexe_init(void)
{
  DBGFN(">");

  trexe_ctx.key[0] = '\0';
  trexe_ctx.ec_key_method = NULL;
  trexe_ctx.key_oid = 0;
  trexe_ctx.pubkeyfilename[0] = '\0';

//  if (optiga_open_application() != IFX_I2C_STACK_SUCCESS)
  if (trustX_Open() != OPTIGA_LIB_SUCCESS)
  {
    ERRFN("< failed to open OPTIGA application.\n");
    return 0;
  }

  DBGFN("<");
  return 1;
}



void trexe_close(void)
{
  DBGFN(">");
  DBGFN("<");
}

/*
 * With command
 * $ openssl pkey -in mykeyid -engine optiga_trust_ex -inform ENGINE -text_pub -noout
 * the "mykeyid" lands in the parameter "key_id".
 */
EVP_PKEY *trexe_loadKey(
  ENGINE      *e,
  const char  *key_id,
  UI_METHOD   *ui,
  void        *cb_data)
{
  EVP_PKEY    *key         = NULL;
  EC_KEY      *ecKey       = NULL;
  X509        *x509_cert   = NULL;
  uint8_t      cert[1024];                  // TODO: refactor constant
  uint32_t     cert_len    = sizeof(cert);

  optiga_lib_status_t return_status;
  
  optiga_key_id_t optiga_key_id;
  uint16_t key_oid;
  FILE *fp;
  char *name;
  char *header;
  uint8_t *data;
  uint32_t len;
  uint8_t pubkey[150];
  
  uint16_t i;
  uint8_t eccheader256[] = {0x30,0x59, // SEQUENCE
							0x30,0x13, // SEQUENCE
							0x06,0x07, // OID:1.2.840.10045.2.1
							0x2A,0x86,0x48,0xCE,0x3D,0x02,0x01,
							0x06,0x08, // OID:1.2.840.10045.3.1.7
							0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x07};

  DBGFN(">");
  DBGFN("key_id=<%s>", key_id);
  DBGFN("cb_data=0x<%x>", (unsigned int) cb_data);


  while (1)
  {
	  
	parseKeyParams(key_id);
	
	key_oid = trexe_ctx.key_oid;
	
	DBGFN("KEY_OID : 0x%.4x \n",key_oid);
	if (key_oid == 0xE0F0)
	{
		DBGFN("Using internal Cert\n");
		cert_len = sizeof(cert);
		return_status = trustX_readCert(eDEVICE_PUBKEY_CERT_IFX, cert, &cert_len);
		if (return_status != OPTIGA_LIB_SUCCESS)
		{
		  ERRFN("failed to read certificate with public key from OPTIGA");
		  break;
		}
		DBGFN("Got raw cert from OPTIGA: 0x%x bytes", cert_len);
	  
		const unsigned char *p = cert;
		x509_cert = d2i_X509(NULL, &p, cert_len);
		if (x509_cert == NULL)
		{
		  // TODO/Hack: If certificate does not start with proper tag
		  DBGFN("applying metadata hack");
		  const unsigned char *p2 = cert + 9;
		  x509_cert = d2i_X509(NULL, &p2, cert_len - 9);
		  if (x509_cert == NULL)
		  {
			ERRFN("failed to parse certificate data from OPTIGA");
			break;
		  }
		}
		DBGFN("Parsed X509 from raw cert");
  
		key = X509_get_pubkey(x509_cert);
		if (key == NULL)
		{
		  ERRFN("failed to extract public key from X509 certificate");
		  break;
		}
		DBGFN("Extracted public key from cert");
	}
	else
	{
		if (trexe_ctx.pubkeyfilename[0] != '\0')
		{
			DBGFN("filename : %s\n",trexe_ctx.pubkeyfilename);
			//open 
			fp = fopen((const char *)trexe_ctx.pubkeyfilename,"r");
			if (!fp)
			{
				ERRFN("failed to open file %s\n",trexe_ctx.pubkeyfilename);
				break;
			}
			PEM_read(fp, &name,&header,&data,(long int *)&len);
			//DBGFN("name   : %s\n",name);
			//DBGFN("len : %d\n",len);
			//_hexdump(data,len); 
			key = d2i_PUBKEY(NULL,(const unsigned char **)&data,len);
			
		}
		else // if no pubkey file input then gen new key
		{
			DBGFN("No Pubkey filename.\n");
			optiga_key_id = trexe_ctx.key_oid;
			len = sizeof(pubkey);
			for (i=0; i < sizeof(eccheader256);i++)
			{
				pubkey[i] = eccheader256[i];
			}
			
			return_status = optiga_crypt_ecc_generate_keypair(0x03, //key size 256
													0x01, // Type = Auth
													FALSE,
													&optiga_key_id,
													(pubkey+i),
													(uint16_t *)&len);
			if (return_status != OPTIGA_LIB_SUCCESS)
			{
				ERRFN("Error!!! [0x%.8X]\n",return_status);
				break;
			}

			//DBGFN("len : %d\n",len+i);			
			//_hexdump(pubkey,len+i);
			data = pubkey;		
			key = d2i_PUBKEY(NULL,(const unsigned char **)&data,len+i);
		}
	}

	ecKey = EVP_PKEY_get1_EC_KEY(key);
    EC_KEY_set_method(ecKey, trexe_ec_get_ec_key_method());

    return key; // SUCCESS
  }

  ERRFN("<");
  return (EVP_PKEY *) NULL; // RETURN FAIL
}



EC_KEY_METHOD *trexe_ec_get_ec_key_method(void)
{
  DBGFN(">");

  if (trexe_ctx.ec_key_method != NULL)
  {
    return trexe_ctx.ec_key_method;
  }

  if ((trexe_ctx.ec_key_method = EC_KEY_METHOD_new(trexe_ctx.ec_key_method)) == NULL)
  {
    ERRFN("< could not create new EC key method");
    return NULL;
  }

  int (*pkeygen)(EC_KEY *key);
  
  EC_KEY_METHOD *default_ec_key_method = (EC_KEY_METHOD *) EC_KEY_get_default_method();
  EC_KEY_METHOD_get_keygen(default_ec_key_method, &pkeygen);

  EC_KEY_METHOD_set_sign(
    trexe_ctx.ec_key_method,
    trexe_ecdsa_sign,        /* sign(...)       */
    NULL,                    /* sign_setup(...) */
    trexe_ecdsa_signSig      /* sign_sig(...)   */
  );

  EC_KEY_METHOD_set_keygen(
    trexe_ctx.ec_key_method,
    pkeygen
  );

  // Verify method
  int (*pverify)(
    int type,
    const unsigned char *dgst,
    int dgst_len,
    const unsigned char *sigbuf,
    int sig_len,
    EC_KEY eckey);
  int (*pverify_sig)(
    const unsigned char *dgst,
    int dgst_len,
    const ECDSA_SIG *sig,
    EC_KEY *eckey);
  EC_KEY_METHOD_get_verify(default_ec_key_method, 
							(int (**)(int, const unsigned char *, int,
                                              const unsigned char *,
                                              int, EC_KEY *))
							&pverify, 
							&pverify_sig);
  
  EC_KEY_METHOD_set_verify(trexe_ctx.ec_key_method, 
							(int (*)(int, const unsigned char *, int,
                                          const unsigned char *,
                                          int, EC_KEY *))
							pverify, 
							pverify_sig);

  DBGFN("<");
  return trexe_ctx.ec_key_method;
}



static int trexe_ecdsa_sign(
  int                   type,
  const unsigned char  *dgst,
  int                   dlen,
  unsigned char        *sig,
  unsigned int         *siglen,
  const BIGNUM         *kinv,
  const BIGNUM         *r,
  EC_KEY               *eckey)
{
  ECDSA_SIG *s;

  DBGFN(">");

  s = trexe_ecdsa_signSig(dgst, dlen, kinv, r, eckey);
  if (s == NULL)
  {
    *siglen = 0;
    DBGFN("Signature is NULL");
    return EVP_FAIL;
  }

  *siglen = i2d_ECDSA_SIG(s, &sig);
  ECDSA_SIG_free(s);

  DBGFN("<");
  return EVP_SUCCESS;
}



static ECDSA_SIG* trexe_ecdsa_signSig(
  const unsigned char  *dgst,
  int                   dgst_len,
  const BIGNUM         *in_kinv,
  const BIGNUM         *in_r,
  EC_KEY               *eckey
)
{
  DBGFN("ECDSA signature computation requested with &EC_KEY=0x<%x> und key_id=<0x%.4x>.",
    (unsigned int) eckey, trexe_ctx.key_oid);
  DBGFN("ECDSA signature computation for dgst=0x<%x> and dgst_len=0x<%x>=%d",
    (unsigned int) dgst, dgst_len, dgst_len);
 
  ECDSA_SIG  *ecdsa_sig = NULL;
  uint8_t     sig[256];
  uint16_t    sig_len;
  //uint16_t    status;
  uint8_t     sig_der[256];

  optiga_lib_status_t return_status;

  // TODO/HACK:
  if (dgst_len != 32)
  {
    dgst_len = 32;
    DBGFN("APPLIED digest length hack");
  }

/*
  status = optiga_calc_sign(
    dgst,
    dgst_len,
    sig,
    &sig_len);
  if (status != IFX_I2C_STACK_SUCCESS)
*/
   return_status = optiga_crypt_ecdsa_sign((uint8_t *)dgst,
                                           dgst_len,
										   trexe_ctx.key_oid,
                                           sig, 
                                           &sig_len);
  if (return_status != OPTIGA_LIB_SUCCESS)                                             
  {
    ERRFN("Could not get signature form OPTIGA");
    return (ECDSA_SIG *) NULL;
  }

  DBGFN("Signature received from TREX: sig=%x, sig_len=0x%x=%d",
    (unsigned int) sig,
    sig_len, sig_len);

  memcpy(sig_der + 2, sig, sig_len);
  sig_der[0] = 0x30;
  sig_der[1] = sig_len;
  const unsigned char *p = sig_der;
  ecdsa_sig = d2i_ECDSA_SIG(NULL, &p, sig_len+2);
  
  DBGFN("<");
  return ecdsa_sig;
}

