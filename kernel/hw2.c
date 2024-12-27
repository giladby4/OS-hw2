#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ctype.h> // Added for isdigit()
#include <linux/init_task.h> // Added for init_task


asmlinkage long sys_hello(void) {
 printk("Hello, World!\n");
 return 0;
}

asmlinkage long sys_set_sec(int s, int m, int c) {
  printk("SEC_set: before change clr: %d\n",(int)current->clr);
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
  current->clr = clr_val; 
  return 0;
}

asmlinkage long sys_get_sec(char clr) {
  switch(clr){
    case 'c':
      return (current->clr/4)==1;
    case 'm':
      return ((current->clr%4)/2)==1;
    case 's':
      return (current->clr%2)==1;
    default:
      return -EINVAL;
  } 
}

asmlinkage long sys_check_sec(pid_t pid, char clr) {
  struct task_struct *target_task = find_task_by_vpid(pid);
  switch(clr){
    case 'c':
      if (!target_task) {
        return -ESRCH;                // No such process
      }
      else if(current->clr/4!=1){
        return -EPERM;
      }
      else{
        return (target_task->clr/4) == 1;
      }
    case 'm':
      if (!target_task) {
        return -ESRCH;                // No such process
      }
      else if(((current->clr%4)/2)!=1){
        return -EPERM;
      }
      else{
        return ((target_task->clr%4)/2) == 1;
      }
    case 's':
      if (!target_task) {
        return -ESRCH;                // No such process
      }
      else if((current->clr%2)!=1){
        return -EPERM;
      }
      else{
        return (target_task->clr%2)==1;
      }
    default:
      return -EINVAL;                 //Invalid argument
  } 
}


asmlinkage long sys_set_sec_branch(int height, char clr) {
  struct task_struct *task = current;
  if(height < 1){
    return -EINVAL;
  }

  int i;
  int count_direct_parents_changed =0;

  switch(clr){
    case 'c':
      if(!(task->clr/4)){
        return -EPERM;
      }

      // Traverse up the process tree
      for (i = 0; i < height; i++) {
        task = task->parent;
        if((task->clr/4)!=1){
          task->clr = task->clr + 4;
          count_direct_parents_changed++;
        }
        if(task->pid == 1){  // We are at init (the root of the tree)
          return count_direct_parents_changed;
        }
      }
      
    case 'm':
      if(!((task->clr%4)/2)){
        return -EPERM;
      }
      // Traverse up the process tree
      for (i = 0; i < height; i++) {
        task = task->parent;
        if(((task->clr%4)/2)!=1){
          task->clr = task->clr + 2;
          count_direct_parents_changed++;
        }
        if(task->pid == 1){  // We are at init (the root of the tree)
          return count_direct_parents_changed;
        }
      }
    case 's':
      if(!(task->clr%2)){
        return -EPERM;
      }
      for (i = 0; i < height; i++) {
        task = task->parent;
        if((task->clr%2)!=1){
          task->clr = task->clr + 1;
          count_direct_parents_changed++;
        }
        if(task->pid == 1){  // We are at init (the root of the tree)
          return count_direct_parents_changed;
        }
      }
      
    default:
      return -EINVAL;
  } 
  return count_direct_parents_changed;
}
