/*--------------------------------------
Melnyk O. V.
DA-82
lab 5
Program: SLAVE
--------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pvm3.h"

#define CV 1
/*const int NN = 65;*/
#define NN 65
#define ErrS -1L

main()
{

   int ptid = 0;
   char ch = '\0';
   char buf[NN] = {'\0'};
   long int t = 0;
   long int pos0 = 0;

   ptid = pvm_parent();

   pvm_recv(ptid, -1);
   pvm_upkbyte(&ch, 1, 1);
   pvm_upklong(&t, 1, 1);
   char * str = malloc((t + 1) * sizeof(char));
   str[t] = '\0';
   pvm_upkbyte(str, t, 1);
   pvm_upklong(&pos0, 1, 1);

   long int pos = ErrS;
   long int i = 0;
   for(i = 0; i < t; ++i)
   {
      if(str[i] == ch)
      {
         pos = i;
         break;
      }
      usleep(500);
   }
   if(pos > -1) pos += pos0;

   gethostname(buf, 64);

   pvm_initsend(PvmDataDefault);
   pvm_pkstr(buf);
   pvm_pklong(&pos, 1, 1);
   pvm_send(ptid, 1);

   free(str);

   pvm_exit();

   return 0;

}

