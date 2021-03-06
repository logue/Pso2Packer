#pragma once

#include "ExportMacro.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PACK_RESOURCE_SET {};
struct PACK_RESOURCE {};

typedef PACK_RESOURCE_SET *PACK_RESOURCE_SET_HANDLE;
typedef PACK_RESOURCE *PACK_RESOURCE_HANDLE;

#define PROGRESS_MONITOR_BEGIN_TASK 0
#define PROGRESS_MONITOR_PROGRESS 1
#define PROGRESS_MONITOR_DONE 2
#define PROGRESS_MONITOR_SET_TASK_NAME 3
#define PROGRESS_MONITOR_SET_SUB_TASK_NAME 4

#define ICE_CRYPT_KEY 0xCD50379E

struct ProgressMonitorData {
  int type;
  union {
    struct _BeginTask {
      LPCTSTR lpszTaskName;
      int totalWork;
    } beginTask;
    struct _Progress {
      int work;
    } progress;
    struct _Done {

    } done;

    struct _SetTaskName {
      LPCTSTR lpszName;
    } setTaskName;

    struct _SetSubTaskName {
      LPCTSTR lpszName;
    } setSubTaskName;
  } content;
  DWORD dwParamter;
};
// Callback function in the suppression of the package file
// False is interrupted.
typedef bool (*ProgressMonitorProc)(ProgressMonitorData *pData);

//////////////////////////////////////////////////////////////////////////
// Create a set of resources on and off the set of resources manage
ICERESOURCE_API PACK_RESOURCE_SET_HANDLE
CreateResourceSetFromFolder(LPCTSTR lpszFolder);
ICERESOURCE_API PACK_RESOURCE_SET_HANDLE
CreateResourceSetFromFile(LPCTSTR lpszFile);
ICERESOURCE_API void CloseResourceSet(PACK_RESOURCE_SET_HANDLE hResourceSet);
//////////////////////////////////////////////////////////////////////////

// Create a resource from a file
ICERESOURCE_API PACK_RESOURCE_HANDLE CreateResourceFromFile(
    LPCTSTR lpszFile, LPCTSTR lpszResourceName, size_t version);

// The release of a resource reference
ICERESOURCE_API void CloseResource(PACK_RESOURCE_HANDLE hResource);

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
