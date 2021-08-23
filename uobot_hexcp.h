int CheckHang;
int LastTickMsg;
void DumpErrorInformation(CONTEXT* CR);
LONG __stdcall MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);
int MyWatchForHangClientThread(LPVOID inutil);
