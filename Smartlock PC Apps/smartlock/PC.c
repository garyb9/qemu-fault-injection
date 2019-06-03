#include "PC.h"

int UART_port = 0;
int got_success = 0;
int got_fail = 0;
int got_termination = 0;

void RESP_SMARTLOCK_A(void *unused){command_and_read("cd ~/project/stm32_p103_demos; make smartlock_a_QEMURUN_PTY;");}
void RESP_SMARTLOCK_B(void *unused){command_and_read("cd ~/project/stm32_p103_demos; make smartlock_b_QEMURUN_PTY;");}
void RESP_SMARTLOCK_C(void *unused){command_and_read("cd ~/project/stm32_p103_demos; make smartlock_c_QEMURUN_PTY;");}


void get_UART_port(char resp[]){
    int j=0;
    char cport;
    const char substring[10] = "/dev/pts/";
    for(int i = 0; (i < strlen(resp) - 1 || j < 10) ; i++){

        cport = resp[i];
        if((j == 9) && isdigit(cport)){
            UART_port = cport - 48;
            return;
        }

        if(resp[i] == substring[j]) j++;
        else j=0;
    }
}

void *cat_UART(void *input) {
    // command example: "cat /dev/pts/x"
    int port = (int *)input;
    char command[MAX_LINE];
    FILE *fp;

    strcpy(command, "cat /dev/pts/");
    command[13] = port + '0'; // int -> char
    command[14] = '\0';

    command_and_wait_for_resp(command, fp, port);
    if(fp!=NULL){
       pclose(fp);
    }
    pthread_exit(NULL);
}

void *echo_UART(void *input_port, void *str) {
     // command example: "echo "str" > /dev/pts/x"
    int port = (int *)input_port;
    char command[MAX_LINE] = "echo \"";

    strcat(command, str);
    strcat(command, "\" > /dev/pts/");

    for(int i=0; i<MAX_LINE; i++){
        if(command[i] == '\0'){
            command[i] = port + '0'; // int -> char
            command[i+1] = '\0';
            break;
        }
    }


    system(command); // send echo system call
    //pthread_exit(NULL);
}


void command_and_read(char *comm)
{
    // if resp isn't needed - pass substring as NULL
    FILE *fp;
    FILE *insts;
    char path[MAX_LINE];

    /* Open the command for reading. */
    insts = fopen("log.txt", "w");
    fp = popen(comm, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }


    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path)-1, fp) != NULL && !got_termination) {
        fprintf(insts,"%s", path);
        if(strstr(path, "4822")){
        sleep(1);
        }
        if(strstr(path, "terminating on signal")!=NULL){ //got termination signal from qemu
            got_termination = 1;
            break;
        }

    }
    /* close */
    fclose(insts);
    pclose(fp);
}

void command_and_wait_for_resp(char *comm, FILE *fp, int port){
    char path[MAX_LINE];

    /* Open the command for reading. */
    fp = popen(comm, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
            printf("%s\n", path);
            if(strstr(path, "cat")!=NULL) break;

            if(strstr(path, "Permission denied")!=NULL)break;

            if(strstr(path, "SUCCESS"))got_success = 1;
            if(strstr(path, "FAIL"))got_fail = 1;

        }

    /* close */
    pclose(fp);
}


Attack_Profile init_att_p(int iter, int seed, int candidate, int attack_type, int min_range, int max_range){
    int R;
    double R_cyc;
    Attack_Profile att_p;

    att_p.Range_Set_Up[0] = min_range;
    att_p.Range_Set_Up[1] = max_range;

    att_p.Type = attack_type;

    att_p.seed = seed;

    att_p.candidate = candidate;

    R = rand();
    R_cyc = (double)(R%1000)/1000;
    att_p.Cycle_Count = att_p.Range_Set_Up[0] + (att_p.Range_Set_Up[1] - att_p.Range_Set_Up[0]+1)*R_cyc; //Randomizes the clock cycle
    att_p.Register = R%16; // Randomizes the register

    return(att_p);
}

