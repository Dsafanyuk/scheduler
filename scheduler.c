/**********************************************************************/
/*                                                                    */
/* Program Name: Scheduler - An OS Scheduler Simulation               */
/* Author:       Dave Safanyuk.                                       */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: March 27, 2019                                       */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* I pledge  the C language  statements in  this  program are  my own */
/* original  work,  and none  of the  C language  statements in  this */
/* program were copied  from any one else,  unless I was specifically  */
/* authorized to do so by my CS326 instructor.                        */
/*                                                                    */
/*                                                                    */
/*                           Signed: ________________________________ */
/*                                             (signature)            */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* This program simulates the environment in Unix where new processes */
/* are continually arriving, existing processes are vying for the     */
/* CPU, processes are using their given quantum (CPU bound) or        */
/* blocking because of I/O operations, and processes are terminating  */
/* when their work is finished.                                        */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>  /* printf, scanf                                  */
#include <stdlib.h> /* malloc, free                                   */

/**********************************************************************/
/*                          Symbolic Constants                        */
/**********************************************************************/
#define LIST_TRAILER           100 /* Trailer of the list              */
#define LIST_HEADER            0   /* Header of the list               */
#define MAX_TIME               18  /* Max processing time              */
#define CREATE_PROCESS_CHANCE  5   /* Process creation chance          */
#define UNBLOCK_PROCESS_CHANCE 20  /* Chance to block a process        */
#define MAX_PROCESSES          100 /* Max processes in process table   */
#define HEADER_ALLOC_ERR       1   /* Header memory alloc error        */
#define TRAILER_ALLOC_ERR      2   /* Trailer memory alloc error       */
#define PROCESS_ALLOC_ERR      3   /* Process memory alloc error       */
#define READY_STATE            'R' /* Ready state indicator            */
#define BLOCKED_STATE          'B' /* Blocked state indicator          */
#define RUNNING_STATE          'N' /* Running state indicator          */


/**********************************************************************/
/*                          Program Structures                        */
/**********************************************************************/
/* A process table entry.                                             */
struct process
{
   int pid,                        /* Id of the process               */
       cpu_used,                   /* CPU used by the process         */
       max_time,                   /* Max time process allowed to run */
       block_priority,             /* Blocking priority               */
       quantum_used,               /* Amount of quantum used          */
       block_time,                 /* Time when process is blocked    */
       wait_time;                  /* Waiting time of the process     */
   struct process *p_next_process; /* Points to the next process      */
   char state;                     /* State of the process            */
};
typedef struct process PROCESS;

/**********************************************************************/
/*                          Function Prototypes                       */
/**********************************************************************/
void initialize_process(PROCESS *p_process_list, int *p_process_id,
                                                 int *p_next_pid);
/* Initialize the beginning processes                                 */
void print_before_process_table(PROCESS *p_process_list, int process_id,
                                                         int next_pid);
/* Print the BEFORE process table                                     */
void print_after_process_table(PROCESS *p_process_list, int process_id,
                                                        int next_pid);
/* Print the AFTER process table                                      */
void print_processes(PROCESS *p_process_list);
/* Print the information of the process                               */
PROCESS *create_process_table();
/* Create  a linked list with a header and a trailer                  */
void create_process(PROCESS *p_process_list, int *p_process_id,
                    int *p_next_pid);
/* Create  a process and inserts the process into the linked list     */
void sort_processes(PROCESS *p_process_list);
/* Sort  process into ascending order including their priority        */
void delete_process(PROCESS *p_process_list, int *p_process_id,
                    int *p_next_pid);
/* Delete a process from the process table                            */

