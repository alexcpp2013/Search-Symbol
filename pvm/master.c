/*--------------------------------------
Melnyk O. V.
DA-82
lab 5
Program: MASTER
--------------------------------------*/

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include "pvm3.h"

#define CLOCK_REALTIME                  0
#define CLOCK_MONOTONIC                 1
#define CLOCK_PROCESS_CPUTIME_ID        2
#define CLOCK_THREAD_CPUTIME_ID         3
#define CLOCK_MONOTONIC_RAW             4
#define CLOCK_REALTIME_COARSE           5
#define CLOCK_MONOTONIC_COARSE          6

#define NDEBUG 1
#define RANDSTR 0
/*const unsigned long int N = 100L;*/
#define N 100L
/*const char file[] = "/home/alex/pvm3/bin/LINUX/log.txt";*/
#define NN 65
#define ErrM -2L

int search(char * str, char ch, long int l);

int searchP(char * str, int k, long int l, char ch);

int randomSTR(char * str, long int l);

/*---------------------MAIN----------------------------------------------------------------*/

int main( int argc, char* argv[] )
{

   int k = 0;
   char ch = '\0';
   char str[N] = {'\0'};
   /*int j = 0;
   for(j = 0; j < N; ++j)
   {
      str[j] = '\0';
   }*/
   long int l = 0;

   pvm_catchout (stdout); /*fp or stdout*/
   printf("\n\nStart program.");
   printf("\nSymbol = ");
   scanf("%c", &ch);
   #if RANDSTR == 0
      printf("String = ");
      scanf("%s", str);
   #endif
   printf("Count of processors = ");
   scanf("%d", &k);
   printf("\n-------------------------RUNNING-----------------------------------\n");
   if(k <= 0)
   {
      printf("\nk must be more then 0.");
      return 1;
   }
   #if RANDSTR == 0
      l = strlen(str);
   #endif
   #if RANDSTR == 1
      l = (N - 1) - 0; /*0 or other value*/
      randomSTR(str, l);
   #endif
   if(l < k)
   {
      k = l;
   }

   printf("\nLength of string = %d bytes.", l);

/*-------------------------------------------------------*/

   search(str, ch, l);

   searchP(str, k, l, ch);

/*-------------------------------------------------------*/

   printf("\n\n");

   return 0;

}

/*----------------------------END MAIN--------------------------------------------------------*/

int search(char * str, char ch, long int l)
{

   const int clkID = CLOCK_REALTIME;
   struct timespec st, et;
   double   iv;
   double testDouble = LONG_MAX;

   clock_gettime(clkID, &st);
   printf("\nStart LINEAR search...\n");

   long int i = 0;
   int b = 1;
   for(i = 0; i < l; ++i)
   {
      if(str[i] == ch)
      {
         #if RANDSTR == 0
            printf("\'%c\' apears in \"%s\" at position %ld", ch, str, i);
         #endif
         #if RANDSTR == 1
            printf("\'%c\' apears in string at position %ld", ch, i);
         #endif
         b = 0;
         break;
      }
      usleep(500);
   }
   if(b)
   {
      #if RANDSTR == 0
         printf("There is not \'%c\' in \"%s\".", ch, str);
      #endif
      #if RANDSTR == 1
         printf("There is not \'%c\' in string.", ch);
      #endif
   }

   clock_gettime(clkID, &et);
   iv = (et.tv_sec * 1.0e9 + et.tv_nsec) - (st.tv_sec * 1.0e9 + st.tv_nsec); /*into nanosec*/
   printf("\n%.8lf seconds for LINEAR search.\n", iv / 1e9);

   return 0;

}

int searchP(char * str, int k, long int l, char ch)
{

   const int clkID = CLOCK_REALTIME;
   struct timespec st, et;
   double   iv;
   double testDouble = LONG_MAX;

   clock_gettime(clkID, &st);
   printf("\nStart PARALLEL search...\n");
   printf("Start MASTER.\n");
   printf("MASTER ID = %i\n", pvm_mytid());

   int * tid;
   if((tid = malloc(k * sizeof(int))) == NULL)
   {
      printf("Malloc error.");
      exit(1);
   }
   char buf[NN] = {'\0'};
   int cc = 0;

   cc = pvm_spawn("slave", 0, 0, "", k, tid);
   printf("Number of SLAVEs = %d\n", cc);

   long int * res;
   if((res = malloc(k * sizeof(long int))) == NULL)
   {
      printf("Malloc error.");
      exit(1);
   }
   int i = 0;
   /*for(i = 0; i < k; ++i, res[i - 1] = ErrM);*/
   memset(res, ErrM, sizeof(int) * k);

   if (cc == k)
   {
      long int nk = l / k;
      long int nk1 = labs(nk * k - l);
      long int t = nk;
      long int j = 0; /*Position after normalization in real string*/
      for(i = 0; i < k; ++i)
      {
         pvm_initsend(PvmDataDefault);
         /*if pvm_upkbyte then only int size but not uli, if pvm_upkstr then use strcmp every time or sent all string*/
         /*better - send 1 then 1 + i then 1 + 2i because statistic of symbol not equil in the text*/
         pvm_pkbyte(&ch, 1, 1);
         if(nk1 <= 0) t = nk;
         if(nk1 > 0)
         {
            t = nk + 1;
            --nk1;
         }
         printf("Count of bytes in the string send to the SLAVE = %ld.", t);
         pvm_pklong(&t, 1, 1);
         pvm_pkbyte(str + j, t, 1);
         pvm_pklong(&j, 1, 1);
         pvm_send(tid[i], 1);
         j += t;
         printf("\n");
      }
      for(i = 0; i < k; ++i)
      {
         pvm_recv(tid[i], -1);
         pvm_upkstr(buf);
         pvm_upklong(&res[i], 1, 1);
         printf("SLAVE name is %s\n", buf);
      }
   }
   else
   {
      printf("Can't start SLAVEs\n");
      for(i = 0; i < cc; ++i) pvm_kill(tid[i]);
      assert(cc == k);
   }

   pvm_exit();

   long int min = res[0];
   for(i = 0; i < k; ++i)
   {
      if(res[i] >= 0)
      {
         if(min < 0) min = res[i];
         if(res[i] < min)
            min = res[i];
      }
   }
   if(min >= 0)
   {
      #if RANDSTR == 0
         printf("\'%c\' appears in \"%s\" at position %ld", ch, str, min);
      #endif
      #if RANDSTR == 1
         printf("\'%c\' appears in string at position %ld", ch, min);
      #endif
   }
   else
   {
      #if RANDSTR == 0
         printf("There is not \'%c\' in \"%s\".", ch, str);
      #endif
      #if RANDSTR == 1
         printf("There is not \'%c\' in string.", ch);
      #endif
   }

   free(res);
   free(tid);

   clock_gettime(clkID, &et);
   iv = (et.tv_sec * 1.0e9 + et.tv_nsec) - (st.tv_sec * 1.0e9 + st.tv_nsec);
   printf("\n%.8lf seconds for PARALLEL search.\n", iv / 1e9);

   return 0;

}

int randomSTR(char * str, long int l) /*without last value '\0'*/
{

   long int i = 0;
   srand (time(NULL));
   for(i = 0; i < l; ++i)
   {
      int c = rand() % 23 + 98; /*no 97 'a' and no 122 'z' [98..121]*/
      str[i] = (char) c;
   }

   return 0;

}


