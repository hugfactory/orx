/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxText.c
 * @date 02/12/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "display/orxText.h"

#include "display/orxDisplay.h"
#include "memory/orxMemory.h"
#include "core/orxConfig.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"


/** Module flags
 */
#define orxTEXT_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags */

#define orxTEXT_KU32_STATIC_FLAG_READY        0x00000001  /**< Ready flag */

#define orxTEXT_KU32_STATIC_MASK_ALL          0xFFFFFFFF  /**< All mask */

/** orxTEXT flags / masks
 */
#define orxTEXT_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxTEXT_KU32_MASK_ALL                 0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxTEXT_KZ_CONFIG_STRING              "String"
#define orxTEXT_KZ_CONFIG_FONT                "Font"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Text structure
 */
struct __orxTEXT_t
{
  orxSTRUCTURE  stStructure;                    /**< Public structure, first structure member : 16 */
  orxHANDLE     hData;                          /**< Data : 20 */
  orxSTRING     zReference;                     /**< Config reference : 24 */

  /* Padding */
  orxPAD(24)
};

/** Static structure
 */
typedef struct __orxTEXT_STATIC_t
{
  orxU32        u32Flags;                       /**< Control flags : 4 */

} orxTEXT_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

orxSTATIC orxTEXT_STATIC sstText;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all texts
 */
orxSTATIC orxINLINE orxVOID orxText_DeleteAll()
{
  orxTEXT *pstText;

  /* Gets first text */
  pstText = orxTEXT(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXT));

  /* Non empty? */
  while(pstText != orxNULL)
  {
    /* Deletes text */
    orxText_Delete(pstText);

    /* Gets first text */
    pstText = orxTEXT(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXT));
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups the text module
 */
orxVOID orxText_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_TEXT, orxMODULE_ID_HASHTABLE);

  return;
}

