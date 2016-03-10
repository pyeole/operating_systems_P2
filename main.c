/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *  Author...........: Vincent W. Freeh & Pratik P. Yeole
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

 extern char **environ;
 static void prCmd(Cmd c)
 {
 	int i;

  if ( c ) {
    //printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
   /* if ( c->in == Tin )
      //printf("<(%s) ", c->infile);
    if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
	printf(">(%s) ", c->outfile);
	break;
      case Tapp:
	printf(">>(%s) ", c->outfile);
	break;
      case ToutErr:
	printf(">&(%s) ", c->outfile);
	break;
      case TappErr:
	printf(">>&(%s) ", c->outfile);
	break;
      case Tpipe:
	printf("| ");
	break;
      case TpipeErr:
	printf("|& ");
	break;
      default:
	fprintf(stderr, "Shouldn't get here\n");
	exit(-1);
      }*/

    //if ( c->nargs > 1 ) {
      //printf("[");
      //for ( i = 1; c->args[i] != NULL; i++ )
	//printf("%d:%s,", i, c->args[i]);
      //printf("\b]");
    //}
    //putchar('\n');
    // this driver understands one command
    if ( !strcmp(c->args[0], "end") )
      exit(0);
  }
 }
 
 static void prPipe(Pipe p)
 {
 	int i = 0;
  Cmd c;

  if ( p == NULL )
    return;

  //printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  for ( c = p->head; c != NULL; c = c->next ) {
    //printf("  Cmd #%d: ", ++i);
    prCmd(c);
  }
  //printf("End pipe\n");
  prPipe(p->next);
 }
 
 int main(int argc, char *argv[])
 {
 	int file;
	Pipe p;
	//get hostname of the system
	char host[20];
	gethostname(host,20);
	int flag;
	flag=0;
	int fd[2];
	pid_t pid;

	//reading from /.ushrc file at the start
	//int freshstdIn = dup(0);
	int oldStdIn;
	char *host_n = malloc(100);
	host_n = getenv("HOME");
	strcat(host_n, "/.ushrc");
	//printf("%s\n",host_n);
	file=open(host_n,O_RDONLY,S_IRUSR | S_IWUSR | S_IXUSR);
	if(file){
		flag=1;
		oldStdIn = dup(0);
		dup2(file, 0);
	}
	while(1)
	{
	if(flag==0){
		printf("%s%% ", host);
		fflush(stdout);
	}
    	p = parse();
    	prPipe(p);
    	Cmd c;
    	int read,write;
    	while(p)
    	{
    		c=p->head;
			/*if(c->in==Tin){
			int fd0;
			fd0=open(c->infile,O_RDONLY);
			dup2(fd0,0);
			close(fd0);
			}*/
			if(c->out==Tpipe)
			{
				pipe(fd);
				read=fd[0];
				write=fd[1];
			}
			while(c)
			{
				int oldStdIn = dup(0);
				int oldStdOut = dup(1);
				int oldStdErr = dup(2);
				if(!strcmp(c->args[0], "cd") && c->nargs>1)
				{
					//printf("inside IF for cd with args\n");
					//char *path[1000];		
					//strcpy(path,c->args[1]);
					//char cwd[256];
					//getcwd(cwd,sizeof(cwd));
					//strcat(cwd,"/"); 
					//strcat(cwd,path);
					chdir(c->args[1]);
					//printf("%s%%\n",path);  	
				}
				else if(!strcmp(c->args[0], "cd") && c->nargs==1)
				{
					//printf("inside IF for cd without args\n");
					char *path;
					path=getenv("HOME");
					chdir(path);
					//printf("%s%s%%\n",host,path);
				}
				else if(!strcmp(c->args[0], "pwd"))
				{
					//printf("inside IF for pwd\n");
					char cwd[256];
					if(c->out==Tout){
						int fd1;
						fd1=open(c->outfile,O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
						dup2(fd1,1);
						getcwd(cwd,sizeof(cwd));
						printf("%s\n",cwd);
						close(fd1);
						dup2(oldStdOut,1);
					}
					else if(c->out == Tpipe){
						dup2(write,1);
						int f_local = read;
						close(f_local);
						getcwd(cwd,sizeof(cwd));
						printf("%s\n",cwd);
					}
					else {
						getcwd(cwd,sizeof(cwd));
						printf("%s\n",cwd);
					}
				}
				else if(!strcmp(c->args[0],"echo"))
				{
					//printf("inside IF for echo\n");		
					int i;
					for(i = 1; c->args[i] != NULL; i++){
						printf("%s ", c->args[i]);
					}
					printf("\n");
				}
				else if(!strcmp(c->args[0],"logout"))
				{
					//printf("Bye %s!\n",host);
					_exit(0);
				}
				else if(!strcmp(c->args[0],"setenv") && c->nargs==3)
				{
					//printf("inside IF for setenv\n");
					if(c->in==Tin){
						int fd0;
						fd0=open(c->infile,O_RDONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
						dup2(fd0,0);
						setenv(c->args[1],c->args[2],c->args[3]);
						char *p;
						p=getenv(c->args[1]);
						//printf("setenv successfull! %s:%s\n",c->args[1],p);
						close(fd0);
						dup2(oldStdIn,0);
					}
					else{
						setenv(c->args[1],c->args[2],c->args[3]);
						char *p;
						p=getenv(c->args[1]);
						//printf("setenv successfull! %s:%s\n",c->args[1],p);
					}	
				}
				else if(!strcmp(c->args[0],"setenv") && c->nargs==2)
				{
					if(c->in==Tin){
						int fd0;
						fd0=open(c->infile,O_RDONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
						dup2(fd0,0);
						setenv(fd0,NULL,1);
						close(fd0);
						dup2(oldStdIn,0);
					}
					else{
						setenv(c->args[1],NULL,1);
					}
				}
				else if(!strcmp(c->args[0],"setenv") && c->nargs==1){
					//printf("Environment list:\n");
					char **ch;
					if(c->out==Tout){
						int fd1;
						fd1=open(c->outfile,O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
						dup2(fd1,1);
						//write into a file
						for(ch=environ; (*ch)!=NULL; ch++){
							puts(*ch);
						}
						close(fd1);
						dup2(oldStdOut,1);
					}
					else{
						for(ch=environ; (*ch)!=NULL; ch++){
							puts(*ch);
						}
					}
				}
				else if(!strcmp(c->args[0],"unsetenv") && c->nargs==2)
				{
					//p=getenv(c->args[1]);
					if(c->in==Tin){
						int fd0;
						fd0=open(c->infile,O_RDONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
						dup2(fd0,0);
						unsetenv(fd0);
						close(fd0);
						dup2(oldStdIn,0);
					}
					else{
						unsetenv(c->args[1]);
					}
					//printf("unsetenv successfull!\n");
				}
				else if(!strcmp(c->args[0],"end"))
				{
					//printf("inside END");
					exit(0);
				}
				else
				{
					pid_t pid;
					int status;
					pid = fork();
					if(pid < 0){
						//printf("ERROR:Child process fork failed!");
						exit(1);
					}
					else if(pid == 0){
						if(c->in==Tin){
							int fd1;
							fd1=open(c->infile,O_RDONLY,S_IRUSR | S_IWUSR | S_IXUSR);
							dup2(fd1,0);
							close(fd1);
							//write into a file
						}
						if(c->out==Tout){
							int fd1;
							fd1=open(c->outfile,O_WRONLY | O_CREAT | O_RDWR,S_IRUSR | S_IWUSR | S_IXUSR);
							dup2(fd1,1);
							close(fd1);
							//write into a file
						}
						if(c->out==Tapp){
							int fd2;
							fd2=open(c->outfile,O_APPEND | O_CREAT | O_RDWR,S_IRUSR | S_IWUSR | S_IXUSR);
							dup2(fd2,1);
							close(fd2);
							//write into a file
						}
						if(c->out==ToutErr){
							int fd3;
							fd3=open(c->outfile,O_WRONLY | O_CREAT | O_RDWR,S_IRUSR | S_IWUSR | S_IXUSR);
							dup2(fd3,1);
							dup2(fd3,2);
							close(fd3);
							//write into a file
						}
						if(c->out==TappErr){
							int fd4;
							fd4=open(c->outfile,O_APPEND | O_CREAT | O_RDWR,S_IRUSR | S_IWUSR | S_IXUSR);
							dup2(fd4,1);
							dup2(fd4,2);
							close(fd4);
							//write into a file
						}
						if(c->in==Tpipe){
							dup2(read,0);
							int f_local = write;
							close(f_local);
						}
						if(c->out==Tpipe){
							dup2(write,1);
							int f_local = read;
							close(f_local);
						}
						if(c->out==TpipeErr){
							dup2(write,1);
							dup2(write,2);
							int f_local = read;
							close(f_local);
						}
						//printf("before\n");
						execvp(c->args[0], c->args);		//execute the command
						if(errno == EACCES || errno == EISDIR){
							printf("permisssion denied\n");
						}
						else if(errno == ENOENT){
							printf("command not found\n");
						}
						exit(EXIT_FAILURE);
					}
					else {
						while(wait(&status) != pid);		//wait for completion
						close(write);
					}
				}
				c=c->next;
			}
			p=p->next;
    		}
    	freePipe(p);
	flag=0;
	dup2(oldStdIn,0);
	}
 }
