
#ifndef _WIN32
# include <string.h>
# include <signal.h>
#else
# include <Windows.h>
#endif

namespace libpython {
extern void (*PyErr_SetInterrupt)();
} // namespace libpython

// flag indicating whether R interrupts are pending
#ifndef _WIN32
extern "C" int R_interrupts_pending;
#else
extern "C" int UserBreak;
#endif

namespace reticulate {
namespace signals {

void setInterruptsPending(bool value) {
  
#ifndef _WIN32
  R_interrupts_pending = value ? 1 : 0;
#else
  UserBreak = value ? 1 : 0;
#endif
  
}

void interruptHandler(int signum) {
  
  // set R interrupts pending
  setInterruptsPending(true);
  
  // set Python interrupts pending
  libpython::PyErr_SetInterrupt();
  
}

#ifndef _WIN32

void registerInterruptHandlerUnix() {
  
  // initialize sigaction struct
  struct sigaction sigint;
  memset(&sigint, 0, sizeof sigint);
  sigemptyset(&sigint.sa_mask);
  sigint.sa_flags = 0;
  
  // set handler
  sigint.sa_handler = interruptHandler;
  
  // install signal handler
  sigaction(SIGINT, &sigint, NULL);
  
}

#else

BOOL CALLBACK consoleCtrlHandler(DWORD type)
{
  switch (type)
  {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
    interruptHandler(2);
  }
}

void registerInterruptHandlerWin32() {
  
  // accept Ctrl + C interrupts
  ::SetConsoleCtrlHandler(nullptr, FALSE);
  
  // remove an old registration, if any
  ::SetConsoleCtrlHandler(consoleCtrlHandler, FALSE);
  
  // and register the handler
  ::SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
  
}

#endif

void registerInterruptHandler() {
#ifndef _WIN32
  registerInterruptHandlerUnix();
#else
  registerInterruptHandlerWin32();
#endif
}

} // end namespace signals
} // end namespace reticulate

// [[Rcpp::export]]
void initialize_interrupt_handler() {
  reticulate::signals::registerInterruptHandler();
}