/** Inits the text module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxText_Init()
{
  orxSTATUS eResult;

  /* Not already Initialized? */
  if(!(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstText, sizeof(orxTEXT_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(TEXT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Success? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Updates flags for screen text creation */
      sstText.u32Flags = orxTEXT_KU32_STATIC_FLAG_READY;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to initialize text module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Not initialized? */
  if(eResult != orxSTATUS_SUCCESS)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Initializing text module failed.");

    /* Updates Flags */
    sstText.u32Flags &= ~orxTEXT_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the text module
 */
orxVOID orxText_Exit()
{
  /* Initialized? */
  if(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY)
  {
    /* Deletes text list */
    orxText_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_TEXT);

    /* Updates flags */
    sstText.u32Flags &= ~orxTEXT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to exit text module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty text
 * @return      orxTEXT / orxNULL
 */
orxTEXT *orxText_Create()
{
  orxTEXT *pstResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);

  /* Creates text */
  pstResult = orxTEXT(orxStructure_Create(orxSTRUCTURE_ID_TEXT));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Creates internal data */
    pstResult->hData = orxDisplay_CreateText();

    /* Valid? */
    if(pstResult->hData != orxNULL)
    {
      /* Inits flags */
      orxStructure_SetFlags(pstResult, orxTEXT_KU32_FLAG_NONE, orxTEXT_KU32_MASK_ALL);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create internal data for text.");

      /* Deletes structure */
      orxStructure_Delete(pstResult);
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create structure for text.");
  }

  return pstResult;
}

/** Creates a text from config
 * @param[in]   _zConfigID    Config ID
 * @return      orxTEXT / orxNULL
 */
orxTEXT *orxFASTCALL orxText_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxTEXT    *pstResult;
  orxSTRING   zPreviousSection;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates text */
    pstResult = orxText_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32 u32Flags;

      /* Inits flags */
      u32Flags = orxTEXT_KU32_FLAG_NONE;

      /* Stores text */
      orxText_SetString(pstResult, orxConfig_GetString(orxTEXT_KZ_CONFIG_STRING));

      /* Stores font */
      orxText_SetFont(pstResult, orxConfig_GetString(orxTEXT_KZ_CONFIG_FONT));

      /* Stores its reference key */
      pstResult->zReference = orxConfig_GetCurrentSection();

      /* Stores flags */
      orxStructure_SetFlags(pstResult, u32Flags, orxTEXT_KU32_FLAG_NONE);
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't find config section named (%s).", _zConfigID);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Deletes a text
 * @param[in]   _pstText      Concerned text
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_Delete(orxTEXT *_pstText)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstText) == 0)
  {
    /* Deletes internal data */
    orxDisplay_DeleteText(_pstText->hData);

    /* Deletes structure */
    orxStructure_Delete(_pstText);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to delete text object when it was still referenced.");

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets text size
 * @param[in]   _pstText      Concerned text
 * @param[out]  _pfWidth      Text's width
 * @param[out]  _pfHeight     Text's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_GetSize(orxCONST orxTEXT *_pstText, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxFLOAT eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets text size */
  eResult = orxDisplay_GetTextSize(_pstText->hData, _pfWidth, _pfHeight);

  /* Done! */
  return eResult;
}

/** Gets text name
 * @param[in]   _pstText      Concerned text
 * @return      Text name / orxNULL
 */
orxSTRING orxFASTCALL orxText_GetName(orxCONST orxTEXT *_pstText)
{
  orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Updates result */
  zResult = (_pstText->zReference != orxNULL) ? _pstText->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Gets text string 
 * @param[in]   _pstText      Concerned text
 * @return      Text string / orxSTRING_EMPTY
 */
orxSTRING orxFASTCALL orxText_GetString(orxCONST orxTEXT *_pstText)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Gets result */
  zResult = orxDisplay_GetTextString(_pstText->hData);
  if(zResult == orxNULL)
  {
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Gets text font
 * @param[in]   _pstText      Concerned text
 * @return      Text font / orxNULL
 */
orxSTRING orxFASTCALL orxText_GetFont(orxCONST orxTEXT *_pstText)
{
  orxSTRING zResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Gets result */
  zResult = orxDisplay_GetTextFont(_pstText->hData);

  /* Done! */
  return zResult;
}

/** Gets text data
 * @param[in]   _pstText      Concerned text
 * @return      orxDISPLAY_TEXT / orxNULL
 */
orxDISPLAY_TEXT *orxFASTCALL orxText_GetData(orxCONST orxTEXT *_pstText)
{
  orxDISPLAY_TEXT *pstResult;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Gets result */
  pstResult = (orxDISPLAY_TEXT *)(_pstText->hData);

  /* Done! */
  return pstResult;
}

/** Sets text string 
 * @param[in]   _pstText      Concerned text
 * @param[in]   _zString      String to contain
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_SetString(orxTEXT *_pstText, orxCONST orxSTRING _zString)
{
  orxSTRING zString;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Gets current string */
  zString = orxDisplay_GetTextString(_pstText->hData);

  /* Valid? */
  if(zString != orxNULL)
  {
    /* Deletes it */
    orxString_Delete(zString);
  }

  /* Has new string? */
  if(_zString != orxNULL)
  {
    /* Stores a duplicate */
    orxDisplay_SetTextString(_pstText->hData, orxString_Duplicate(_zString));
  }
  else
  {
    /* Clears string */
    orxDisplay_SetTextString(_pstText->hData, orxNULL);
  }

  /* Done! */
  return eResult;
}

/** Sets text font
 * @param[in]   _pstText      Concerned text
 * @param[in]   _zFont        Font name / orxNULL to use default
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxText_SetFont(orxTEXT *_pstText, orxCONST orxSTRING _zFont)
{
  orxSTRING zFont;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstText.u32Flags & orxTEXT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstText);

  /* Gets current font */
  zFont = orxDisplay_GetTextFont(_pstText->hData);

  /* Valid? */
  if(zFont != orxNULL)
  {
    /* Deletes it */
    orxString_Delete(zFont);
  }

  /* Has new font? */
  if((_zFont != orxNULL) && (_zFont != orxSTRING_EMPTY))
  {
    /* Stores a duplicate */
    orxDisplay_SetTextFont(_pstText->hData, orxString_Duplicate(_zFont));
  }
  else
  {
    /* Clears string */
    orxDisplay_SetTextFont(_pstText->hData, orxNULL);
  }

  /* Done! */
  return eResult;
}
