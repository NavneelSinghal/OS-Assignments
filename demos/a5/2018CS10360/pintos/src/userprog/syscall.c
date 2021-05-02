#include "userprog/syscall.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/exception.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "vm/frame.h"
#include "vm/sup_page.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>

struct create_args
{
  int id;
  const char *file;
  unsigned initial_size;
};

struct remove_args
{
  int id;
  const char *file;
};

struct open_args
{
  int id;
  const char *file;
};

struct close_args
{
  int id;
  int fd;
};

struct read_args
{
  int id;
  int fd;
  void *buffer;
  unsigned length;
};

struct write_args
{
  int id;
  int fd;
  const void *buffer;
  unsigned length;
};

struct seek_args
{
  int id;
  int fd;
  unsigned position;
};

struct filesize_args
{
  int id;
  int fd;
};

struct exec_args
{
  int id;
  const char *cmd_line;
};

struct exit_args
{
  int id;
  int status;
};

struct wait_args
{
  int id;
  tid_t child;
};

static bool sys_create (struct create_args *args);
static bool sys_remove (struct remove_args *args);
static int sys_open (struct open_args *args);
static void sys_close (struct close_args *args);
static int sys_read (struct read_args *args);
static int sys_write (struct write_args *args);
static void sys_seek (struct seek_args *args);
static int sys_filesize (struct filesize_args *args);
static void sys_halt (void);
static tid_t sys_exec (struct exec_args *args);
void sys_exit (struct exit_args *args);
static int sys_wait (struct wait_args *args);

static void exit_thread_cleanly (void);

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Returns true if the user address sequence is in valid range or false
   otherwise. If exact is true, the whole range is checked, otherwise this can
   be used to check for validity of strings - it only looks up to end of string
   \0.
 */
bool validate_user_addr_range (uint8_t *va, size_t bcnt, uint32_t *esp,
                               bool exact);

