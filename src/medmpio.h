//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#ifndef uvjet$monitor
#define uvjet$monitor 1

#include <pthread.h>

pthread_mutex_t serializeAccess;
pthread_mutex_t protectAccess;

#define MONITOR_CREATE(x)  {  \
                            pthread_mutexattr_t attr; \
                            if(pthread_mutexattr_init(&attr) == -1) { \
                               perror("pthread_mutexattr_init failed"); \
                               exit(1);  \
                            }  \
                            if(pthread_mutex_init(x, &attr) == -1) { \
                               perror("pthread_mutex_init failed"); \
                               exit(2);  \
                            }  \
                           }

#define MONITOR_ENTER(x,y) { \
                          /*printf("enter pthread_mutex_lock %s\n",y);*/  \
                          if(pthread_mutex_lock(&(x)) == -1) { \
                            perror("pthread_mutex_lock failed"); \
                            exit(3); \
                          } \
                         }

#define MONITOR_EXIT(x,y)  { \
                          /*printf("enter pthread_mutex_unlock %s\n",y);*/  \
                          if(pthread_mutex_unlock(&(x)) == -1) { \
                            perror("pthread_mutex_unlock failed"); \
                            exit(4); \
                          } \
                         }


#endif