/**********************************************************************/
/*                            Main Function                           */
/**********************************************************************/
int main()
{
   PROCESS *p_process_list, /* Points to the process table            */
           *p_process;      /* Points to each process in the table    */

   int process_id = 0,      /* Number of processes in the table       */
       next_pid   = 1;      /* Next process id to come to the table   */

   /* Create a process table                                          */
   p_process_list = create_process_table();

   /* Fill process table with the initial processes                   */
   initialize_process(p_process_list, &process_id, &next_pid);

   /* Print the the BEFORE process table                              */
   print_before_process_table(p_process_list, process_id, next_pid);

   /* Point to the first process and set it as running                 */
   p_process = p_process_list->p_next_process;
   p_process->state = RUNNING_STATE;
   print_after_process_table(p_process_list, process_id, next_pid);

   /* Loops processing until the highest process is reached           */
   while (next_pid <= MAX_PROCESSES)
   {
      p_process = p_process_list->p_next_process;

      /* Checks to see if the amount of processes is less than 1      */
      if (process_id <= 1)
      {
         /* Loops until number of processes is greater than 1         */
         while (process_id <= 1)
         {
            if (!(rand() % CREATE_PROCESS_CHANCE))
               create_process(p_process_list, &process_id, &next_pid);
         }
      }

      /* Create a process                                             */
      if (!(rand() % CREATE_PROCESS_CHANCE))
         create_process(p_process_list, &process_id, &next_pid);

      /* Loops to unblock a process                                   */
      while (p_process->pid != LIST_TRAILER && p_process->state == BLOCKED_STATE)
      {
         if (!(rand() % UNBLOCK_PROCESS_CHANCE))
            p_process->state = READY_STATE;
         p_process = p_process->p_next_process;
      }

      /* Loops until it finds a running process                       */
      while (p_process->pid != LIST_TRAILER)
      {
         if (p_process->state == BLOCKED_STATE || p_process->state == READY_STATE)
            p_process = p_process->p_next_process;
         else
            break;
      }

      /* Checks to see if processes cpu used equals max time          */
      if (p_process->cpu_used == p_process->max_time)
      {
         print_before_process_table(p_process_list, process_id,
                                    next_pid);
         delete_process(p_process_list, &process_id, &next_pid);
         p_process = p_process_list->p_next_process;

         /* Loops until it finds a ready process                      */
         while (p_process->pid != LIST_TRAILER &&
                p_process->state != BLOCKED_STATE && p_process->state != READY_STATE)
         {
            p_process = p_process->p_next_process;
         }
         p_process->state = RUNNING_STATE;
         print_after_process_table(p_process_list, process_id, next_pid);
      }

      /* Checks to see if the process block time matches quantum used */
      if (p_process->block_time == p_process->quantum_used)
      {
         print_before_process_table(p_process_list, process_id,
                                    next_pid);
         if (p_process->block_priority < 0)
            p_process->block_priority *= -1;
         p_process->block_priority = ((int)(((float)(p_process->block_priority +
                                                     p_process->quantum_used)) /
                                                2.0f +
                                            0.5f));
         if (p_process->block_time < 6)
         {
            p_process->block_priority *= -1;
            p_process->state = BLOCKED_STATE;
         }
         else
            p_process->state = READY_STATE;

         p_process->quantum_used = 0;
         p_process = p_process_list->p_next_process;
         sort_processes(p_process_list);

         /* Loops until it finds a ready process                      */
         while (p_process->pid != LIST_TRAILER &&
                p_process->state == BLOCKED_STATE)
         {
            p_process = p_process->p_next_process;
         }
         p_process->state = RUNNING_STATE;
         print_after_process_table(p_process_list, process_id, next_pid);
      }
      p_process = p_process_list->p_next_process;

      /* Loops througgh the process to set the process information    */
      while (p_process->pid != LIST_TRAILER)
      {
         if (p_process->state == RUNNING_STATE)
         {
            p_process->cpu_used += 1;
            p_process->quantum_used += 1;
         }
         if (p_process->state == READY_STATE)
            p_process->wait_time += 1;
         p_process = p_process->p_next_process;
      }

      p_process = p_process_list->p_next_process;

      /* Loops until the process is blocked to unblock a process      */
      while (p_process->pid != LIST_TRAILER && p_process->state == BLOCKED_STATE)
      {
         if (!(rand() % UNBLOCK_PROCESS_CHANCE))
            p_process->state = READY_STATE;
         p_process = p_process->p_next_process;
      }
   }
   return 0;
}

