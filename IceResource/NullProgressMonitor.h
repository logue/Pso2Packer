#pragma once
#include "IProgressMonitor.h"

class NullProgressMonitor : public IProgressMonitor {
public:
  NullProgressMonitor(void);
  virtual ~NullProgressMonitor(void);

  virtual void BeginWork(LPCTSTR lpszName, int totalWork);

  virtual void Progress(int work);

  virtual void Done();

  virtual void SetTaskName(LPCTSTR name);

  virtual void SetSubTaskName(LPCTSTR lpszName);

  virtual bool IsCanceled();

  virtual void SetCanceled(bool value);

private:
  bool m_bCanceled;
};
