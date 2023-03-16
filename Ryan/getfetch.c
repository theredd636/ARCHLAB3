#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-riscv.h"
/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-RISCV Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}
int binary[32];
Register Reg[32];
/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE.PC+=4;
	INSTRUCTION_COUNT++;
}

/***************************************************************/
/* Simulate RISCV for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}
//instructuonaligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");	
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-RISCV SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-RISCV! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_instruction(CURRENT_STATE.PC); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

int binaryToInt2(int* input,int size){
    int count=1;
    int total=0;
    for (int i=size;i>=0;i--){  
        total+=input[i]*count;
        if(i<size){
             count*=2;
        }
    }
    return total;
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < RISCV_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size= MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* decode and execute instruction                                                                     */ 
/***********q*************************************************/
void add(int rd,int rs1,int rs2){
	int *x,*y;
	int *temp;
	x=Reg[rs1].data;
	y=Reg[rs2].data;
	temp=Reg[rd].data;
	*temp=*x+*y;
}
void sub(int rd,int rs1,int rs2){
	int *x,*y;
	int *temp;
	x=Reg[rs1].data;
	y=Reg[rs2].data;
	temp=Reg[rd].data;
	*temp=*x-*y;
}
void and(int rd,int rs1,int rs2){
	int *x,*y,*temp;
	x=Reg[rs1].data;
	y=Reg[rs2].data;
	temp=Reg[rd].data;
	if(x && y){
		*temp=1;
	}
}
void or(int rd,int rs1,int rs2){
	int *x,*y,*temp;
	x=Reg[rs1].data;
	y=Reg[rs2].data;
	temp=Reg[rd].data;
	if(x || y){
		*temp=1;
	}
}
void load(int ime,int rs1,int rd){
	int *temp,*x;
	x=Reg[rs1].data;
	temp=Reg[rd].data;
	*temp=*x+ime;
}
void store(int ime,int rs1,int rs2){
	int *temp,*x;
	x=Reg[rs1+ime].data;
	temp=Reg[rs2].data;
	*x=*temp; 
}
char * handleI(int funct3,char *instName){
	 // this is the funct 3 of the instruction
	int imm[12],ali,RD[5],RS1[5],nuts,crack;
	imm[0]=binary[19];
	imm[1]=binary[20];
	imm[2]=binary[21];
	imm[3]=binary[22];
	imm[4]=binary[23];
	imm[5]=binary[24];
	imm[6]=binary[25];
	imm[7]=binary[26];
	imm[8]=binary[27];
	imm[9]=binary[28];
	imm[10]=binary[29];
	imm[11]=binary[31];
	RD[0]=binary[7];
	RD[1]=binary[8];
	RD[2]=binary[9];
	RD[3]=binary[10];
	RD[4]=binary[11];
	RS1[0]=binary[15];
	RS1[1]=binary[16];
	RS1[2]=binary[17];
	RS1[3]=binary[18];
	RS1[4]=binary[19];
	ali=binaryToInt(imm,12);
	nuts=binaryToInt(RD,5);
	crack=binaryToInt(RS1,5);
	switch (funct3)	{
		case 0:
		//addi or lbunsigned
			instName="addi";
			return instName;
			printf("addi\n");
			break;
		case 1:
		//slti or lw
		//
			instName="loadw";
			load(ali,nuts,crack);
			return instName;
			break;
		case 3:
		//sltiu
			break;
		case 4:
		//xori or lbu
			break;
		case 5:
		// srli or srai or lhu
			break;
		case 6:
		//ori
			break;
		case 7:
		//andi
			break;
	}
}

