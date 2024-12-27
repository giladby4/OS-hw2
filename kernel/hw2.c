#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ctype.h> // Added for isdigit()
#include <linux/init_task.h> // Added for init_task


asmlinkage long sys_hello(void) {
 printk("Hello, World11!\n");
 return 0;
}

asmlinkage long sys_set_sec(int s, int m, int c) {
  if (s < 0 || m < 0 || c < 0) {
    return -EINVAL;
  }
  if(!capable(CAP_SYS_ADMIN)){
    return  -EPERM;
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
 printk("SEC - Set clr for running process, s= %d, m= %d,c= %d, clr_val: %d!!\n",s,m,c,clr_val);
 return 0;
}

asmlinkage long sys_get_sec(char clr) {
  struct task_struct *task = current; // Pointer to the current process
  printk("SEC - Running process CLR is %c ?\n",task->clr);
  switch(clr){
    case 'c':
      return (task->clr/4)==1;
    case 'm':
      return ((task->clr%4)/2)==1;
    case 's':
      return (task->clr%2)==1;
    default:
      return -EINVAL;
  } 
}

asmlinkage long sys_check_sec(pid_t pid, char clr) {

  if(clr != 'm' && clr != 'c' && clr != 's'){
    return -EINVAL;               // No such clearance
  }

  struct task_struct *task;
  task = find_task_by_vpid(pid);  // Find the task_struct for the given pid
  if (!task) {
    return -ESRCH;                // No such process
  }

  if(current->clr != clr){
    return -EPERM;                // The calling process does not have “clr” clearance. 

  }

  printk("SEC - Check if pid: %d have clr: %c!\n",pid,clr);
  return task->clr == clr;
}


asmlinkage long sys_set_sec_branch(int height, char clr) {

  if(height < 0 || (clr != 'm' && clr != 'c' && clr != 's') ){
    return -EINVAL;
  }

  if(current->clr != clr){
    return -EPERM;
  }

  struct task_struct *task;
  int i;
  int count_direct_parents_changed =0;

  task = current;

  // Traverse up the process tree
  for (i = 0; i < height; i++) {
    task = task->parent;
    task->clr = clr;
    count_direct_parents_changed++;
    if(task == task->parent){  // We are at the init process so we can exit the traverse
      return count_direct_parents_changed;
    }
  }
  printk("SEC - Set branch with limit: %d, and clr: %c \n",height,clr);

  return count_direct_parents_changed;
}