int main(int argc, char** argv){

    if(argc != 7){
        printf("Number of arguments given:  %d...\n", argc);
        printf("Not enough arguments given, Exiting...\n");
        exit(1);
    }

    int itr = (int) strtol(argv[1], (char **)NULL, 10),         //argv[1] - iterations
    seed = (int) strtol(argv[2], (char **)NULL, 10),            //argv[2] - seed
    candidate = (int) strtol(argv[3], (char **)NULL, 10),       //argv[3] - candidate
    attack_type = (int) strtol(argv[4], (char **)NULL, 10),     //argv[4] - attack type
    min_range = (int) strtol(argv[5], (char **)NULL, 10),       //argv[5] - min range
    max_range = (int) strtol(argv[6], (char **)NULL, 10);       //argv[6] - max range

    if(attack_type < 0 || attack_type > 4){
        printf("Wrong attack type, Exiting...\n");
        exit(1);
    }

    /* attack type: 0 -> flip, 1-> all zero, 2 -> all one, 3 -> random insn, 4-> random pc addition */

    // for threads
    int  iret1, iret2;
    pthread_t thread1, thread2;

    printf("---Starting ARM Cortex M3 Fault Injection\n");
    printf("---Number of iterations:               %d\n", itr);
    printf("---Seed Given:                         %d\n", seed);

    if(candidate == 1) printf("---Smartlock:                          A\n");
    if(candidate == 2) printf("---Smartlock:                          B\n");
    if(candidate == 3) printf("---Smartlock:                          C\n");

    if(attack_type == 0) printf("---Attack Type:      Instruction bit flip\n");
    if(attack_type == 1) printf("---Attack Type:      Instruction all zero\n");
    if(attack_type == 2) printf("---Attack Type:      Instruction all ones\n");
    if(attack_type == 3) printf("---Attack Type:      Instruction   random\n");
    if(attack_type == 4) printf("---Attack Type:      Random  PC  addition\n");
    printf("---Range:                         %d - %d\n", min_range, max_range);
    printf("--------------------------------------------------------------\n");

    srand(seed);

    int first_activation = 1;
    time_t first_time = time(NULL);
    char* c_first_time_string = ctime(&first_time);
    int rand_inst;
    int rand_pc;

    int success_count = 0, fail_count = 0, termination_count = 0;
    Attack_Profile *att_p_succ_array[10] = {NULL};

    // start iterating
    for(int i=1; i<=itr; i++){

        printf("Running Iteration Number: %d\n", i);


        FILE *capture = freopen("stderr.txt", "w", stderr);

        // create attack profile
        candidate = i%2 + 1;
        attack_type = i%5;

        Attack_Profile att_p = init_att_p(itr, seed, candidate, attack_type, min_range, max_range);
        printf("Cycle Count: %d\n", att_p.Cycle_Count);
        rand_inst = abs((rand() * 6373 *  21169 ) & 0xFFFF);
        rand_pc = abs((rand() * 251 ) % 20);
        //modify translate.c
        char str[MAX_LINE]; strcpy(str, "\0");
        const char substring1[MAX_LINE] = "######## look for this string ########";
        const char substring2[MAX_LINE] = "******** look for this string ********";
        const char substring3[MAX_LINE] = "^^^^^^^^^^ look for this string ^^^^^^^^^";
        FILE *fptr_read = fopen("./../../qemu_stm32/target-arm/translate.c", "r");
        FILE *fptr_write = fopen("./translate.c", "w");

        if(fptr_read == NULL){
            fptr_read = fopen("./../../../../qemu_stm32/target-arm/translate.c", "r");
            if(fptr_read == NULL){
                printf("FAILED TO OPEN READ FILE\n");
                exit(1);
            }
        }

        if(fptr_write == NULL){
            printf("FAILED TO OPEN WRITE FILE\n");
            exit(1);
        }


        while (!feof(fptr_read)){
                fgets(str, MAX_LINE, fptr_read);
                if(strstr(str, substring1)!=NULL){
                    fprintf(fptr_write,"int instruction_flag = %d, att_type = %d; // ######## look for this string ########\n", att_p.Cycle_Count, att_p.Type);
                }
                else if(strstr(str, substring2)!=NULL){
                    fprintf(fptr_write,"uint32_t random_16b_insn = %d;        // ******** look for this string ********\n", rand_inst);
                }
                else if(strstr(str, substring3)!=NULL){
                    fprintf(fptr_write,"target_ulong rand_pc = %d;         // ^^^^^^^^^^ look for this string ^^^^^^^^^\n", rand_pc);
                }
                else{
                    fprintf(fptr_write,"%s", str);
                }
        }


        fclose(fptr_read);
        fclose(fptr_write);

        // make simulation
        printf("Running candidate: %d\n",candidate);
        if(attack_type == 0) printf("Attack Type: Instruction bit flip\n");
        if(attack_type == 1) printf("Attack Type: Instruction all zero\n");
        if(attack_type == 2) printf("Attack Type: Instruction all ones\n");
        if(attack_type == 3) printf("Attack Type: Instruction   random\n");
        if(attack_type == 4) printf("Attack Type: Random  PC  addition\n");

        printf("Copying new translate.c...\n");
        system("\cp -rf ./translate.c ~/project/qemu_stm32/target-arm/translate.c");
        printf("Making new Qemu output\n");
        system("cd ~/project/qemu_stm32; make;");


        // make and run demo

        switch(candidate){
        case 1:
            iret1 = pthread_create( &thread1, NULL, RESP_SMARTLOCK_A, NULL);
            if(iret1){
                 printf("Error - pthread_create() return code: %d\n",iret1);
                 exit(EXIT_FAILURE);
             }
            break;
        case 2:
            iret1 = pthread_create( &thread1, NULL, RESP_SMARTLOCK_B, NULL);
            if(iret1){
                 printf("Error - pthread_create() return code: %d\n",iret1);
                 exit(EXIT_FAILURE);
             }
            break;
        case 3:
            iret1 = pthread_create( &thread1, NULL, RESP_SMARTLOCK_C, NULL);
            if(iret1){
                 printf("Error - pthread_create() return code: %d\n",iret1);
                 exit(EXIT_FAILURE);
             }
            break;
        }

        // read stderr to get /dev/pts/x
        sleep(5);
        fclose(capture);
        capture = fopen("stderr.txt", "r");
        if(capture){
            char buffer[MAX_LINE];
            while (fgets(buffer, MAX_LINE, capture) != NULL) {
                if(strstr(buffer, "/dev/pts/")){
                    get_UART_port(buffer);
                }
            }
        }
        // open thread to send to UART_port

        iret2 = pthread_create(&(thread2), NULL, cat_UART, UART_port);
        sleep(1);
        echo_UART(UART_port, "987654321");
        sleep(1);

        // find process and close
        //sleep(5);
        system("kill $(ps aux | grep 'qemu-system-arm' | awk '{print $2}')");
        char buffer[MAX_LINE];
        while (fgets(buffer, MAX_LINE, capture) != NULL) {
            if(strstr(buffer, "terminating on signal")){
                got_termination = 1;
            }
        }
        // write data to statistics file
        // statistics file
        FILE *statFptr = fopen("Statistics-Smartlock.txt", "a");
        printf("Writing Data to Statistics File\n");
        if(first_activation){
            fprintf(statFptr, "########################################################\n");
            fprintf(statFptr, "Starting a new simulation at: %s", c_first_time_string);
            fprintf(statFptr, "########################################################\n");
            first_activation = 0;
        }
        time_t current_time = time(NULL);
        char* c_time_string = ctime(&current_time);
        fprintf(statFptr, "Current time is %s\n", c_time_string);
        fprintf(statFptr, "Iteration Number: %d\n", i);

        if(got_success) {
            fprintf(statFptr, "Got SUCCESS\n");
            att_p_succ_array[success_count] = &att_p;
            success_count++;
        }

        if(got_fail) {
            fprintf(statFptr, "Got FAIL\n");
            fail_count++;
        }

        if(!got_success && !got_fail) {
            fprintf(statFptr, "Program Terminated\n");
            termination_count++;
        }

        fprintf(statFptr, "Attack Profile - Seed: %d, Cycle Count: %d, Register: %d\n", seed, att_p.Cycle_Count, att_p.Register);

        if(candidate == 1) fprintf(statFptr, "Smartlock: A\n");
        if(candidate == 2) fprintf(statFptr, "Smartlock: B\n");
        if(candidate == 3) fprintf(statFptr, "Smartlock: C\n");

        if(attack_type == 0) fprintf(statFptr, "Attack Type: Instruction bit flip\n");
        if(attack_type == 1) fprintf(statFptr, "Attack Type: Instruction all zero\n");
        if(attack_type == 2) fprintf(statFptr, "Attack Type: Instruction all ones\n");
        if(attack_type == 3) fprintf(statFptr, "Attack Type: Instruction   random\n");
        if(attack_type == 4) fprintf(statFptr, "Attack Type: Random  PC  addition\n");

        fprintf(statFptr, "--------------------------------------------------------------\n");
        fclose(statFptr);

        // exit
        fclose(capture);
        printf("Exiting Iteration Number: %d\n", i);
        printf("--------------------------------------------------------------\n");
    }

    printf("---Simulation Ended. Statistics Analysis:\n");
    printf("---Number of Fails: %d\n", fail_count);
    printf("---Number of Terminations: %d\n", termination_count);
    printf("---Number of Successes: %d\n", success_count);
    for(int i=0; i < success_count;i++){
        if(!i) printf("--------------------------------------------------------------\n"); //first iteration

        printf("Attack Profile - Seed: %d, Cycle Count: %d, Register: %d\n", att_p_succ_array[i]->seed, att_p_succ_array[i]->Cycle_Count, att_p_succ_array[i]->Register);

        if(att_p_succ_array[i]->candidate == 1) printf("Smartlock: A\n");
        if(att_p_succ_array[i]->candidate == 2) printf("Smartlock: B\n");
        if(att_p_succ_array[i]->candidate == 3) printf("Smartlock: C\n");

        if(att_p_succ_array[i]->Type == 0) printf("Attack Type: Instruction bit flip\n");
        if(att_p_succ_array[i]->Type == 1) printf("Attack Type: Instruction all zero\n");
        if(att_p_succ_array[i]->Type == 2) printf("Attack Type: Instruction all ones\n");
        if(att_p_succ_array[i]->Type == 3) printf("Attack Type: Instruction   random\n");
        if(att_p_succ_array[i]->Type == 4) printf("Attack Type: Random  PC  addition\n");
        printf("--------------------------------------------------------------\n");
    }


    return 0;
}


