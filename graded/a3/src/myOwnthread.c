#include "../include/myOwnthread.h"

#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../include/queue.h"

#define NODEBUG

#ifdef NODEBUG

#define debug_print(...)
#define debug_print_queue(...)

#else

#include <stdarg.h>
#include <stdio.h>
void debug_print(const char *format, ...) {
    va_list args;
    /* fprintf(stderr, "Log:\n"); */
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stdout);
}

void debug_print_queue(queue *q) {
    node *n = q->head;
    while (n != NULL) {
        debug_print("%d -> ", ((tcb_t *)n->data)->tid);
        n = n->nxt;
    }
    debug_print("NULL\n");
}

#endif

#ifdef __x86_64__

#define JB_BP 1
#define JB_SP 6
#define JB_PC 7

/* mangling pointers to make it compatible with jmp_buf stuff */
unsigned long ptr_mangle(unsigned long addr) {
    unsigned long ret;
    asm volatile(
        "xor %%fs:0x30, %0\n"
        "rol $0x11, %0\n"
        : "=g"(ret)
        : "0"(addr));
    return ret;
}

#else

#define JB_BP 3
#define JB_SP 4
#define JB_PC 5
/* mangling pointers to make it compatible with jmp_buf stuff */
unsigned long ptr_mangle(unsigned long addr) {
    unsigned long ret;
    asm volatile(
        "xor %%gs:0x18, %0\n"
        "rol $0x9, %0\n"
        : "=g"(ret)
        : "0"(addr));
    return ret;
}

#endif

/* define enough stack size for the scheduler to run without any issues */
#define SCHEDULER_STACK_SIZE 100000
#define DEFAULT_STACK_SIZE 1000000
#define INTERVAL_MS 50

/* thread id conventions:
 *      - 0 reserved for main
 *      - 1 reserved for scheduler (for debugging)
 *      - threads start from 2
 * the scheduler is semantically the same as a thread, however not really a
 * thread
 * */

static tcb_t *main_tcb = NULL;
static tcb_t *scheduler_tcb = NULL;
static tcb_t *current_tcb = NULL;

static int available_tid = 2;

static queue *runnable = NULL;
static queue *waiting = NULL;

static int create_called = 0;
static int first_jump = 0;
static int disabled_interrupts = 0;
static struct sigaction *custom_handler;

static struct itimerval timer;

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
 *      - is_finished = 0 or 1
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

/********************************
 * Thread-related functionality *
 ********************************/

/*
 * invariant for scheduler
 *      - long jump to scheduler happens only when interrupts are disabled
 *      - whenever we return from scheduler, interrupts are enabled
 */
void scheduler() {
    debug_print("reached the scheduler");
    int got_thread_to_run = 0;
    if (!first_jump) {
        first_jump = 1;
        debug_print(
            "doing the first jump to main to continue creating the thread");
        current_tcb = queue_peek(runnable)->data;
        debug_print("longjmp to current_tcb->env");
        debug_print("queue before longjmp");
        debug_print_queue(runnable);
        disabled_interrupts = 0;
        longjmp(current_tcb->env, 1);
    } else {
        while (runnable->size > 0) {
            debug_print_queue(runnable);
            debug_print("size of runnable is: %d", runnable->size);
            node *front_tcb_node = queue_peek(runnable);
            current_tcb = front_tcb_node->data;
            debug_print("id of the thread picked up from runnable: %d",
                        current_tcb->tid);
            if (current_tcb->is_finished == 1) {
                /* remove it from the front of the runnable queue */
                queue_erase(runnable, current_tcb);
                /* add this node to the waiting queue */
                queue_push(waiting, front_tcb_node);
            } else {
                /* rotate by 1 to get the next possible runnable thread
                 * to the front of the runnable queue */
                got_thread_to_run = 1;
                debug_print("got a thread to run");
                debug_print("id of the thread to be run: %d", current_tcb->tid);
                queue_erase(runnable, current_tcb);
                queue_push(runnable, front_tcb_node);
                break;
            }
        }
        /* we'll always have a thread to run, for instance, main;
         * so do something while doing the first create thread where you do
         * add a function for running at exit, using the atexit function */
        assert(got_thread_to_run == 1);
        disabled_interrupts = 0;
        debug_print("longjmp to current_tcb->env");
        debug_print("queue before longjmp");
        debug_print_queue(runnable);
        longjmp(current_tcb->env, 1);
    }
}

