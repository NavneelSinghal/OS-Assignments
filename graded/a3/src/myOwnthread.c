#include "../include/myOwnthread.h"

#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#include "../include/queue.h"

// mangling code
// TODO: attribute later on
//

#ifdef __x86_64__
/* code for 64 bit Intel arch */

/* FIXME: find the correct macros */
typedef unsigned long address_t;
#define JB_BP 1
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t ptr_mangle(address_t addr) {
    address_t ret;
    asm volatile(
        "xor    %%fs:0x30,%0\n"
        "rol    $0x11,%0\n"
        : "=g"(ret)
        : "0"(addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

/* FIXME: find the correct macros */
typedef unsigned int address_t;
#define JB_BP 3
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t ptr_mangle(address_t addr) {
    address_t ret;
    asm volatile(
        "xor    %%gs:0x18,%0\n"
        "rol    $0x9,%0\n"
        : "=g"(ret)
        : "0"(addr));
    return ret;
}

#endif
// #ifndef JB_BP /* in libc >= 3.5 they hide JB_* and mangle SP */
// #define JB_BP 3
// #define JB_SP 4
// #include <features.h> /* header file defining the GLIBC version */
// #if (__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 8)
// /* the new mangling method (as of 2.8 (or even 2.7?) */
// #warning Using the new pointer mangling method (glibc 2.8 and up)
//
// #define PTR_MANGLE(var)       \
//     asm("xorl %%gs:%c2, %0\n" \
//         "roll $9, %0"         \
//         : "=r"(var)           \
//         : "0"(var), "i"(24))
//
// #else /* the original pointer mangling method */
// #warning Using the old pointer mangling method (up to glibc 2.7)
// #define PTR_MANGLE(var) asm("xorl %%gs:%c2, %0" : "=r"(var) : "0"(var),
// "i"(24)) #endif #else /* JB_BP and JB_SP are available and no mangling needed
// */ #warning No pointer mangling is necessary with this glibc #define
// PTR_MANGLE(var) /* no-op */ #endif

#define SCHEDULER_STACK_SIZE 1000
#define DEFAULT_STACK_SIZE 1000000

// 0 reserved for main
// 1 for scheduler
// other threads start from 2
// scheduler is semantically a thread but not really a thread

tcb_t *scheduler_tcb = NULL;
tcb_t *current_tcb = NULL;

int available_tid = 2;

/*
 * assumed api of queue (circular queue)
 *      - get next
 *      - remove node (given by the address) from queue
 *      - add node to the end of the queue
 */

/*
 * assertions for the runnable queue:
 *      - no element in the queue is NULL or the queue itself is NULL
 *      - is_finished = 0
 *      - 0 <= tid < number of tid's assigned
 *      - stacksize is valid
 *      - stack_start is valid
 *      - env is valid
 *      - start_routine is valid
 *      - arg is NULL or valid
 *      - retval is NULL or valid
 *      - join_caller_id is NULL or valid and points to the correct tcb
 *      - joiner is NULL or valid and points to the correct tcb
 *      - is_main is 0 or 1
 *      - if it is is_main then join_caller_id is NULL
 */

/*
 * assertions for the waiting queue:
 *      - no element in the
 *      - no element in the queue is NULL or the queue itself is NULL
 *      - is_finished = 0
 *      - 0 <= tid < number of tid's assigned
 *      - stacksize is valid
 *      - stack_start is valid
 *      - env is valid
 *      - start_routine is valid
 *      - arg is NULL or valid
 *      - retval is NULL or valid
 *      - join_caller_id is NULL or valid and points to the correct tcb
 *      - joiner is NULL or valid and points to the correct tcb
 *      - is_main is 0 or 1
 *      - if it is is_main then join_caller_id is NULL
 *      - the function is actually inside a join call
 */

queue *runnable = NULL;
queue *waiting = NULL;

int create_called = 0;

/* TODO: see where to disable interrupts and where not to */
int disabled_interrupts = 0;

/*
 * invariant for scheduler
 *      - long jump to scheduler happens only when interrupts are disabled
 *      - whenever we return from scheduler, interrupts are enabled
 */
void scheduler() {
    int got_thread_to_run = 0;
    while (runnable->size > 0) {
        node *front_tcb_node = queue_peek(runnable);
        current_tcb = front_tcb_node->data;
        if (current_tcb->is_finished == 1) {
            // add this node to the waiting queue
            queue_push(waiting, front_tcb_node);
            // and remove it from the front of the runnable queue
            queue_erase(runnable, front_tcb_node);
        } else {
            // rotate by 1 to get the next possible runnable thread
            // to the front of the runnable queue
            got_thread_to_run = 1;
            queue_erase(runnable, front_tcb_node);
            queue_push(runnable, front_tcb_node);
            break;
        }
    }
    /* we'll always have a thread to run, for instance, main;
     * so do something while doing the first create thread where you do
     * add a function for running at exit, using the atexit function */
    disabled_interrupts = 0;
    longjmp(current_tcb->env, 1);
}

void run_thread() {
    assert(!(current_tcb->is_main));
    void *ret = current_tcb->start_routine(current_tcb->arg);
    // if not already exited, do the following here instead of the thread
    myThread_exit(ret);
}

void signal_handler(int sig) {
    /* disabling interrupts to ensure that signal handler never calls itself
     * - might be possible if the clock keeps ticking when the os context
     * switches out the main kernel thread */
    disabled_interrupts = 1;
    /* due to the invariant for scheduler, whenever we get out of the scheduler,
     * interrupts get enabled automatically */
    if (setjmp(current_tcb->env) == 0) {
        longjmp(scheduler_tcb->env, 1);
    } else {
        /* is disabled_interrupts always 0 before this?
         * well, when we first do the jump to the scheduler, the scheduler sets
         * it to 0 before going to the new thread's tcb, and whenever this
         * thread (or any other thread with the same pc) is scheduled again,
         * interrupts are already on */
        assert(disabled_interrupts == 0);
    }
}

struct sigaction *custom_handler;

int myThread_create(myThread_t *thread, const myThread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg) {
    if (create_called == 0) {
        /* here signal handler is not in place as of now */
        runnable = queue_create();
        waiting = queue_create();

        create_called = 1;

        // set up main's tcb and add it to the runnable queue
        tcb_t *main_tcb = (tcb_t *)malloc(sizeof(tcb_t));
        main_tcb->tid = 0;
        main_tcb->stack_size = 0;        // special case for main
        main_tcb->stack_start = NULL;    // special case for main
        main_tcb->start_routine = NULL;  // special case for main
        main_tcb->arg = NULL;            // special case for main
        main_tcb->retval = NULL;         // special case for main

        if (setjmp(main_tcb->env) != 0) {
            // if we are now in main coming from the scheduler
            // returning for the first time, just return from
            // the call to create thread
            return 2;  // return thread id as needed
        }

        main_tcb->join_caller_id = NULL;  // special case for main
        main_tcb->joiner = NULL;
        main_tcb->is_main = 1;
        main_tcb->is_finished = 0;

        queue_push(runnable, node_create(main_tcb));

        /* scheduler's tcb creation */
        // set up scheduler's tcb and DONT add it to the queue (keep it global)
        scheduler_tcb = (tcb_t *)malloc(sizeof(tcb_t));
        scheduler_tcb->tid = -1;
        scheduler_tcb->stack_size = SCHEDULER_STACK_SIZE;
        scheduler_tcb->stack_start =
            (void *)malloc(SCHEDULER_STACK_SIZE) + SCHEDULER_STACK_SIZE - 1;
        scheduler_tcb->start_routine = NULL;  // never used (really?)
        scheduler_tcb->arg = NULL;            // never used
        scheduler_tcb->retval = NULL;         // never used

        address_t scheduler_sp = (address_t)scheduler_tcb->stack_start;
        // ensure proper alignment, since it was originally void*
        scheduler_sp >>= 3;
        scheduler_sp <<= 3;
        // stack pointer to scheduler
        scheduler_tcb->env->__jmpbuf[JB_SP] =
            (address_t)ptr_mangle((address_t)scheduler_sp);
        // program counter for scheduler
        scheduler_tcb->env->__jmpbuf[JB_PC] =
            (address_t)ptr_mangle((address_t)scheduler_tcb->start_routine);
        // base pointer to scheduler
        scheduler_tcb->env->__jmpbuf[JB_BP] =
            (address_t)ptr_mangle((address_t)scheduler_sp);

        scheduler_tcb->join_caller_id = NULL;  // never used
        scheduler_tcb->joiner = NULL;          // never used
        scheduler_tcb->is_main = 0;            // never used
        scheduler_tcb->is_finished = 0;        // never used

        /* thread tcb creation */

        (*thread) = (tcb_t *)malloc(sizeof(tcb_t));
        (*thread)->tid = available_tid++;
        (*thread)->stack_size = *attr;
        (*thread)->stack_start = (void *)malloc(*attr) + (*attr) - 1;
        (*thread)->start_routine = start_routine;
        (*thread)->arg = arg;
        (*thread)->retval = NULL;
        address_t thread_sp = (address_t)((*thread)->stack_start);
        // ensure proper alignment, since it was originally void*
        thread_sp >>= 3;
        thread_sp <<= 3;

        // set the jump buffer

        // stack pointer to thread function; for
        // normal threads, it needs to be a wrapper
        // function for the original function
        (*thread)->env->__jmpbuf[JB_SP] =
            (address_t)ptr_mangle((address_t)thread_sp);
        // program counter for thread function
        (*thread)->env->__jmpbuf[JB_PC] =
            (address_t)ptr_mangle((address_t)start_routine);
        // base pointer for the thread function
        (*thread)->env->__jmpbuf[JB_BP] =
            (address_t)ptr_mangle((address_t)thread_sp);

        (*thread)->join_caller_id = NULL;
        (*thread)->joiner = NULL;
        (*thread)->is_main = 0;
        (*thread)->is_finished = 0;

        queue_push(runnable, node_create(*thread));

        custom_handler = (struct sigaction *)malloc(sizeof(struct sigaction));
        custom_handler->sa_handler = &signal_handler;
        custom_handler->sa_flags = SA_NOMASK;
        sigaction(SIGVTALRM, custom_handler, NULL);

        disabled_interrupts = 1;
        longjmp(scheduler_tcb->env, 1);

    } else {
        disabled_interrupts = 1;

        (*thread) = (tcb_t *)malloc(sizeof(tcb_t));
        (*thread)->tid = available_tid++;
        (*thread)->stack_size = *attr;
        (*thread)->stack_start = (void *)malloc(*attr) + *attr - 1;
        (*thread)->start_routine = start_routine;
        (*thread)->arg = arg;
        (*thread)->retval = NULL;
        address_t thread_sp = (address_t)((*thread)->stack_start);
        // ensure proper alignment, since it was originally void*
        // while (thread_sp % 8 != 0) --thread_sp;
        thread_sp >>= 3;
        thread_sp <<= 3;
        // set the jump buffer
        // stack pointer to thread function; for
        // normal threads, it needs to be a wrapper
        // function for the original function
        (*thread)->env->__jmpbuf[JB_SP] =
            (address_t)ptr_mangle((address_t)thread_sp);
        // program counter for run_thread
        (*thread)->env->__jmpbuf[JB_PC] =
            (address_t)ptr_mangle((address_t)run_thread);
        // base pointer for the thread function
        (*thread)->env->__jmpbuf[JB_BP] =
            (address_t)ptr_mangle((address_t)thread_sp);
        (*thread)->join_caller_id = NULL;
        (*thread)->joiner = NULL;
        (*thread)->is_main = 0;
        (*thread)->is_finished = 0;

        queue_push(runnable, node_create(*thread));

        disabled_interrupts = 0;

        return (*thread)->tid;
    }

    return (*thread)->tid;
}

/* disable interrupts? - probably */
void myThread_exit(void *retval) {
    current_tcb->is_finished = 1;
    current_tcb->retval = retval;
    if (current_tcb->join_caller_id != NULL) {
        // put the join caller into the ready queue
        // go back to scheduler
    } else {
        // go to the scheduler
    }
}

/* disable interrupts? - yes (same implementation as the signal handler) */
int myThread_yield(void) {

    // the following two lines are taken care of by calling the scheduler
    // put this thread into the runnable queue or something? or is this already
    // the case and ofc long jump to the next queue by jumping to the scheduler?

    // copy pasted code from the signal handler

    /* disabling interrupts to ensure that this never calls signal handler
     * - might be possible if the clock keeps ticking when the os context
     * switches out the main kernel thread */
    disabled_interrupts = 1;
    /* due to the invariant for scheduler, whenever we get out of the scheduler,
     * interrupts get enabled automatically */
    if (setjmp(current_tcb->env) == 0) {
        longjmp(scheduler_tcb->env, 1);
    } else {
        /* is disabled_interrupts always 0 before this?
         * well, when we first do the jump to the scheduler, the scheduler sets
         * it to 0 before going to the new thread's tcb, and whenever this
         * thread (or any other thread with the same pc) is scheduled again,
         * interrupts are already on */
        assert(disabled_interrupts == 0);
    }
}

/* disable interrupts? - yes */
void myThread_join(myThread_t thread, void **retval) {
    // check if is_finished
    // if not finished, put this in waiting queue and call scheduler
    // get stuff from the thread it joins if finished
    // free stuff from the thread that this thread was waiting for to join
    // return or what? sounds fine to return
}

int myThread_cancel(myThread_t thread);

/* probably completed */
int myThread_attr_init(myThread_attr_t *attr) { *attr = DEFAULT_STACK_SIZE; }

/* probably completed */
int myThread_attr_destroy(myThread_attr_t *attr) { free(attr); }

/* probably completed */
myThread_t myThread_self(void) { return current_tcb; }

