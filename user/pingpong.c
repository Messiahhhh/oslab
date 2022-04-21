#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main()
{
    int p1[2];
    int p2[2];
    if(pipe(p1)<0)
    {
        fprintf(2,"error\n");
        exit(1);
    }
    if(pipe(p2)<0)
    {
        fprintf(2,"error\n");
        exit(1);
    }
    if(fork()==0)
    {
    
        char c;
        read(p1[1],&c,1);
        fprintf(2,"%d: received ping\n",getpid());
        write(p2[0],&c,1);
        exit(0);
        
    }
    else{
        char d='a';
        write(p1[0],&d,1);
        read(p2[1],&d,1);
        
        fprintf(2,"%d: received pong\n",getpid());
        exit(0);
    }   
    
}