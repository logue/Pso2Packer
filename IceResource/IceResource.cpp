// IceResource.cpp : DLL
// アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "IceResource.h"
#include "IProgressMonitor.h"
#include "IResource.h"

class ProgressMonitor : public IProgressMonitor {
public:
  ProgressMonitor(ProgressMonitorProc proc, DWORD dwParameter)
      : m_pfnProc(proc), m_dwParameter(dwParameter), m_bCanceled(false) {}

  virtual ~ProgressMonitor() {}

  virtual void BeginWork(LPCTSTR lpszName, int totalWork) {
    if (m_pfnProc == NULL) {
      return;
    }
    ProgressMonitorData data;
    data.type = PROGRESS_MONITOR_BEGIN_TASK;
    data.dwParamter = m_dwParameter;
    data.content.beginTask.lpszTaskName = lpszName;
    data.content.beginTask.totalWork = totalWork;
    SetCanceled(!m_pfnProc(&data));
  }

  virtual void Progress(int work) {
    if (m_pfnProc == NULL) {
      return;
    }
    ProgressMonitorData data;
    data.type = PROGRESS_MONITOR_PROGRESS;
    data.dwParamter = m_dwParameter;
    data.content.progress.work = work;
    SetCanceled(!m_pfnProc(&data));
  }

  virtual void Done() {
    if (m_pfnProc == NULL) {
      return;
    }
    ProgressMonitorData data;
    data.type = PROGRESS_MONITOR_DONE;
    data.dwParamter = m_dwParameter;
    SetCanceled(!m_pfnProc(&data));
  }

  virtual void SetTaskName(LPCTSTR name) {
    if (m_pfnProc == NULL) {
      return;
    }
    ProgressMonitorData data;
    data.type = PROGRESS_MONITOR_SET_TASK_NAME;
    data.dwParamter = m_dwParameter;
    data.content.setTaskName.lpszName = name;
    SetCanceled(!m_pfnProc(&data));
  }

  virtual void SetSubTaskName(LPCTSTR lpszName) {
    if (m_pfnProc == NULL) {
      return;
    }
    ProgressMonitorData data;
    data.type = PROGRESS_MONITOR_SET_SUB_TASK_NAME;
    data.dwParamter = m_dwParameter;
    data.content.setSubTaskName.lpszName = lpszName;
    SetCanceled(!m_pfnProc(&data));
  }

  virtual bool IsCanceled() { return m_bCanceled; }

  virtual void SetCanceled(bool value) {
    if (m_bCanceled != value) {
      m_bCanceled = value;
      // TODO
    }
  }

private:
  ProgressMonitorProc m_pfnProc;
  DWORD m_dwParameter;
  bool m_bCanceled;
};

//////////////////////////////////////////////////////////////////////////

ICERESOURCE_API PACK_RESOURCE_SET_HANDLE
CreateResourceSetFromFolder(LPCTSTR lpszFolder) {
  return 0;
}

ICERESOURCE_API void CloseResourceSet(PACK_RESOURCE_SET_HANDLE hResourceSet) {
  delete hResourceSet;
}

//////////////////////////////////////////////////////////////////////////