char *handleR(int funct3,int funct7,char *instName){
	int RS2[5],RS1[5],RD[5],Ryan,Zack,Jack;
	RD[0]=binary[7];
	RD[1]=binary[8];
	RD[2]=binary[9];
	RD[3]=binary[10];
	RD[4]=binary[11];
	RS1[0]=binary[15];
	RS1[1]=binary[16];
	RS1[2]=binary[17];
	RS1[3]=binary[18];
	RS1[4]=binary[19];
	RS2[0]=binary[20];
	RS2[1]=binary[21];
	RS2[2]=binary[22];
	RS2[3]=binary[23];
	RS2[4]=binary[24];
	Ryan=binaryToInt(RD,5);
	Zack=binaryToInt(RS1,5);
	Jack=binaryToInt(RS2,5);
	switch(funct3){
		case 0:
			//add or sub
			if(funct7==0){
				instName="add";
				add(Ryan,Zack,Jack); 
				return instName;
			}
			else{
				instName="sub";
				sub(Ryan,Zack,Jack);
				return instName;
			}
			break;
		case 2:
			//or
			instName="or";
			or(Ryan,Zack,Jack);
			return instName;
			break;
		case 4:
			//and
			instName="and";
			and(Ryan,Zack,Jack);
			return instName;
			break;
		default:
			printf("no function found\n");
			return "NF";
			break;


	}
}
char *handleS(int funct3,char *instName){
	int imm[12],RS1[5],RS2[5],num1,num2,num3;
	imm[0]=binary[7];
	imm[1]=binary[8];
	imm[2]=binary[9];
	imm[3]=binary[10];
	imm[4]=binary[11];
	imm[5]=binary[24];
	imm[6]=binary[25];
	imm[7]=binary[26];
	imm[8]=binary[27];
	imm[9]=binary[28];
	imm[10]=binary[29];
	imm[11]=binary[31];
	RS2[0]=binary[20];
	RS2[1]=binary[21];
	RS2[2]=binary[22];
	RS2[3]=binary[23];
	RS2[4]=binary[24];
	RS1[0]=binary[15];
	RS1[1]=binary[16];
	RS1[2]=binary[17];
	RS1[3]=binary[18];
	RS1[4]=binary[19];
	num1=binaryToInt(imm,12);
	num2=binaryToInt(RS1,5);
	num3=binaryToInt(RS2,5);
	switch (funct3){
		case 0:
			//sb
			break;
		case 2:
			//sh
			break;
		case 1:
			//sw
			instName="storew";
			store(num1,num2,num3);
			return instName;
			break;
	}	
}
char *handleB(int funct3,char *instName){

}
void handleJALR(int funct3,char *instName){

}

int binaryToInt(int* input,int size){
    int count=1;
    int total=0;
    for (int i=0;i<size;i++){
        total+=input[i]*count;
        if(i>0){
             count*=2;
        }
    }
    return total;
}

void handle_instruction(){
	/*IMPLEMENT THIS*/
	int instruction=mem_read_32(CURRENT_STATE.PC);
	int Sam=instruction;
	//print_program(Sam);
	//int hex=printBits(sizeof(instruction),&instruction);
	for(int craig=0; craig<32;craig++){
		binary[craig]=0;
	}
	int i=0;
	while(instruction>0){
		if(instruction%2==0){
			binary[i]=0;
		}
		binary[i++]=instruction%2;
		instruction/=2;
	}
	int op[7];
	i=32;
	int y=0;
	for(int w= i-26; w>=0; w--)
	{
		op[y] = binary[w];
		y++;
	}
	int funct3[3];
	funct3[0] = binary[14];
	funct3[1] = binary[13];
	funct3[2] = binary[12];
	char *instName;
	int opcode=binaryToInt(op,7);
	printf("%d\n",opcode);
	int f3=binaryToInt(funct3,3);
	printf("f3 %d\n",f3);

	    if(opcode==50){
			instName=handleI(f3,instName);
		}
	    if (opcode== 51){
			int func7[7];
			func7[0] = binary[24];
			func7[1] = binary[25];
			func7[2] = binary[26];
			func7[3] = binary[27];
			func7[4] = binary[28];
			func7[5] = binary[29]; // = 1 for sub
			func7[6] = binary[30];
			int f7=binaryToInt(func7,7);
			instName=handleR(f3,f7,instName);	
		}
	    if (opcode==48){
	        // this case is an I instruction
			instName=handleI(f3,instName);
		}
	    if(opcode==49){
	        // this cas is a S instruction
			instName=handleS(f3,instName);
		}
	    if(opcode==99){
	        // this case is a B insruction
			handleB(f3,instName);
		}
		if(opcode==0){
			instName=handleB(f3,instName);
		}
		if(opcode==12){
			RUN_FLAG=FALSE;
			instName="end";
		}
		puts("");
		printf("instName %s\n",instName);
		if(Sam==0){
			RUN_FLAG=FALSE;
		}
}

/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in RISCV assembly format)    */ 
/************************************************************/

