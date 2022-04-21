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
        close(p1[1]);
        close(p2[0]);
        char c;
        read(p1[0],&c,1);
        fprintf(2,"%d: received ping\n",getpid());
        write(p2[1],&c,1);
        close(p1[0]);
        close(p2[1]);
        exit(0);
        
    }
    else{
        char d='a';
        close(p1[0]);
        close(p2[1]);
        write(p1[1],&d,1);
        read(p2[0],&d,1);
        
        fprintf(2,"%d: received pong\n",getpid());
        close(p1[1]);
        close(p2[0]);
        exit(0);
    }   
    
}