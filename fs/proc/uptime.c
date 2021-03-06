#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/kernel_stat.h>
#include <linux/cputime.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/printk.h>

SYSCALL_DEFINE1(uptime, long __user *, mseconds)
{
	struct timespec ut;
	long		result;

	get_monotonic_boottime(&ut);
	pr_debug("seconds: %ld, nanoseconds: %ld\n", ut.tv_sec, ut.tv_nsec);
	result = ut.tv_nsec / NSEC_PER_MSEC;
	if (result + ut.tv_sec * MSEC_PER_SEC > LONG_MAX)
	  return -ERANGE;
	result += ut.tv_sec * MSEC_PER_SEC;
	if (copy_to_user(mseconds, &result, sizeof(result)))
	  return -EFAULT;
	pr_debug("result in milliseconds: %ld\n", *mseconds);
	return 0;
}

static int uptime_proc_show(struct seq_file *m, void *v)
{
	struct timespec uptime;
	struct timespec idle;
	u64 idletime;
	u64 nsec;
	u32 rem;
	int i;

	idletime = 0;
	for_each_possible_cpu(i)
		idletime += (__force u64) kcpustat_cpu(i).cpustat[CPUTIME_IDLE];

	get_monotonic_boottime(&uptime);
	nsec = cputime64_to_jiffies64(idletime) * TICK_NSEC;
	idle.tv_sec = div_u64_rem(nsec, NSEC_PER_SEC, &rem);
	idle.tv_nsec = rem;
	seq_printf(m, "%lu.%02lu %lu.%02lu\n",
			(unsigned long) uptime.tv_sec,
			(uptime.tv_nsec / (NSEC_PER_SEC / 100)),
			(unsigned long) idle.tv_sec,
			(idle.tv_nsec / (NSEC_PER_SEC / 100)));
	return 0;
}

static int uptime_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, uptime_proc_show, NULL);
}

static const struct file_operations uptime_proc_fops = {
	.open		= uptime_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_uptime_init(void)
{
	proc_create("uptime", 0, NULL, &uptime_proc_fops);
	return 0;
}
fs_initcall(proc_uptime_init);
