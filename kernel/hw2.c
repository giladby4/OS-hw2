#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ctype.h> // Added for isdigit()
#include <linux/cred.h> // Added for current_euid()


asmlinkage long sys_hello(void) {
 printk("Hello, World11!\n");
 return 0;
}

asmlinkage long sys_set_sec(int s, int m, int c) {
  if (!uid_eq(current_euid(), GLOBAL_ROOT_UID)) {
    return -EPERM; // Return error if not root
  }
  if (s < 0 || m < 0 || c < 0) {
    return -EINVAL;
  }
  if (s > 1) {
    s = 1;
  }
  if (m > 1) {
    m = 1;
  }
  if (c > 1) {
    c = 1;
  }
 char clr_val=s+2*m+4*c;
 struct task_struct *task = current; // Pointer to the current process
 task->clr = clr_val; 
 printk("SEC set sec %d%d%d%d!!\n",s,m,c,clr_val);
 return 0;
}

asmlinkage long sys_get_sec(char clr) {
 struct task_struct *task = current; // Pointer to the current process
 switch(clr){
  case 'c':
   return (task->clr/4)==1;
  case 'm':
   return ((task->clr%4)/2)==1;
  case 's':
   return (task->clr%2)==1;
 }
 printk("SEC get sec%c!\n",clr);
 return EINVAL;
}

asmlinkage long sys_check_sec(pid_t pid, char clr) {
 printk("SEC check sec%d%c!\n",pid,clr);
 return 0;
}


asmlinkage long sys_set_sec_branch(int height, char clr) {
  if(current->clr != clr){
    return -EPERM;
  }
  if(height < 0){
    return -EINVAL;
  }
  if (!isdigit(clr) || clr < '0' || clr > '7') {
    return -EINVAL;
  }

  struct task_struct *task;
  int i;
  int count =0;

  task = current;

  // Traverse up the process tree
  for (i = 0; i < height && task->parent != &init_task; i++) {
    task = task->parent;
    task->clr = clr;
    count++;
  }
  printk("SEC set sec branch%d%c!\n",height,clr);

  return count;
}
