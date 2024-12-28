#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ctype.h> // Added for isdigit()
#include <linux/init_task.h> // Added for init_task


asmlinkage long sys_hello(void) {
 printk("Hello, World!\n");
 return 0;
}

/**
 * sys_set_sec - System call to set security levels.
 * @s: Security level for 's' (0 or 1).
 * @m: Security level for 'm' (0 or 1).
 * @c: Security level for 'c' (0 or 1).
 *
 * This function sets the security levels for the current process.
 * The security levels are combined into a single value and stored
 * in the 'clr' field of the current process structure to minimize task_struct size.
 *
 * Return: 
 * -  0 on success,
 * -  -EINVAL if any of the parameters are negative,
 * -  -EPERM if the calling process does not have the CAP_SYS_ADMIN capability.
 */
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

/**
 * sys_get_sec - System call to get security level based on input character.
 * @clr: Character indicating the security level to check ('c', 'm', or 's').
 *
 * This system call checks the security level of the current process based on
 * the input character:
 * - 'c': Checks if the security level is "clamp".
 * - 'm': Checks if the security level is "midnight".
 * - 's': Checks if the security level is "sword".
 *
 * Return:
 * - 1 if the security level matches the input character.
 * - -EINVAL if the input character is invalid.
 * - -EINVAL if the @clr parameter is invalid.
 */
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


/**
 * sys_set_sec_branch - Adjust the security clearance of a process's ancestors.
 * @height: The number of levels to move up the process tree.
 * @clr: The type of security clearance to adjust ('c' for clearance, 'm' for middle, 's' for secret).
 *
 * This system call moves up the process tree from the current process and adjusts the security
 * clearance of the ancestors based on the specified type. The function will stop moving up if it 
 * reaches the root of the process tree (init process with pid 1) or if the specified height is reached.
 *
 * Return: 
 *   - The number of direct parent processes whose security clearance was adjusted.
 *   - -EINVAL if the height is less than 1 or if an invalid clearance type is specified.
 *   - -EPERM if the current process does not have the required security clearance to perform the operation.
 */
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
      return count_direct_parents_changed;
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
      return count_direct_parents_changed;
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
        if(task->pid == 1){  // We are at init (the root of the tree) return.
          return count_direct_parents_changed;
        }
      }
      return count_direct_parents_changed;
    default:
      return -EINVAL;
  } 
  return count_direct_parents_changed;
}
