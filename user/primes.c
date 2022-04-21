#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void dfs(int p[])
{
    int pr;
    if(read(p[0],&pr,4)<=0)exit(1);
    fprintf(2,"prime %d\n",pr);
    int n;
    int p2[2];
    pipe(p2);
    if(read(p[0],&n,4)<=0)exit(1);
    if(fork()==0)
    {
        close(p2[1]);
        dfs(p2);
        close(p2[0]);
        exit(0);
    }
    else{
        close(p2[0]);
        if(n%pr)
        {
            write(p2[1],&n,4);
        }
        while(read(p[0],&n,4))
        {
            if(n%pr)
            {
                write(p2[1],&n,4);
            }
        }
        close(p2[1]);
        wait(0);

        exit(0);
    }
}
int main()
{
    int p[2];
    pipe(p);
    int i;
    if(fork()==0)
    {   close(p[1]);
        dfs(p);
        close(p[0]);
        exit(0);
    }
    else{
    close(p[0]);
    for(i=2;i<35;i++)
    {
        write(p[1],&i,4);
    }
    close(p[1]);
    wait(0);
    exit(0);
    }
}