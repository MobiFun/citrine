/* Include Nucleus C-Library file */
//#include "ncl\inc\nu_ncl.h"

/* Include necessary Nucleus PLUS files.  */
#include "nucleus.h"

/* Define serial output/input functionality. To disable serial I/O,
   replace NU_TRUE with NU_FALSE */

#define NU_SERIAL_OUTPUT  NU_TRUE
#define NU_SERIAL_INPUT   NU_TRUE

#if (NU_SERIAL_OUTPUT)
#include "nu_sd.h"            /* Nucleus Serial Driver interface */
#endif

/* Define Application data structures.  */

NU_TASK         Task_0;
NU_TASK         Task_1;
NU_TASK         Task_2;
NU_TASK         Task_3;
NU_TASK         Task_4;
NU_TASK         Task_5;
NU_QUEUE        Queue_0;
NU_SEMAPHORE    Semaphore_0;
NU_EVENT_GROUP  Event_Group_0;
NU_MEMORY_POOL  System_Memory;


/* Allocate global counters. */
UNSIGNED  Task_Time;
UNSIGNED  Task_2_messages_received;
UNSIGNED  Task_2_invalid_messages;
UNSIGNED  Task_1_messages_sent;
NU_TASK  *Who_has_the_resource;
UNSIGNED  Event_Detections;

#if (NU_SERIAL_OUTPUT)
NU_SERIAL_PORT  port;
#endif

#ifdef NU_FIQ_DEMO
UINT32 FIQ_Count;
#endif

extern  UNSIGNED TMD_System_Clock;

/* Define prototypes for function references.  */
VOID    task_0(UNSIGNED argc, VOID *argv);
VOID    task_1(UNSIGNED argc, VOID *argv);
VOID    task_2(UNSIGNED argc, VOID *argv);
VOID    task_3_and_4(UNSIGNED argc, VOID *argv);
VOID    task_5(UNSIGNED argc, VOID *argv);
CHAR    buffer[12]; /* temp buffer for Itoa conversion */
INT     n; /* strlen */



/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */

void    Application_Initialize(void *first_available_memory)
{

VOID           *pointer;
STATUS         status;

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
    status = NU_Create_Memory_Pool(&System_Memory, "SYSMEM",
                        first_available_memory, 25000, 50, NU_FIFO);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create each task in the system.  */

    /* Create task 0.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    status = NU_Create_Task(&Task_0, "TASK 0", task_0, 0, NU_NULL, pointer, 2000, 1, 20,
                                                      NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create task 1.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    status = NU_Create_Task(&Task_1, "TASK 1", task_1, 0, NU_NULL, pointer, 2000, 10, 5,
                                                      NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create task 2.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    status = NU_Create_Task(&Task_2, "TASK 2", task_2, 0, NU_NULL, pointer, 2000, 10, 5,
                                                      NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create task 3.  Note that task 4 uses the same instruction area.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    status = NU_Create_Task(&Task_3, "TASK 3", task_3_and_4, 0, NU_NULL, pointer,
                                        2000, 5, 0, NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create task 4.  Note that task 3 uses the same instruction area.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    status = NU_Create_Task(&Task_4, "TASK 4", task_3_and_4, 0, NU_NULL, pointer,
                                        2000, 5, 0, NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create task 5.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    status = NU_Create_Task(&Task_5, "TASK 5", task_5, 0, NU_NULL, pointer, 2000, 7, 0,
                                                      NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }


    /* Create communication queue.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 100*sizeof(UNSIGNED),
                                                        NU_NO_SUSPEND);
    status = NU_Create_Queue(&Queue_0, "QUEUE 0", pointer, 100, NU_FIXED_SIZE, 1,
                                                                      NU_FIFO);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create synchronization semaphore.  */
    status = NU_Create_Semaphore(&Semaphore_0, "SEM 0", 1, NU_FIFO);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

    /* Create event flag group.  */
    status = NU_Create_Event_Group(&Event_Group_0, "EVGROUP0");
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }


}

/* Define the system timer task.  More complicated systems might use a
   routine like this to perform periodic message sending and other time
   oriented functions.  */


