#include <stdio.h>  /*required for proper input output functions*/
#include <string.h> /*contains the functions required for string manipulation and other string related functions*/
#include <stdlib.h> /*gives the user access to several other predefined library functions.*/
#include <signal.h> /*required to be able to handle signals efficiently*/
#include <unistd.h> /*unistd header gives the user access to the POSIX Operating System API and helps us manipulate System files*/

#define  no_usb "# this rule does not allow any new usb devices, use script to disable\nACTION==\"add\", DRIVERS==\"usb\",  ATTR{authorized}=\"0\"\n"
#define fname "/etc/udev/rules.d/11-to_rule_all.rules"
#define wait 20 // wait 20 seconds for new usb device

static unsigned int euid, ruid;        //program effective user ID and the running user ID
void do_setuid (void);
void undo_setuid(void);
void make_file();
void sig_handler(int signo);

int main(int argc, char *argv[])
{
  char * allow_rule;
  ruid = getuid();
  euid = geteuid();
  int choice;

  // rule to allow usb drives for a predefined amount of time
  char rule2[] =
    "#tmp allows all USB devices\n"
    "ACTION==\"add\", DRIVERS==\"usb\"\n";

  allow_rule=rule2;
	printf("1.\tBlock all USB Ports\n2.\tTemporarily Unblock all USB Ports\n3.\tPermanently Unblock all USB Ports\n");
	scanf("%d",&choice);
	if(choice==3)
	{
		FILE *fp;
  		do_setuid();
		fp= fopen(fname, "r+"); // opening in r+ mode
		undo_setuid();

		if (fp)
  		{
     			fprintf(fp, "%s", allow_rule); // for modifications in the rules file
     			if (ftruncate(fileno(fp), strlen(allow_rule)) != 0)
     			return 27;
     			fclose(fp);
     			printf("Unblocked USB Ports.\n");
     			system("rm -rf /etc/udev/rules.d/11-to_rule_all.rules");
  		}
		else 
			printf("USB Ports are not blocked.\n");
	}
	else if(choice==2)
	{
		FILE *fp;
  		do_setuid();
		fp= fopen(fname, "r+"); // opening in r+ mode
		undo_setuid();
		if (fp)
  		{
     			fprintf(fp, "%s", allow_rule); // for modifications in the rules file
     			if (ftruncate(fileno(fp), strlen(allow_rule)) != 0)
     			return 27;
     			fclose(fp);     			
			printf("USB Ports Temporarily Unblocked for 20 seconds.\n");
			system("rm -rf /etc/udev/rules.d/11-to_rule_all.rules");
			sleep(wait);
  			printf("Blocking again.\n");
  			if (signal(SIGTERM, sig_handler) == SIG_ERR)
  			return 88;         // mount error signal
			if (signal(SIGINT, sig_handler) == SIG_ERR)
			return 88;
			if (signal(SIGUSR1, sig_handler) == SIG_ERR)
			return 88;
			atexit(make_file); 
		}
		else 
			printf("USB Ports are not blocked.\n");

	}
	else if(choice==1)
	{
			if (signal(SIGTERM, sig_handler) == SIG_ERR)
  			return 88;         // mount error signal
			if (signal(SIGINT, sig_handler) == SIG_ERR)
			return 88;
			if (signal(SIGUSR1, sig_handler) == SIG_ERR)
			return 88;
			atexit(make_file); 
	}
	else 
		printf("Run program again with correct input.\n");
  return 0;
}

void sig_handler(int signo)
{
  if (signo == SIGUSR1) 
  printf("received SIGUSR1\n");
  else if (signo == SIGTERM) 
  printf("received SIGSTERM\n");
  else if (signo == SIGINT) 
  printf("\nreceived SIGINT\n");
  make_file();            //calling makefile function to create file
}

void make_file()
{
  // printf("making the file now.\n");
  FILE *fp;
  do_setuid();
  if (( fp = fopen(fname, "w") ))
  {
    fprintf(fp, "%s", no_usb);
    printf("Block all new USB rule added\n");
  }
  else
  {
    fprintf(stderr, "ERROR: could not make paranoid udev rules\n");
  }
  undo_setuid();
}

void do_setuid (void)
{
   int status;
   #ifdef _POSIX_SAVED_IDS
   status = seteuid(euid);
   #else
   status = setreuid(ruid, euid);
   #endif
   if (status < 0)
   {
     fprintf(stderr, "Couldn't set uid.\n");
     exit(status);
   }
}

void undo_setuid(void)
{
   int status;
   #ifdef _POSIX_SAVED_IDS
   status = seteuid(ruid);
   #else
   status = setreuid(euid, ruid);
   #endif
   if (status < 0)
   {
     fprintf(stderr, "Couldn't set uid.\n");
     exit(status);
   }
}
