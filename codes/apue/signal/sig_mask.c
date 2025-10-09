#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

extern const char *const sys_siglist[];
void handle_signal(int sig_num);
volatile sig_atomic_t unblock_sig_int = 0;
sigset_t old_sig_mask, new_sig_mask;
/**
 *
 * If and when the
signal-catching function returns, the signal mask of the process is reset to its previous
value. This way, we are able to block certain signals whenever a signal handler is
invoked
 */
int main(int argc, char const *argv[])
{
  printf("SIGINT name is %s\n SIGTERM name is %s\n", sys_siglist[SIGINT], sys_siglist[SIGTERM]);
  printf("!!!!!sigprocmask 操作的是进程的信号掩码(实际上也是线程，当前进程的主线程)，如果对于线程，需要使用 pthread_sigmask\n");
  printf("process id is %d\n", getpid());
  printf("NSIG is %d\n", NSIG);
#ifdef SIGRTMIN
  printf("SIGRTMIN = %d\n", SIGRTMIN);
#endif
#ifdef SIGRTMAX
  printf("SIGRTMAX = %d\n", SIGRTMAX);
#endif
  // 使用 sigaction 替代 signal，提供更可靠的信号处理
  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sa.sa_flags = 0; // 默认行为
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1)
  {
    printf("register SIGINT error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1)
  {
    printf("register SIGTERM error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  sigemptyset(&old_sig_mask);
  if (sigprocmask(0, NULL, &old_sig_mask) == -1)
  {
    printf("Invoke sigprocmask function error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (sigemptyset(&new_sig_mask) == -1)
  {
    printf("Invoke sigemptyset error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (sigaddset(&new_sig_mask, SIGINT) == -1)
  {
    printf("Invoke sigaddset error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (sigprocmask(SIG_BLOCK, &new_sig_mask, &old_sig_mask) == -1)
  {
    printf("Invoke sigprocmask block SIGINT error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  for (;;)
  {
    pause();
    if (unblock_sig_int)
    {
      sigset_t pending_sig_set;
      if (sigpending(&pending_sig_set) == -1)
      {
        printf("Invoke sigpending error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (sigismember(&pending_sig_set, SIGINT) == 1)
      {
        printf("SIGINT is pending\n");
        // 重置信号掩码，解除 SIGINT 阻塞
        if (sigprocmask(SIG_SETMASK, &old_sig_mask, NULL) == -1)
        {
          printf("reset process signal mask error: %s\n", strerror(errno));
          exit(EXIT_FAILURE);
        }
        printf("Signal mask reset, pending SIGINT should be delivered now\n");
      }
      else
      {
        printf("SIGINT is not pending\n");
      }
    }
  }
  return 0;
}

void handle_signal(int sig_num)
{
  if (sig_num == SIGINT)
  {
    printf("SIGINT is received and handled\n");
  }
  else if (sig_num == SIGTERM)
  {
    printf("SIGTERM is received\n");
    printf("在接收 SIGTERM 之后解开 SIGINT\n");
    unblock_sig_int = 1;
    // 下面这段代码是没有效果的，
    /**
     * If and when the
      signal-catching function returns, the signal mask of the process is reset to its previous
      value. This way, we are able to block certain signals whenever a signal handler is
      invoked
     */
    // sigset_t pending_sig_set;
    //   if (sigpending(&pending_sig_set) == -1)
    //   {
    //     printf("Invoke sigpending error: %s\n", strerror(errno));
    //     exit(EXIT_FAILURE);
    //   }
    //   if (sigismember(&pending_sig_set, SIGINT) == 1)
    //   {
    //     printf("SIGINT is pending\n");
    //     // 重置信号掩码，解除 SIGINT 阻塞
    //     if (sigprocmask(SIG_SETMASK, &old_sig_mask, NULL) == -1)
    //     {
    //       printf("reset process signal mask error: %s\n", strerror(errno));
    //       exit(EXIT_FAILURE);
    //     }
    //     printf("Signal mask reset, pending SIGINT should be delivered now\n");
    //   }
    //   else
    //   {
    //     printf("SIGINT is not pending\n");
    //   }
  }
}
