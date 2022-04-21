#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc,char* argv[])
{
    int i=0;
    char args[MAXARG][MAXARG];
    
    
    while(read(0,args[i],512)>0)
    {
        if(args[i][strlen(args[i])-1]=='\n')args[i][strlen(args[i])-1]='\0';
        i++;
    }
    char* argsv[MAXARG];
    int j=0;
    int k=0;
    for(j=1;j<argc;j++)
    {
        argsv[k++]=argv[j];
        
    }
    for(j=0;j<i;j++)
    {
        argsv[k++]=args[j];
    }
    if(fork()==0)
    {
        // char p[MAXARG];
        // p[0]='/';
        // char *m=p;
        // m++;
        // strcpy(m,argsv[0]);
        // argsv[0]=p;
        exec(argsv[0],argsv);
    }
    else
    {
        wait(0);
        exit(0);
    }
    exit(0);
    
   
    
}