/**********************************************************************/
/*                  Initialize the Beginning Processes                */
/**********************************************************************/
void initialize_process(PROCESS *p_process_list, int *p_process_id,
                        int *p_next_pid)
{
   int count_process;

   for (count_process = 1; count_process <= 5; count_process++)
   {
      create_process(p_process_list, p_process_id, p_next_pid);
   }
   return;
}

/**********************************************************************/
/*                   Sort And Print BEFORE Process Table              */
/**********************************************************************/
void print_before_process_table(PROCESS *p_process_list, int process_id,
                                int next_pid)
{
   sort_processes(p_process_list);
   printf("\nBEFORE SCHEDULING CPU:  ");
   printf("Next PID =  %d,  ", next_pid);
   printf("Number of Processes =  %d ", process_id);
   printf("\n PID   CPU Used   MAX Time   STATE   PRI   QUANTUM USED");
   printf("   BLK TIME   WAIT TKS");
   printf("\n");
   print_processes(p_process_list);
   return;
}

/**********************************************************************/
/*                  Sort And Print AFTER Process Table                */
/**********************************************************************/
void print_after_process_table(PROCESS *p_process_list, int process_id,
                               int next_pid)
{
   sort_processes(p_process_list);
   printf("\nAFTER SCHEDULING CPU:  ");
   printf("Next PID =  %d,  ", next_pid);
   printf("Number of Processes =  %d ", process_id);
   printf("\n PID   CPU Used   MAX Time   STATE   PRI   QUANTUM USED");
   printf("   BLK TIME   WAIT TKS");
   printf("\n");
   print_processes(p_process_list);
   return;
}

/**********************************************************************/
/*                     Print Process Information                      */
/**********************************************************************/
void print_processes(PROCESS *p_process_list)
{
   /* Loops until process reaches list trailer                        */
   while (p_process_list = p_process_list->p_next_process,
          p_process_list->pid != LIST_TRAILER)
   {
      printf("  %2d", p_process_list->pid);
      printf("      %2d", p_process_list->cpu_used);
      printf("         %2d", p_process_list->max_time);
      printf("        %c", p_process_list->state);
      printf("      %2d", p_process_list->block_priority);
      printf("        %2d", p_process_list->quantum_used);
      printf("           %2d", p_process_list->block_time);
      printf("         %3d", p_process_list->wait_time);
      printf("\n");
   }
   return;
}

/**********************************************************************/
/*                        Create a Process Table                      */
/**********************************************************************/
PROCESS *create_process_table()
{
   PROCESS *p_new_list; /* Points to the newly created list           */

   if ((p_new_list = (PROCESS *)malloc(sizeof(PROCESS))) == NULL)
   {
      printf("\nError #%d occurred in create_process_table.",
             HEADER_ALLOC_ERR);
      printf("\nCannot allocate memory for the list header.");
      printf("\nThe program is aborting.");
      exit(HEADER_ALLOC_ERR);
   }
   p_new_list->pid = LIST_HEADER;

   if ((p_new_list->p_next_process = (PROCESS *)malloc(sizeof(PROCESS))) == NULL)
   {
      printf("\nError #%d occurred in create_process_table.",
             TRAILER_ALLOC_ERR);
      printf("\nCannot allocate memory for the list trailer.");
      printf("\nThe program is aborting.");
      exit(TRAILER_ALLOC_ERR);
   }
   p_new_list->p_next_process->pid = LIST_TRAILER;
   p_new_list->p_next_process->p_next_process = NULL;
   return p_new_list;
}

