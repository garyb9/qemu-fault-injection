#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>

#include <pthread.h>

#define FALSE 0
#define TRUE 1
#define MAX_LINE 1024
#define MAX_PASS_SIZE 65535
#define GOD_PASS "1234567890"


typedef struct Attack_Profile{
    int Range_Set_Up[2],
    Register,
    Cycle_Count,
    Type,
    seed,
    candidate;

}Attack_Profile;


unsigned long hash(unsigned char *str);
char *hash_sha256(char *string);
void command_and_read(char *comm);
void command_and_wait_for_resp(char *comm, FILE *fp, int port);
Attack_Profile init_att_p(int iter, int seed, int candidate, int attack_type, int min_range, int max_range);



