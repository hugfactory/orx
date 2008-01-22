/**
 * @file orxFileSystem.c
 */

/***************************************************************************
 orxFileSystem.c
 Lib C implementation of the File System module
 begin                : 21/01/2008
 author               : (C) Arcallians
 email                : keleborn@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"
#include "math/orxMath.h"
#include "io/orxFileSystem.h"

#ifdef __orxWINDOWS__

  #include <io.h>

#endif /* __orxWINDOWS__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Module flags
 */
#define orxFILESYSTEM_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxFILESYSTEM_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxFILESYSTEM_STATIC_t
{
  orxU32            u32Flags;
  
} orxFILESYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxFILESYSTEM_STATIC sstFileSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATIC orxINLINE orxVOID orxFileSystem_LibC_GetInfoFromData(orxCONST struct _finddata_t *_pstData, orxFILESYSTEM_INFO *_pstFileInfo)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Stores info */
  _pstFileInfo->u32Size       = _pstData->size;
  _pstFileInfo->u32TimeStamp  = _pstData->time_write;
  orxString_NCopy(_pstFileInfo->zName, (orxSTRING)_pstData->name, 255);
  orxString_Copy(_pstFileInfo->zFullName + orxString_GetLength(_pstFileInfo->zPath), _pstFileInfo->zName);
  _pstFileInfo->u32Flags      = (_pstData->attrib == 0)
                                ? orxFILESYSTEM_KU32_FLAG_INFO_NORMAL
                                : ((_pstData->attrib & _A_RDONLY) ? orxFILESYSTEM_KU32_FLAG_INFO_RDONLY : 0)
                                | ((_pstData->attrib & _A_HIDDEN) ? orxFILESYSTEM_KU32_FLAG_INFO_HIDDEN : 0)
                                | ((_pstData->attrib & _A_SUBDIR) ? orxFILESYSTEM_KU32_FLAG_INFO_DIR : 0);

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

orxSTATUS orxFileSystem_LibC_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstFileSystem, 0, sizeof(orxFILESYSTEM_STATIC));

    /* Updates status */
    sstFileSystem.u32Flags |= orxFILESYSTEM_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;  
}

orxVOID orxFileSystem_LibC_Exit()
{
  /* Was initialized? */
  if(sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Set(&sstFileSystem, 0, sizeof(orxFILESYSTEM_STATIC));
  }

  return;
}

orxBOOL orxFileSystem_LibC_FindFirst(orxCONST orxSTRING _zSearchPattern, orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxS32              s32Handle;

  /* Opens the search */
  s32Handle = _findfirst(_zSearchPattern, &stData);

  /* Valid? */
  if(s32Handle >= 0)
  {
    orxS32 s32LastSeparator, i;

    /* Gets last directory separator */
    for(s32LastSeparator = -1, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR, 0);
        i >= 0;
        s32LastSeparator = i, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR, i + 1));

    /* Stores path & full name base */
    orxMemory_Set(_pstFileInfo->zPath, 0, 1024);
    orxString_NCopy(_pstFileInfo->zPath, _zSearchPattern, (s32LastSeparator < 0) ? 1023 : orxMIN(s32LastSeparator + 1, 1023));
    orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

    /* Tranfers file info */
    orxFileSystem_LibC_GetInfoFromData(&stData, _pstFileInfo);

    /* Stores handle */
    _pstFileInfo->hInternal = (orxHANDLE)s32Handle;

    /* Updates result */
    bResult = orxTRUE;
  }

#else /* __orxWINDOWS__ */

  //! TODO : For linux use opendir()/readdir() + fnmatch() or glob()?

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

#endif /* __orxWINDOWS__ */
  
  /* Done! */
  return bResult;
}


orxBOOL orxFileSystem_LibC_FindNext(orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxS32              s32FindResult;

  /* Opens the search */
  s32FindResult = _findnext((orxS32)_pstFileInfo->hInternal, &stData);

  /* Valid? */
  if(s32FindResult == 0)
  {
    /* Tranfers file info */
    orxFileSystem_LibC_GetInfoFromData(&stData, _pstFileInfo);

    /* Updates result */
    bResult = orxTRUE;
  }

#else /* __orxWINDOWS__ */

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

#endif /* __orxWINDOWS__ */

  /* Done! */
  return bResult;
}

orxVOID orxFileSystem_LibC_FindClose(orxFILESYSTEM_INFO *_pstFileInfo)
{
  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

#ifdef __orxWINDOWS__

  /* Closes the search */
  _findclose((orxS32)_pstFileInfo->hInternal);

  /* Clears handle */
  _pstFileInfo->hInternal = 0;

#else /* __orxWINDOWS__ */

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

#endif /* __orxWINDOWS__ */

  return;
}

orxSTATUS orxFileSystem_LibC_Info(orxSTRING _zFileName, orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Finds for the file */
  if(orxFileSystem_LibC_FindFirst(_zFileName, _pstFileInfo) != orxFALSE)
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Closes the find */
  orxFileSystem_LibC_FindClose(_pstFileInfo);
    
  /* Done! */
  return eResult;
}

orxSTATUS orxFileSystem_LibC_Copy(orxSTRING _zSource, orxSTRING _zDest)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFileSystem_LibC_Rename(orxSTRING _zSource, orxSTRING _zDest)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  
  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFileSystem_LibC_Delete(orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");
  
  /* Done! */
  return eResult;
}

orxSTATUS orxFileSystem_LibC_CreateDir(orxSTRING _zDirName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  
  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFileSystem_LibC_DeleteDir(orxSTRING _zDirName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}


/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_USER_CORE_FUNCTION_START(FILESYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Init, FILESYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Exit, FILESYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_FindFirst, FILESYSTEM, FIND_FIRST);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_FindNext, FILESYSTEM, FIND_NEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_FindClose, FILESYSTEM, FIND_CLOSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Info, FILESYSTEM, INFO);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Copy, FILESYSTEM, COPY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Rename, FILESYSTEM, RENAME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Delete, FILESYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_CreateDir, FILESYSTEM, CREATE_DIR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_DeleteDir, FILESYSTEM, DELETE_DIR);
orxPLUGIN_USER_CORE_FUNCTION_END();