/*
 * wrapper function to jump to when executing a thread
 */
void run_thread() {
    assert(!(current_tcb->is_main));
    void *ret = current_tcb->start_routine(current_tcb->arg);
    /* if not already exited, do the following here instead of the thread */
    myThread_exit(ret);
}

/*
 * function for clearing up a thread
 */
void delete_thread(myThread_t thread) {
    free(thread->stack_end);
    free(thread);
}

/*
 * exit handler in case the thread system has been initialized
 */
void final_cleanup() {
    /* delete threads and stuff */
    /* delete all stuff in waiting */
    while (waiting->size > 0) {
        node *n = queue_erase(waiting, queue_peek(waiting)->data);
        /* free(n->data);
         * assumed to be freed when we free the thread during join */
        free(n);
    }
    free(waiting);

    assert(runnable->size == 1);
    while (runnable->size > 0) {
        node *n = queue_erase(runnable, queue_peek(runnable)->data);
        free(n->data);
        free(n);
    }
    /* free(queue_peek(runnable)->data);  -- this is main */
    free(runnable); /* at exit, this has only main */
    delete_thread(scheduler_tcb);
    free(custom_handler);
}

/*
 * signal handler for SIGVTALRM
 */
void signal_handler(int sig) {
    if (disabled_interrupts) return;

    /* disabling interrupts to ensure that signal handler never calls itself
     * - might be possible if the clock keeps ticking when the os context
     * switches out the main kernel thread */
    disabled_interrupts = 1;
    /* due to the invariant for scheduler, whenever we get out of the scheduler,
     * interrupts get enabled automatically */
    debug_print("inside signal handler");

    if (setjmp(current_tcb->env) == 0) {
        debug_print("jumping to scheduler from inside signal handler");
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

/************************
 * Thread API functions *
 ************************/

int myThread_create(myThread_t *thread, const myThread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg) {
    debug_print("inside myThread_create");

    if (create_called == 0) {
        /* here signal handler is not in place as of now */
        create_called = 1;

        debug_print("myThread_create called for the first time");

        runnable = queue_create();
        waiting = queue_create();

        debug_print("runnable and waiting queues created");
        debug_print("size of runnable is %d", runnable->size);
        debug_print("size of waiting is %d", waiting->size);

        atexit(final_cleanup);

        debug_print("allocating main_tcb...");

        /* set up main's tcb and add it to the runnable queue */
        main_tcb = (tcb_t *)malloc(sizeof(tcb_t));
        main_tcb->tid = 0;
        main_tcb->stack_size = 0;       /* special case for main */
        main_tcb->stack_start = NULL;   /* special case for main */
        main_tcb->stack_end = NULL;     /* special case for main */
        main_tcb->start_routine = NULL; /* special case for main */
        main_tcb->arg = NULL;           /* special case for main */
        main_tcb->retval = NULL;        /* special case for main */

        if (setjmp(main_tcb->env) != 0) {
            /* if we are now in main coming from the scheduler
             * doing a longjmp for the first time, just return
             * from the call to create thread */
            debug_print("returned from scheduler to main for the first time");
            return 2; /* return thread id as needed */
        }

        main_tcb->join_caller_id = NULL; /* special case for main */
        main_tcb->joiner = NULL;
        main_tcb->is_main = 1;
        main_tcb->is_finished = 0;

        debug_print("allocating main_tcb completed");

        debug_print("size of runnable is %d", runnable->size);
        debug_print("size of waiting is %d", waiting->size);
        debug_print("allocating scheduler_tcb...");

        /*
         * scheduler's tcb creation
         */

        /* set up scheduler's tcb and don't add it to the queue (keep it global)
         */
        scheduler_tcb = (tcb_t *)malloc(sizeof(tcb_t));
        scheduler_tcb->tid = -1;
        scheduler_tcb->stack_size = SCHEDULER_STACK_SIZE;
        scheduler_tcb->stack_start =
            (void *)malloc(SCHEDULER_STACK_SIZE) + SCHEDULER_STACK_SIZE - 1;
        scheduler_tcb->stack_end =
            scheduler_tcb->stack_start - SCHEDULER_STACK_SIZE + 1;
        scheduler_tcb->start_routine = NULL; /* never used */
        scheduler_tcb->arg = NULL;           /* never used */
        scheduler_tcb->retval = NULL;        /* never used */

        unsigned long scheduler_sp =
            (unsigned long)(scheduler_tcb->stack_start);
        /* ensure proper alignment, since it was originally void* */
        scheduler_sp >>= 3;
        scheduler_sp <<= 3;
        /* stack pointer to scheduler */
        scheduler_tcb->env->__jmpbuf[JB_SP] =
            (unsigned long)ptr_mangle((unsigned long)scheduler_sp);
        /* program counter for scheduler */
        scheduler_tcb->env->__jmpbuf[JB_PC] =
            (unsigned long)ptr_mangle((unsigned long)scheduler);
        /* base pointer to scheduler */
        scheduler_tcb->env->__jmpbuf[JB_BP] =
            (unsigned long)ptr_mangle((unsigned long)scheduler_sp);

        scheduler_tcb->join_caller_id = NULL; /* never used */
        scheduler_tcb->joiner = NULL;         /* never used */
        scheduler_tcb->is_main = 0;           /* never used */
        scheduler_tcb->is_finished = 0;       /* never used */

        debug_print("allocating scheduler_tcb completed");

        /*
         * thread tcb creation
         */

        debug_print("allocating thread_tcb...");

        int stack_size = DEFAULT_STACK_SIZE;
        if (attr != NULL) stack_size = *attr;

        (*thread) = (tcb_t *)malloc(sizeof(tcb_t));
        (*thread)->tid = available_tid++;
        (*thread)->stack_size = *attr;
        (*thread)->stack_start = (void *)malloc(stack_size) + (stack_size)-1;
        (*thread)->stack_end = (*thread)->stack_start - (stack_size) + 1;
        (*thread)->start_routine = start_routine;
        (*thread)->arg = arg;
        (*thread)->retval = NULL;
        unsigned long thread_sp = (unsigned long)((*thread)->stack_start);
        /* ensure proper alignment, since it was originally void* */
        thread_sp >>= 3;
        thread_sp <<= 3;

        /* set the jump buffer */

        /* stack pointer to thread function; for
         * normal threads, it needs to be a wrapper
         * function for the original function */

        /* stack pointer to thread stack */
        (*thread)->env->__jmpbuf[JB_SP] =
            (unsigned long)ptr_mangle((unsigned long)thread_sp);
        /* program counter for thread function */
        (*thread)->env->__jmpbuf[JB_PC] =
            (unsigned long)ptr_mangle((unsigned long)run_thread);
        /* base pointer for the thread function */
        (*thread)->env->__jmpbuf[JB_BP] =
            (unsigned long)ptr_mangle((unsigned long)thread_sp);

        (*thread)->join_caller_id = NULL;
        (*thread)->joiner = NULL;
        (*thread)->is_main = 0;
        (*thread)->is_finished = 0;

        debug_print("allocating thread_tcb completed");

        debug_print("adding main_tcb to runnable");
        queue_push(runnable, node_create(main_tcb));

        debug_print("adding thread_tcb to the runnable queue");
        queue_push(runnable, node_create(*thread));

        debug_print("size of runnable is %d", runnable->size);
        debug_print("size of waiting is %d", waiting->size);
        debug_print("setting up signal handler");

        /* setting the signal handler */

        custom_handler = (struct sigaction *)malloc(sizeof(struct sigaction));
        custom_handler->sa_handler = &signal_handler;
        custom_handler->sa_flags = SA_NODEFER;

        /* setting the timer */

        timer.it_value.tv_sec = INTERVAL_MS / 1000;
        timer.it_value.tv_usec = (INTERVAL_MS * 1000) % 1000000;
        timer.it_interval = timer.it_value;
        setitimer(ITIMER_VIRTUAL, &timer, NULL);

        /* note that SIGVTALRM is used instead of SIGALRM because
         * it seems more natural for the timer to get ticks only
         * when the process is working, and not otherwise */
        sigaction(SIGVTALRM, custom_handler, NULL);

        /* disable interrupts to maintain scheduler invariant */

        disabled_interrupts = 1;

        debug_print("signal handler set up completed");

        debug_print("longjmp to scheduler");

        longjmp(scheduler_tcb->env, 1);

    } else {
        disabled_interrupts = 1;

        debug_print("disabled interrupts and allocating thread_tcb...");

        int stack_size = DEFAULT_STACK_SIZE;
        if (attr != NULL) stack_size = *attr;

        (*thread) = (tcb_t *)malloc(sizeof(tcb_t));
        (*thread)->tid = available_tid++;
        (*thread)->stack_size = stack_size;
        (*thread)->stack_start = (void *)malloc(stack_size) + (stack_size)-1;
        (*thread)->stack_end = (*thread)->stack_start - (stack_size) + 1;
        (*thread)->start_routine = start_routine;
        (*thread)->arg = arg;
        (*thread)->retval = NULL;
        unsigned long thread_sp = (unsigned long)((*thread)->stack_start);
        /* ensure proper alignment, since it was originally void* */
        thread_sp >>= 3;
        thread_sp <<= 3;

        /* set the jump buffer */

        /* stack pointer to thread function; for
         * normal threads, it needs to be a wrapper
         * function for the original function */

        /* stack pointer to thread stack */
        (*thread)->env->__jmpbuf[JB_SP] =
            (unsigned long)ptr_mangle((unsigned long)thread_sp);
        /* program counter for run_thread */
        (*thread)->env->__jmpbuf[JB_PC] =
            (unsigned long)ptr_mangle((unsigned long)run_thread);
        /* base pointer for the thread function */
        (*thread)->env->__jmpbuf[JB_BP] =
            (unsigned long)ptr_mangle((unsigned long)thread_sp);
        (*thread)->join_caller_id = NULL;
        (*thread)->joiner = NULL;
        (*thread)->is_main = 0;
        (*thread)->is_finished = 0;

        debug_print("allocating thread_tcb completed");

        debug_print("adding thread_tcb to the runnable queue");

        queue_push(runnable, node_create(*thread));

        debug_print("enabling interrupts");

        disabled_interrupts = 0;

        return (*thread)->tid;
    }

    debug_print("returning thread id to the thread-creating function");

    return (*thread)->tid;
}

/* disable interrupts? - probably */
void myThread_exit(void *retval) {
    disabled_interrupts = 1;
    current_tcb->is_finished = 1;
    current_tcb->retval = retval;
    /* put this thread into the waiting queue (but it's finished; however,
     * this is valid since the waiting queue is basically a buffer) */
    node *current_tcb_node = queue_erase(runnable, current_tcb);
    queue_push(waiting, current_tcb_node);
    if (current_tcb->join_caller_id != NULL) {
        /* put the join caller into the runnable queue */
        node *join_caller = queue_erase(waiting, current_tcb->join_caller_id);
        queue_push(runnable, join_caller);
        current_tcb->join_caller_id = NULL;
    }
    /* go back to scheduler */
    longjmp(scheduler_tcb->env, 1);
}

/* disable interrupts? - yes (same implementation as the signal handler) */
int myThread_yield(void) {
    /* the following two lines are taken care of by calling the scheduler
     * put this thread into the runnable queue or something? or is this already
     * the case and ofc long jump to the next queue by jumping to the scheduler?
     * */

    /* same code as the signal handler follows */

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
    return 0;
}

/* disable interrupts? - yes */
void myThread_join(myThread_t thread, void **retval) {
    /* check if is_finished
     * if not finished, put this in waiting queue and call scheduler
     * do we need to do a setjmp for the current tho? probably yes since this
     * behaviour is like yield but it goes into the waiting queue instead get
     * stuff from the thread it joins if finished. free stuff from the thread
     * that this thread was waiting for to join return or what? sounds fine to
     * return */

    debug_print("inside join now in thread %d, disabling interrupts...",
                current_tcb->tid);

    disabled_interrupts = 1;
    current_tcb->joiner = thread;
    thread->join_caller_id = current_tcb;

    if (thread->is_finished) {
        goto acquire_ret_val_and_free_stuff;
    } else {
        debug_print("thread hasn't finished, going to sleep");
        if (setjmp(current_tcb->env) == 0) {
            /* add this thread into the waiting queue and jump to the scheduler
             */
            node *n = queue_erase(runnable, current_tcb);
            assert(n != NULL);
            queue_push(waiting, n);
            longjmp(scheduler_tcb->env, 1);
        } else {
            debug_print(
                "thread being joined on is now finished, starting cleanup and "
                "resuming operation");
            /* coming back after finishing stuff, now thread has a return value
             * in it free this thread */
            assert(thread->is_finished);
            goto acquire_ret_val_and_free_stuff;
        }
    }

acquire_ret_val_and_free_stuff:
    debug_print("cleanup for thread %d started", thread->tid);
    current_tcb->joiner = NULL;
    if (retval != NULL) *retval = thread->retval;
    delete_thread(thread);
    disabled_interrupts = 0;
    return;
}

int myThread_cancel(myThread_t thread);

int myThread_attr_init(myThread_attr_t *attr) {
    if (attr == NULL) return -1;
    *attr = DEFAULT_STACK_SIZE;
    return 0;
}

int myThread_attr_destroy(myThread_attr_t *attr) {
    free(attr);
    return 0;
}

myThread_t myThread_self(void) { return current_tcb; }



/************
 * Lock API *
 ************/


void myThread_mutex_init(mutex_t *mutex, void *attr) {
    mutex->is_free = 1;
    mutex->waiting = queue_create();
}

/* do contents of each mutex's waiting queue need to be freed up at the end?
 * - ideally no, since otherwise there will be a mutex which is still busy at
 * the end of the program or the program doesn't end, which is a deadlock */
void myThread_mutex_destroy(mutex_t *mutex) {
    queue_destroy(mutex->waiting);
}

/* note that when we are trying to acquire a lock, we can never be in a blocking
 * state and the join_caller_id of a thread is set iff join_caller_id is NULL or
 * is waiting so it is impossible for a thread to try to put a thread from the
 * waiting queue which hasn't acquired its lock yet onto the runnable queue */
void myThread_mutex_lock(mutex_t *mutex) {
    disabled_interrupts = 1;

    debug_print("trying to lock from thread %d", current_tcb->tid);
    if (!(mutex->is_free)) {
        debug_print("lock isn't free, queue looks like:");
        /* move current_tcb from runnable to waiting (lock-local and global) */
        node *current_tcb_node = queue_erase(runnable, current_tcb);
        /* make a new copy of a node to add to local queue, to avoid messing
         * with the nxt pointers. note that this will still point to the same
         * tcb */
        debug_print_queue(runnable);
        node *current_tcb_node_copy = node_create(current_tcb);
        queue_push(waiting, current_tcb_node);
        queue_push(mutex->waiting, current_tcb_node_copy);
        /* setjmp current tcb to env and longjmp to scheduler */
        if (setjmp(current_tcb->env) == 0) {
            longjmp(scheduler_tcb->env, 1);
        }
    }
    debug_print("successfully acquired the lock");
    mutex->is_free = 0;
    disabled_interrupts = 0;
}

void myThread_mutex_unlock(mutex_t *mutex) {
    disabled_interrupts = 1;
    debug_print("unlocking lock from thread %d", current_tcb->tid);
    if (mutex->waiting->size > 0) {
        /* move front tcb from waiting (global) to the runnable queue
         * and pop from the lock-local queue */
        debug_print(
            "before freeing the first waiting thread from the mutex queue, it "
            "looks like:");
        debug_print_queue(mutex->waiting);

        node *to_remove_copy =
            queue_erase(mutex->waiting, queue_peek(mutex->waiting)->data);
        node *to_remove = queue_erase(waiting, to_remove_copy->data);

        free(to_remove_copy);

        queue_push(runnable, to_remove);

        debug_print(
            "after freeing the first waiting thread from the mutex queue, it "
            "looks like:");
        debug_print_queue(mutex->waiting);

        debug_print(
            "after adding the removed stuff to runnable, runnable looks like:");
        debug_print_queue(runnable);
        /* the following is not necessary - proof: in book */
        /* if (mutex->waiting->size == 0) mutex->is_free = 1; */
        /* don't need to longjmp to scheduler here, since unlock is not meant to
         * be blocking and we can just return because of this */
    } else {
        mutex->is_free = 1;
    }
    disabled_interrupts = 0;
}


/**************************
 * Condition variable API *
 **************************/



void myThread_cond_init(cv_t *cond, void *attr) {
    cond->waiting = queue_create();
}

void myThread_cond_destroy(cv_t *cond) { queue_destroy(cond->waiting); }

void myThread_cond_wait(cv_t *cond, mutex_t *mutex) {
    disabled_interrupts = 1;

    node *current_tcb_node = queue_erase(runnable, current_tcb);
    /* make a new copy of a node to add to local queue, to avoid messing
     * with the nxt pointers. note that this will still point to the same
     * tcb */

    debug_print_queue(runnable);

    node *current_tcb_node_copy = node_create(current_tcb);
    queue_push(waiting, current_tcb_node);
    queue_push(cond->waiting, current_tcb_node_copy);

    myThread_mutex_unlock(mutex);

    /* setjmp current tcb to env and longjmp to scheduler */
    if (setjmp(current_tcb->env) == 0) {
        longjmp(scheduler_tcb->env, 1);
    }

    disabled_interrupts = 0;
    myThread_mutex_lock(mutex);
}

void myThread_cond_signal(cv_t *cond) {
    disabled_interrupts = 1;
    if (cond->waiting->size > 0) {
        /* move front tcb from waiting (global) to the runnable queue
         * and pop from the cv-local queue */
        debug_print(
            "before freeing the first waiting thread from the cv queue, it "
            "looks like:");
        debug_print_queue(cond->waiting);

        node *to_remove_copy =
            queue_erase(cond->waiting, queue_peek(cond->waiting)->data);
        node *to_remove = queue_erase(waiting, to_remove_copy->data);

        free(to_remove_copy);

        queue_push(runnable, to_remove);

        debug_print(
            "after freeing the first waiting thread from the cv queue, it "
            "looks like:");
        debug_print_queue(cond->waiting);

        debug_print(
            "after adding the removed stuff to runnable, runnable looks like:");
        debug_print_queue(runnable);
        /* don't need to longjmp to scheduler here, since signal is not meant to
         * be blocking and we can just return because of this */
    }
    disabled_interrupts = 0;
}

void myThread_cond_broadcast(cv_t *cond) {
    disabled_interrupts = 1;
    while (cond->waiting->size > 0) {
        /* move front tcb from waiting (global) to the runnable queue
         * and pop from the cv-local queue */
        debug_print(
            "before freeing the first waiting thread from the cv queue, it "
            "looks like:");
        debug_print_queue(cond->waiting);

        node *to_remove_copy =
            queue_erase(cond->waiting, queue_peek(cond->waiting)->data);
        node *to_remove = queue_erase(waiting, to_remove_copy->data);

        free(to_remove_copy);

        queue_push(runnable, to_remove);

        debug_print(
            "after freeing the first waiting thread from the cv queue, it "
            "looks like:");
        debug_print_queue(cond->waiting);

        debug_print(
            "after adding the removed stuff to runnable, runnable looks like:");
        debug_print_queue(runnable);
        /* don't need to longjmp to scheduler here, since broadcast is not meant
         * to be blocking and we can just return because of this */
    }
    disabled_interrupts = 0;
}

