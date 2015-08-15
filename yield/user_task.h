#ifndef _USER_TASK_H_
#define _USER_TASK_H_

/* user_task.c */
int atexit ( void (*func )());
void abort ( void );
void esp_yield ( void );
void esp_schedule ( void );
//void __yield ( void );
void yield ( void );
void loop_wrapper ( void );
void init_done ( void );
void user_init ( void );

#endif                                            /* _USER_TASK_H_ */