/**********************************************************************/
/*                 Create a Process and Initialize it                 */
/**********************************************************************/
void create_process(PROCESS *p_process_list, int *p_process_id,
                    int *p_next_pid)
{
   PROCESS *p_new_process; /* Points to the newly created process     */

   if ((*p_process_id) < 10)
   {
      if ((p_new_process = (PROCESS *)malloc(sizeof(PROCESS))) == NULL)
      {
         printf("\nError #%d occurred in create_process_table.",
                PROCESS_ALLOC_ERR);
         printf("\nCannot allocate memory for the process.");
         printf("\nThe program is aborting.");
         exit(PROCESS_ALLOC_ERR);
      }
      p_new_process->p_next_process = p_process_list->p_next_process;
      p_process_list->p_next_process = p_new_process;
      p_new_process->pid = *p_next_pid;
      p_new_process->cpu_used = 0;
      p_new_process->max_time = rand() % MAX_TIME + 1;
      p_new_process->state = READY_STATE;
      p_new_process->block_priority = 0;
      p_new_process->quantum_used = 0;
      if (rand() % 3)
         p_new_process->block_time = rand() % 5 + 1;
      else
         p_new_process->block_time = 6;
      p_new_process->wait_time = 0;
      (*p_process_id)++;
      (*p_next_pid)++;
   }
   return;
}

/**********************************************************************/
/*          Sort the Process Table and Sort Process Priority          */
/**********************************************************************/
void sort_processes(PROCESS *p_process_list)
{
   PROCESS *p_current = p_process_list, /* Points to each process     */
       *p_temp;                         /* Holds a process            */
   int process_num;                     /* The amount of processes    */

   for (process_num = 1; process_num <= 6 - 1; process_num++)
   {
      p_current = p_process_list;
      while (p_current->p_next_process->p_next_process->pid <
             LIST_TRAILER)
      {
         if (p_current->p_next_process->pid >
             p_current->p_next_process->p_next_process->pid)
         {
            p_temp = p_current->p_next_process->p_next_process;
            p_current->p_next_process->p_next_process =
                p_current->p_next_process->p_next_process->p_next_process;
            p_temp->p_next_process = p_current->p_next_process;
            p_current->p_next_process = p_temp;
         }

         if (p_current->p_next_process->block_priority >
             p_current->p_next_process->p_next_process->block_priority)
         {
            p_temp = p_current->p_next_process->p_next_process;
            p_current->p_next_process->p_next_process =
                p_current->p_next_process->p_next_process->p_next_process;
            p_temp->p_next_process = p_current->p_next_process;
            p_current->p_next_process = p_temp;
         }

         if (p_current->p_next_process->block_priority < 0)
         {
            if (p_current->p_next_process->p_next_process->block_priority < 0)
            {
               if (p_current->p_next_process->block_priority <
                   p_current->p_next_process->p_next_process->block_priority)
               {
                  p_temp = p_current->p_next_process->p_next_process;
                  p_current->p_next_process->p_next_process =
                      p_current->p_next_process->p_next_process->p_next_process;
                  p_temp->p_next_process = p_current->p_next_process;
                  p_current->p_next_process = p_temp;
               }
            }
         }
         p_current = p_current->p_next_process;
      }
   }
   return;
}

/**********************************************************************/
/*                Terminate the Process When Finished                 */
/**********************************************************************/
void delete_process(PROCESS *p_process_list, int *p_process_id,
                    int *p_next_pid)
{
   PROCESS *p_current = p_process_list, /* Points to current process */
       *p_previous = p_process_list;    /* Points to previous process*/

   while (p_current = p_current->p_next_process, p_current->pid !=
                                                     LIST_TRAILER)
   {
      if (p_current->cpu_used == p_current->max_time)
      {
         p_previous->p_next_process = p_current->p_next_process;
         free(p_current);
         (*p_process_id) -= 1;
      }
      else
         p_previous = p_current;
   }
   return;
}