void   task_0(UNSIGNED argc, VOID *argv)
{

STATUS          status;


#if (NU_SERIAL_OUTPUT)
CHAR            msg[40];
INT             i;

CHAR            ch;
#endif /* NU_SERIAL_OUTPUT */


#if (NU_SERIAL_OUTPUT)
    /* Init the serial port. */
    port.com_port   = DEFAULT_UART_PORT;
    port.baud_rate  = DEFAULT_UART_BAUD;
    port.data_bits  = DEFAULT_UART_DATA;
    port.stop_bits  = DEFAULT_UART_STOP;
    port.parity     = DEFAULT_UART_PARITY;
    port.data_mode  = DEFAULT_UART_MODE;
    port.communication_mode = SERIAL_MODE;
    port.sd_buffer_size   =   DEFAULT_UART_BUFFER;

    status = NU_SD_Init_Port (&port);
    if (status != NU_SUCCESS)
    {
        ERC_System_Error(status);
    }

#endif /* NU_SERIAL_OUTPUT */


    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Set the clock to 0.  This clock ticks every 18 system timer ticks. */
    Task_Time =  0;

        while(1)
        {

            /* Sleep for 100 timer ticks.  The value of the tick is programmable
               in INT.S and is relative to the speed of the target system.  */
            NU_Sleep(100);

#if (NU_SERIAL_OUTPUT)
            NU_SD_Put_String("\n\r****************************************", &port);
            NU_SD_Put_String("***************************************\n\r", &port);
            NU_SD_Put_String(NU_Release_Information(), &port);
            NU_SD_Put_String("\n\r", &port);

            NU_SD_Put_String("****************************************", &port);
            NU_SD_Put_String("***************************************\n\n\r", &port);
            NU_SD_Put_String("System Variable Status: \n\n\r", &port);

            strcpy(msg, "Task 0 time:                      ");
	    sprintf(buffer, "%lu", Task_Time);
            n = strlen(buffer);
            if (n>=8)
            {
                strcat(msg, buffer);
                strcat(msg, "\n\r");
            }
            else
            {
                for (i=0;i<(8-n);i++)
                    strcat(msg, " ");
                strcat(msg, buffer);
                strcat(msg, "\n\r");
            }

            NU_SD_Put_String(msg, &port);

            strcpy(msg, "Event detections:                 ");
	    sprintf(buffer, "%lu", Event_Detections);
            n = strlen(buffer);
            if (n>=8)
            {
                strcat(msg, buffer);
                strcat(msg, "\n\n\n\r");
            }
            else
            {
                for (i=0;i<(8-n);i++)
                    strcat(msg, " ");
                strcat(msg, buffer);
                strcat(msg, "\n\n\n\r");
            }

            NU_SD_Put_String(msg, &port);

            strcpy(msg, "Task 1 messages sent:             ");
	    sprintf(buffer, "%lu", Task_1_messages_sent);
            n = strlen(buffer);
            if (n>=8)
            {
                strcat(msg, buffer);
                strcat(msg, "\n\r");
            }
            else
            {
                for (i=0;i<(8-n);i++)
                    strcat(msg, " ");
                strcat(msg, buffer);
                strcat(msg, "\n\r");
            }

            NU_SD_Put_String(msg, &port);

            strcpy(msg, "Task 2 messages received:         ");
	    sprintf(buffer, "%lu", Task_2_messages_received);
            n = strlen(buffer);
            if (n>=8)
            {
                strcat(msg, buffer);
                strcat(msg, "\n\n\r");
            }
            else
            {
                for (i=0;i<(8-n);i++)
                    strcat(msg, " ");
                strcat(msg, buffer);
                strcat(msg, "\n\n\r");
            }

            NU_SD_Put_String(msg, &port);

            strcpy(msg, "Task 2 invalid messages:          ");
	    sprintf(buffer, "%lu", Task_2_invalid_messages);
            n = strlen(buffer);
            if (n>=8)
            {
                strcat(msg, buffer);
                strcat(msg, "\n\n\r");
            }
            else
            {
                for (i=0;i<(8-n);i++)
                    strcat(msg, " ");
                strcat(msg, buffer);
                strcat(msg, "\n\n\r");
            }

            NU_SD_Put_String(msg, &port);

            if (Who_has_the_resource == &Task_3)
               NU_SD_Put_String("Who has the resource:               Task 3", &port);
            else if (Who_has_the_resource == &Task_4)
               NU_SD_Put_String("Who has the resource:               Task 4", &port);
            else
               NU_SD_Put_String("Who has the resource:               Nobody", &port);
            NU_SD_Put_String("\n\n\n\r", &port);

            strcpy(msg, "Timer Interrupts:                 ");
	    sprintf(buffer, "%lu", TMD_System_Clock);
            n = strlen(buffer);
            if (n>=8)
            {
                strcat(msg, buffer);
                strcat(msg, "\n\n\r");
            }
            else
            {
                for (i=0;i<(8-n);i++)
                    strcat(msg, " ");
                strcat(msg, buffer);
                strcat(msg, "\n\n\r");
            }

            NU_SD_Put_String(msg, &port);

            NU_SD_Put_String("Buffer:  ", &port);

#if (NU_SERIAL_INPUT)
            while (NU_SD_Data_Ready(&port))
            {
                ch = NU_SD_Get_Char(&port);
                NU_SD_Put_Char(ch, &port);
            }
#endif /* NU_SERIAL_INPUT */


            NU_SD_Put_String("\n\n\r", &port);


#endif /* NU_SERIAL_OUTPUT */
            /* Increment the time.  */
            Task_Time++;

            /* Set an event flag to lift the suspension on task 5.  */
            status =  NU_Set_Events(&Event_Group_0, 1, NU_OR);

        }

}