/* Uses the second technique mentioned in pintos doc. 3.1.5
   to cause page faults and check addresses (returns -1 on fault) */

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm("movl $1f, %0; movzbl %1, %0; 1:" : "=&a"(result) : "m"(*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
/*
static bool
put_user (uint8_t *udst, uint8_t byte) {
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:" : "=&a" (error_code), "=m" (*udst) :
"q" (byte)); return error_code != -1;
}
*/
/* Used to validate pointers, buffers and strings.
   With exact false, it validates until 0 (end of string). */
bool
validate_user_addr_range (uint8_t *va, size_t bcnt, uint32_t *esp, bool exact)
{
  if (va == NULL) /* NULL is not allowed */
    return false;
  for (size_t i = 0; (i < bcnt) || !exact; i++)
    {
      if (!is_user_vaddr (va + i)) /* outside user space! wrong */
        return false;
      int w = get_user (va + i);
      if (!exact && w == 0) /* end of string */
        return true;
      if (w == -1)
        { /* outside mapped pages */
#ifdef VM
          uint8_t *uaddr = PFNO_TO_ADDR (ADDR_TO_PFNO (va + i));
          struct sup_page *spg = lookup_page (uaddr);
          if (spg != NULL && load_page (spg, uaddr)) /* page must be loaded */
            continue;                                /* check next address */
          if (va + i > (uint8_t *)esp
              && grow_stack (uaddr)) /* 1st stack access in syscall */
            continue;                /* check next address*/
                                     /* none of these situations! */
#endif
          return false;
        }
    }
  return true;
}

/* File system primitive synchronization. Sequentialize file system accesses.
 */
#define FS_ATOMIC(code)                                                       \
  {                                                                           \
    fs_take ();                                                               \
    { code } fs_give ();                                                      \
  }

// ptr size
static void
syscall_handler (struct intr_frame *f)
{
  /* validate 4 bytes - the size of an int */
  if (!validate_user_addr_range (f->esp, 4, f->esp, true))
    exit_thread_cleanly ();
  struct create_args *cr_args;
  struct remove_args *rm_args;
  struct open_args *op_args;
  struct close_args *cl_args;
  struct read_args *rd_args;
  struct write_args *wr_args;
  struct seek_args *sk_args;
  struct filesize_args *fs_args;
  struct exec_args *xc_args;
  struct exit_args *xt_args;
  struct wait_args *wt_args;

  switch (*((uint32_t *)(f->esp)))
    {
    case SYS_CREATE:
      cr_args = (struct create_args *)(f->esp);
      if (!validate_user_addr_range (cr_args->file, 0, f->esp, false)
          || !validate_user_addr_range (f->esp, 12, f->esp, true))
        exit_thread_cleanly ();
      f->eax = sys_create (cr_args);
      break;
    case SYS_REMOVE:
      rm_args = (struct remove_args *)(f->esp);
      if (!validate_user_addr_range (rm_args->file, 0, f->esp, false)
          || !validate_user_addr_range (f->esp, 8, f->esp, true))
        exit_thread_cleanly ();
      f->eax = sys_remove (rm_args);
      break;
    case SYS_OPEN:
      op_args = (struct open_args *)(f->esp);
      if (!validate_user_addr_range (op_args->file, 0, f->esp, false)
          || !validate_user_addr_range (f->esp, 8, f->esp, true))
        exit_thread_cleanly ();
      f->eax = sys_open (op_args);
      break;
    case SYS_CLOSE:
      if (!validate_user_addr_range (f->esp, 8, f->esp, true))
        exit_thread_cleanly ();
      cl_args = (struct close_args *)(f->esp);
      sys_close (cl_args);
      break;
    case SYS_READ:
      rd_args = (struct read_args *)(f->esp);
      if (!validate_user_addr_range (rd_args->buffer, rd_args->length, f->esp,
                                     true)
          || !validate_user_addr_range (f->esp, 16, f->esp, true))
        exit_thread_cleanly ();
      f->eax = sys_read (rd_args);
      break;
    case SYS_WRITE:
      wr_args = (struct write_args *)(f->esp);
      if (!validate_user_addr_range (wr_args->buffer, wr_args->length, f->esp,
                                     true)
          || !validate_user_addr_range (f->esp, 16, f->esp, true))
        exit_thread_cleanly ();
      f->eax = sys_write (wr_args);
      break;
    case SYS_SEEK:
      sk_args = (struct seek_args *)(f->esp);
      if (!validate_user_addr_range (f->esp, 12, f->esp, true))
        exit_thread_cleanly ();
      sys_seek (sk_args);
      break;
    case SYS_FILESIZE:
      fs_args = (struct filesize_args *)(f->esp);
      if (!validate_user_addr_range (f->esp, 8, f->esp, true))
        exit_thread_cleanly ();
      f->eax = sys_filesize (fs_args);
      break;
    case SYS_HALT:
      if (!validate_user_addr_range (f->esp, 4, f->esp, true))
        exit_thread_cleanly ();
      sys_halt ();
      break;
    case SYS_EXEC:
      xc_args = (struct exec_args *)(f->esp);
      if (!validate_user_addr_range (xc_args->cmd_line, 0, f->esp, false)
          || (!validate_user_addr_range (f->esp, 8, f->esp, true)))
        exit_thread_cleanly ();
      f->eax = sys_exec (xc_args);
      break;
    case SYS_EXIT:
      if (!validate_user_addr_range (f->esp, 8, f->esp, true))
        exit_thread_cleanly ();
      xt_args = (struct exit_args *)(f->esp);
      sys_exit (xt_args);
      break;
    case SYS_WAIT:
      if (!validate_user_addr_range (f->esp, 8, f->esp, true))
        exit_thread_cleanly ();
      wt_args = (struct wait_args *)(f->esp);
      f->eax = sys_wait (wt_args);
      break;
    default:
      printf ("This system call is not implemented.\n");
      exit_thread_cleanly ();
    }
}

/* syscall to create a file with a given name and size */
static bool
sys_create (struct create_args *args)
{
  bool created;
  FS_ATOMIC (created = filesys_create (args->file, args->initial_size););
  return created;
}

/* syscall to delete a file with a given name */
static bool
sys_remove (struct remove_args *args)
{
  bool removed;
  FS_ATOMIC (removed = filesys_remove (args->file););
  return removed;
}

/* syscall to return a file handle to a file identified by its name */
static int
sys_open (struct open_args *args)
{
  int fd = -1;
  FS_ATOMIC (struct file *f = filesys_open (args->file); if (f != NULL) {
    for (int i = 2; i < 256; ++i)
      if (thread_current ()->fileptr[i] == NULL)
        {
          thread_current ()->fileptr[i] = f;
          fd = i;
          break;
        }
  });
  return fd;
}

/* syscall to close a file identified by a file handle */
static void
sys_close (struct close_args *args)
{
  if (args->fd < 0 || args->fd >= 256)
    return;
  FS_ATOMIC (thread_current ()->fileptr[args->fd] = NULL;);
}

/* syscall to read a requested number of bytes from a file identified by a
 * handle*/
static int
sys_read (struct read_args *args)
{
  char *ptr = args->buffer;
  if (args->fd == STDOUT_FILENO)
    {
      return -1;
    }
  else if (args->fd == STDIN_FILENO)
    {
      unsigned read_chars = 0;
      while (read_chars < args->length)
        {
          char c = input_getc ();
          if (c == 0)
            break;
          *ptr = c;
          ++read_chars;
        }
      return (int)read_chars;
    }
  else
    {
      int read_chars = 0;
      if (args->fd < 0 || args->fd >= 256)
        {
          read_chars = -1;
        }
      else
        {
          FS_ATOMIC (struct file *f = thread_current ()->fileptr[args->fd];
                     if (f == NULL) read_chars = -1;
                     else { read_chars = file_read (f, ptr, args->length); });
        }
      return read_chars;
    }
}

/* syscall to write a requested amount of bytes from a file identified by a
 * handle */
static int
sys_write (struct write_args *args)
{
  int written = 0;
  char *ptr = args->buffer;
  if (args->fd == STDOUT_FILENO)
    {
      for (unsigned i = 0; i < args->length; ++i)
        {
          printf ("%c", *(ptr + i));
          written++;
        }
    }
  else if (args->fd == STDIN_FILENO)
    {
      written = -1;
    }
  else
    {
      if (args->fd < 0 || args->fd >= 256)
        {
          written = -1;
        }
      else
        {
          FS_ATOMIC (struct file *f = thread_current ()->fileptr[args->fd];
                     if (f == NULL) written = -1;
                     else { written = file_write (f, ptr, args->length); });
        }
    };
  return written;
}

/* syscall to move the cursor to a given position inside a file */
static void
sys_seek (struct seek_args *args)
{
  if (args->fd < 2 || args->fd >= 256)
    {
      return;
    }
  FS_ATOMIC (struct file *f = thread_current ()->fileptr[args->fd];
             if (f != NULL) { file_seek (f, args->position); });
}

/* syscall to return the size of a file in bytes */
static int
sys_filesize (struct filesize_args *args)
{
  if (args->fd < 2 || args->fd >= 256)
    return -1;
  int file_size = -1;
  FS_ATOMIC (struct file *f = thread_current ()->fileptr[args->fd];
             if (f != NULL) { file_size = file_length (f); });
  return file_size;
}

/* syscall to power off the system */
static void
sys_halt (void)
{
  shutdown_power_off ();
  NOT_REACHED ();
}

/* syscall to run the executable */
static tid_t
sys_exec (struct exec_args *args)
{
  tid_t child = -1;
  FS_ATOMIC (child = process_execute (args->cmd_line););
  return child;
}

/* syscall to exit the process */
void
sys_exit (struct exit_args *args)
{
  printf ("%s: exit(%d)\n", thread_current ()->name, args->status);
  struct proc_info *proc = thread_current ()->proc;
  if (proc != NULL)
    proc->ret = args->status;
  thread_exit ();
  NOT_REACHED ();
}

/* syscall to wait for child to exit */
static int
sys_wait (struct wait_args *args)
{
  return process_wait (args->child);
}

void
sys_exit1 (int s)
{
  struct exit_args e;
  e.status = s;
  sys_exit (&e);
}

static void
exit_thread_cleanly (void)
{
  sys_exit1 (-1);
  NOT_REACHED ();
}

