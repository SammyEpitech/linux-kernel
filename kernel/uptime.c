#include <linux/timekeeping.h>

SYSCALL_DEFINE1(uptime, long __user *, time)
{
  struct timespec ut;

  get_monotonic_boottime(&ut);
  *time = (long)ut.tv_sec;
  
  return 0;
}