/* Define the queue sending task.  Note that the only things that cause
   this task to suspend are queue full conditions and the time slice
   specified in the configuration file.  */

void   task_1(UNSIGNED argc, VOID *argv)
{

STATUS         status;
UNSIGNED       Send_Message;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Initialize the message counter.  */
    Task_1_messages_sent =  0;

    /* Initialize the message contents.  The receiver will examine the
       message contents for errors.  */
    Send_Message = 0;

    while(1)
    {
    
         /* Send the message to Queue_0, which task 2 reads from.  Note
            that if the destination queue fills up this task suspends until
            room becomes available.  */
         status =  NU_Send_To_Queue(&Queue_0, &Send_Message, 1, NU_SUSPEND);
         
         /* Determine if the message was sent successfully.  */
         if (status == NU_SUCCESS)
             Task_1_messages_sent++;
             
         /* Modify the contents of the next message to send.  */
         Send_Message++;
    }
}


/* Define the queue receiving task.  Note that the only things that cause
   this task to suspend are queue empty conditions and the time slice
   specified in the configuration file.   */

void   task_2(UNSIGNED argc, VOID *argv)
{

STATUS         status; 
UNSIGNED       Receive_Message;
UNSIGNED       received_size;
UNSIGNED       message_expected;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Initialize the message counter.  */
    Task_2_messages_received =  0;

    /* Initialize the message error counter.  */
    Task_2_invalid_messages =  0;

    /* Initialize the message contents to expect.  */
    message_expected =  0;
    
    while(1)
    {
    
         /* Retrieve a message from Queue_0, which task 1 writes to.  Note
            that if the source queue is empty this task suspends until
            something becomes available.  */
         status =  NU_Receive_From_Queue(&Queue_0, &Receive_Message, 1, 
                                &received_size, NU_SUSPEND);
         
         /* Determine if the message was received successfully.  */
         if (status == NU_SUCCESS)
             Task_2_messages_received++;
             
         /* Check the contents of the message against what this task
            is expecting.  */
         if ((received_size != 1) ||
             (Receive_Message != message_expected))
             Task_2_invalid_messages++;
         
         /* Modify the expected contents of the next message.  */
         message_expected++;
    }
}


/* Tasks 3 and 4 want a single resource.  Once one of the tasks gets the
   resource, it keeps it for 100 clock ticks before releasing it.  During
   this time the other task suspends waiting for the resource.  Note that
   both task 3 and 4 use the same instruction areas but have different 
   stacks.  */
   
void  task_3_and_4(UNSIGNED argc, VOID *argv)
{

STATUS  status;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Loop to allocate and deallocate the resource.  */
    while(1)
    {
    
         /* Allocate the resource.  Suspend until it becomes available.  */
         status =  NU_Obtain_Semaphore(&Semaphore_0, NU_SUSPEND);
         
         /* If the status is successful, show that this task owns the 
            resource.  */
         if (status ==  NU_SUCCESS)
         {
         
             Who_has_the_resource =  NU_Current_Task_Pointer();
             
             /* Sleep for 100 ticks to cause the other task to suspend on 
                the resource.  */
             NU_Sleep(100);
             
             /* Release the semaphore.  */
             NU_Release_Semaphore(&Semaphore_0);
        }
    }
}


/* Define the task that waits for the event to be set by task 0.  */

void  task_5(UNSIGNED argc, VOID *argv)
{

STATUS          status;
UNSIGNED        event_group;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Initialize the event detection counter.  */
    Event_Detections =  0;

    /* Continue this process forever.  */
    while(1)
    {
        /* Wait for an event and consume it.  */
        status =  NU_Retrieve_Events(&Event_Group_0, 1, NU_OR_CONSUME,
                                     &event_group, NU_SUSPEND);

        /* If the status is okay, increment the counter.  */
        if (status == NU_SUCCESS)
        {
          Event_Detections++;

        }
    }
}


#ifdef NU_FIQ_DEMO
void FIQ_LISR(VOID)
{
    FIQ_Count++;
}
#endif