/************************************************************/
/* Print the instruction at given memory address (in RISCV assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/
	//printf("WELCOME TO THE PRINT FUNCTION\n");
	int num=mem_read_32(addr);
	int f=num;
	int i=0;
	for(int craig=0; craig<32;craig++){
		binary[craig]=0;
	}
	while(num>0){
		if(num%2==0){
			binary[i]=0;
		}
		binary[i++]=num%2;
		num/=2;
	}
	i=32;
	puts("");
	int op[7];
	i=32;
	int y=0;
	for(int w= i-26; w>=0; w--)
	{
		op[y] = binary[w];
		y++;
	}
	int funct3[3];
	funct3[0] = binary[14];
	funct3[1] = binary[13];
	funct3[2] = binary[12];
	char *instName;
	int opcode=binaryToInt(op,7);
//	printf("op%d\n",opcode);
	int f3=binaryToInt(funct3,3);
	//printf("f3 %d\n",f3);
	if (opcode== 51){
		int func7[7];
		func7[0] = binary[24];
		func7[1] = binary[25];
		func7[2] = binary[26];
		func7[3] = binary[27];
		func7[4] = binary[28];
		func7[5] = binary[29]; // = 1 for sub
		func7[6] = binary[30];

		int RS2[5],RS1[5],RD[5],Ryan,Zack,Jack;
		RD[0]=binary[7];
		RD[1]=binary[8];
		RD[2]=binary[9];
		RD[3]=binary[10];
		RD[4]=binary[11];
		RS1[0]=binary[15];
		RS1[1]=binary[16];
		RS1[2]=binary[17];
		RS1[3]=binary[18];
		RS1[4]=binary[19];
		RS2[0]=binary[20];
		RS2[1]=binary[21];
		RS2[2]=binary[22];
		RS2[3]=binary[23];
		RS2[4]=binary[24];
		Ryan=binaryToInt(RD,5);
		Zack=binaryToInt(RS1,5);
		Jack=binaryToInt(RS2,5);

		int f7=binaryToInt(func7,7);
		switch (f3){
			case 0:
				if(f7==0){
					instName="add";
					printf("ADD x%d,x%d,x%d" , Ryan,Zack,Jack);
			}else{
				instName="sub";
				printf("SUB x%d,x%d,x%d" , Ryan,Zack,Jack);
			}
				break;
			case 2:
				instName="or";
				printf("OR x%d,x%d,x%d" , Ryan,Zack,Jack);
				break;
			case 4:
				instName="and";
				printf("AND x%d,x%d,x%d" , Ryan,Zack,Jack);
				break;
			default:
				printf("no function found\n");
				break;
			}
		}
	    if (opcode==48){
	        // this case is an I instruction
			int imm[12],ali,RD[5],RS1[5],nuts,crack;
			imm[0]=binary[19];
			imm[1]=binary[20];
			imm[2]=binary[21];
			imm[3]=binary[22];
			imm[4]=binary[23];
			imm[5]=binary[24];
			imm[6]=binary[25];
			imm[7]=binary[26];
			imm[8]=binary[27];
			imm[9]=binary[28];
			imm[10]=binary[29];
			imm[11]=binary[31];
			RD[0]=binary[7];
			RD[1]=binary[8];
			RD[2]=binary[9];
			RD[3]=binary[10];
			RD[4]=binary[11];
			RS1[0]=binary[15];
			RS1[1]=binary[16];
			RS1[2]=binary[17];
			RS1[3]=binary[18];
			RS1[4]=binary[19];
			ali=binaryToInt(imm,12);
			nuts=binaryToInt(RD,5);
			crack=binaryToInt(RS1,5);
			printf("LW x%d,%d(x%d)",crack,ali,nuts);
		}
	    if(opcode==49){
	        // this cas is a S instruction
			int imm[12],RS1[5],RS2[5],num1,num2,num3;
			imm[0]=binary[7];
			imm[1]=binary[8];
			imm[2]=binary[9];
			imm[3]=binary[10];
			imm[4]=binary[11];
			imm[5]=binary[24];
			imm[6]=binary[25];
			imm[7]=binary[26];
			imm[8]=binary[27];
			imm[9]=binary[28];
			imm[10]=binary[29];
			imm[11]=binary[31];
			RS2[0]=binary[20];
			RS2[1]=binary[21];
			RS2[2]=binary[22];
			RS2[3]=binary[23];
			RS2[4]=binary[24];
			RS1[0]=binary[15];
			RS1[1]=binary[16];
			RS1[2]=binary[17];
			RS1[3]=binary[18];
			RS1[4]=binary[19];
			num1=binaryToInt(imm,12);
			num2=binaryToInt(RS1,5);
			num3=binaryToInt(RS2,5);
			printf("SW x%d,%d(x%d)",num3,num1,num2);
		}
	    if(opcode==99){
	        // this case is a B insruction
			handleB(f3,instName);
		}
		if(opcode==12){
			RUN_FLAG=FALSE;
			instName="end";
		}
	addr+=4;
	if(f!=0){
		print_instruction(addr);
	}else{
		printf("//////////////////////////////////// \n");
		printf("End of print functions\n");
		printf("//////////////////////////////////// \n");
	}
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {
	for(int Chris=0;Chris<32;Chris++){
		Reg[Chris].data=malloc(sizeof(void*));
	}                             
	int *p;
	p=Reg[4].data;
	*p=5;
	printf("\n**************************\n");
	printf("Welcome to MU-RISCV SIM...\n");
	printf("**************************\n\n");
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
