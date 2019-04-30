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
#include "trustx_eng_common.h"

#include <string.h>
#include <openssl/engine.h>
#include "trustx_eng.h"



static const char *ENGINE_ID   = "trustx";
static const char *ENGINE_NAME = "Infineon OPTIGA Trust X engine";

static int engine_init(ENGINE *e);
static int engine_finish(ENGINE *e);
static int engine_destroy(ENGINE *e);
static int engine_bind_helper(ENGINE *e);



static int engine_init(ENGINE *e)
{
  DBGFN("> Engine 0x%x init", (unsigned int) e);
  if (trexe_init() != 1)
  {
    ERRFN("Engine context init failed");
    return 0;
  }
  return 1;
}


static int engine_destroy(ENGINE *e)
{
  DBGFN("> Engine 0x%x destroy", (unsigned int) e);
  trexe_close();
  return 1;
}


static int engine_finish(ENGINE *e)
{
  DBGFN("> Engine 0x%x finish (releasing functional reference)", (unsigned int) e);
  return 1;
}



static int engine_bind_helper(ENGINE *e)
{
  DBGFN("> Binding engine %x to OpenSSL/libssl", (unsigned int) e);

  if (NULL
      || !ENGINE_set_id(e, ENGINE_ID)
      || !ENGINE_set_name(e, ENGINE_NAME)
      || !ENGINE_set_init_function(e, engine_init)
      || !ENGINE_set_finish_function(e, engine_finish)
      || !ENGINE_set_destroy_function(e, engine_destroy)
      || !ENGINE_set_load_privkey_function(e, trexe_loadKey)
      )
    {
      ERRFN("< Failed to set engine function");
      return 0;
    }

  DBGFN("< Engine successfully initialized");
  return 1;
}



/*
 * Needed if this ENGINE is being compiled into a self-contained shared library.
 */
#ifdef ENGINE_DYNAMIC_SUPPORT

static int bind_fn(ENGINE *e, const char *id)
  {
    DBGFN(">");

    if (!id)
      {
	DBGFN("Error: id is null");
	return 0;
      }

    if (strcmp(id, ENGINE_ID))
      {
	DBGFN("engine_bind: no engine ID match:");
	//printf("  \"%s\"\n", id);
	/* When the engine is loaded via '$ openssl engine <path-to-engine.so>', the id contains the full path to the engine.so
	 * Therefore, the string comparison with ENGINE_ID would return a difference.
	 */
	//return 0;
      }

    if (!engine_bind_helper(e))
      {
	return 0;
      }

    DBGFN("<");
    return 1;
  }

IMPLEMENT_DYNAMIC_CHECK_FN()
IMPLEMENT_DYNAMIC_BIND_FN(bind_fn)

#endif /* ENGINE_DYNAMIC_SUPPORT */
