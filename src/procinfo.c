#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <pid>\n",a); exit(1);}
static int isnum(const char*s){for(;*s;s++) if(!isdigit(*s)) return 0; return 1;}
int main(int c,char**v){
 if(c!=2||!isnum(v[1])) usage(v[0]);
 
 char path[256];
 char line[256];
 FILE *f;

 
 snprintf(path, sizeof(path), "/proc/%s/status", v[1]);

 f = fopen(path, "r"); 
 if (!f) DIE("Process doesn't exist");
 printf("PID:%s\n", v[1]);

 unsigned long rss = 0;
 while (fgets(line, sizeof(line), f)) {
     if (strncmp(line, "State:", 6) == 0) {
         char state;
         sscanf(line, "State:\t%c", &state);
         printf("State:%c\n", state);
     }
     if (strncmp(line, "PPid:", 5) == 0) {
         unsigned int ppid;
         sscanf(line, "PPid:\t%u", &ppid);
         printf("PPID:%u\n", ppid);
     }
     if (strncmp(line, "VmRSS:", 6) == 0) {
         sscanf(line, "VmRSS:\t%lu", &rss);
     }
 }
 fclose(f);

 snprintf(path, sizeof(path), "/proc/%s/cmdline", v[1]);
 int fd = open(path, O_RDONLY);
 if (fd != -1) {
     ssize_t len = read(fd, line, sizeof(line) - 1);
     if (len > 0) {
         line[len] = '\0';
         for (int i = 0; i < len - 1; i++) if (line[i] == '\0') line[i] = ' ';
         printf("Cmd:%s\n", line);
     }
     close(fd);
    }

 unsigned long utime = 0, stime = 0;
 snprintf(path, sizeof(path), "/proc/%s/stat", v[1]);

 f = fopen(path, "r");
 if (f) { 
     if (fscanf(f, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime) == 2) { 
        unsigned long total_ticks = utime + stime;
        double seconds = (double)total_ticks / sysconf(_SC_CLK_TCK);
        printf("CPU:%lu %0.3f\n", total_ticks, seconds);
     }
     fclose(f);
 }

 printf("VmRSS:%lu\n", rss);

 return 0;
}
