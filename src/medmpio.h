/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

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

