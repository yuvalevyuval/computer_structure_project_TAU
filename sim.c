#define _CRT_SECURE_NO_WARNINGS
/*

		 computer structure final project:

				 A SIMP prossecor


Yuval Dori                               Yuval Levy
 203244066                               312238207


	   last modified    19:06  24.12.2020

*/

// define all are constats and libraries trughout the simulation
#define REG_IO_SIZE 18
#define REG_NUM_SIZE 16
#define LINE_MAX_SIZE 9
#define MEMORY_SIZE 4096
#define STORAGE_SIZE 16384 
#define $ZERO 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//define Command structure - a register that holds 5 fields and its easy to acsses each of them on the software level
typedef struct Command {
	unsigned int imm;          //39:20 bits   - 20 bits representing a number for calculations
	unsigned int OpCode;       //19:12 bits   - operation identifier
	unsigned int rd;           //11:08 bits   - destination register address
	unsigned int rs;           //07:04 bits   - source register address
	unsigned int rt;           //03:00 bits   - temporery register address
}Command;


//behold all of our gloriuos function
int read_memin(unsigned int* mem, char * address);
int read_diskin(unsigned int* disk, char * address);
int read_irq2in(unsigned int* irq2, char * address);
int IntExtand(int imm);
unsigned int GetByte(unsigned int num, int pos);
Command line_to_command(unsigned int inst);
void add(int * regs, Command cmd);
void sub(int * regs, Command cmd);
void and(int * regs, Command cmd);
void or (int * regs, Command cmd);
void sll(int * regs, Command cmd);
void sra(int * regs, Command cmd);
void srl(int * regs, Command cmd);
int beq(int* regs, Command cmd, int pc);
int bne(int* regs, Command cmd, int pc);
int blt(int* regs, Command cmd, int pc);
int bgt(int* regs, Command cmd, int pc);
int ble(int* regs, Command cmd, int pc);
int bge(int* regs, Command cmd, int pc);
int jal(int* regs, Command cmd, int pc);
void lw(int * regs, Command cmd, unsigned int * mem);
void sw(int * regs, Command cmd, unsigned int * mem);
int reti(int* io_regs, int pc, int* reti_flag);
void in(int* io_regs, int* regs, Command cmd);
void out(int* io_regs, int* regs, Command cmd, int* disk, int* mem);
int OPcode_To_execute(int regs[], int io_regs[], int pc, Command cmd, unsigned int * mem, int * disk, int* reti_flag);
void timer(int* io_regs);
void disk_handel(int* disk, int * io_regs, int* mem);
void update_irq2(int* io_regs, int* irq2, int counter);
int neg_to_pos(signed int num);
void create_regout(int regs[], char file_name[]);
void create_memout(unsigned int * mem, char file_name[]);
void create_diskout(unsigned int * disk, char file_name[]);
void create_cycles(int counter, char file_name[]);
void create_line_for_trace(char line_for_trace[], int regs[], int pc, unsigned int inst, int imm);
void create_line_for_hwregtrace(char line_for_hwregtrace[], int io_regs[], int regs[], int counter, Command cmd);
void create_line_for_display(char line_for_display[], int regs[], int io_regs[], int cycles, Command cmd);
void create_line_for_leds(char line_for_leds[], int regs[], int io_regs[], int cycles, Command cmd);
Command interrupt_handler(int* reg_io, int* regs, Command cmd, int* memory, int* PC, int* reti_on);
Command put_stall(Command cmd);

// we must handle interrupts when they occour, due to clock or errors
Command interrupt_handler(int* reg_io, int* regs, Command cmd, int* memory, int* PC, int* reti_on) {
	if (*reti_on) {
		int temp_PC = 0;                    //setting a temp integer to hold the PC for after the interrupt is over
		*reti_on = 0;                       //zeroing the flag
		reg_io[7] = *PC;                    //the 7th spot on the i/o register holds the current Program Counter
		*PC = reg_io[6];                    //our PC is being changed one spot back
		temp_PC = memory[reg_io[6]];        //
		return line_to_command(temp_PC);    //
	}
	//if reti isnt on then there's no need to interrupt
	else
		return cmd;
}

// a function that reads memin.txt and store it's content into an array.returns 1 if error occured, else returns 0.
int read_memin(unsigned int* mem, char * path)
{
	FILE *fp = fopen(path, "r"); // open memin file
	if (!fp) { // handle error
		printf("Error opening memin file\n");
		return 1;
	}

	// read memin file line by line and turn it into array
	char line[LINE_MAX_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp))          //while the file is not ended or reached max line 
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore white spaces
			continue;
		mem[i] = strtol(line, NULL, 16);
		i++;
	}
	fclose(fp);                                                 // closing the file
	return 0;
}


// a function that reads diskin.txt and store it's content into an array.returns 1 if error occured, else returns 0.
int read_diskin(unsigned int* disk, char * address)
{
	FILE *fp = fopen(address, "r"); // open diskin file
	if (!fp) { // handle error
		printf("Error opening diskin file\n");
		return 1;
	}

	// read diskin file line by line and turn it into array
	char line[LINE_MAX_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp))
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore white spaces
			continue;
		disk[i] = strtol(line, NULL, 16);
		i++;
	}
	fclose(fp); // close file
	return 0;
}


// a function that reads irq2in.txt and store it's content into an array.returns 1 if error occured, else returns 0.
int read_irq2in(unsigned int* irq2, char * address)
{
	FILE *fp = fopen(address, "r"); // open diskin file
	if (!fp) { // handle error
		printf("Error opening irq2in file\n");
		return 1;
	}

	// read diskin file line by line and turn it into array
	char line[LINE_MAX_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp))
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore white spaces
			continue;
		irq2[strtol(line, NULL, 10)] = 1;
		i++;
	}
	fclose(fp); // close file
	return 0;
}


//edds ones in front of imm
int IntExtand(int imm)
{
	int extanded = (0x00000FFF & imm);       //000000000000000000000000xxxxxxxxxxxx
	int mask = 0x00000800;                   //000000000000000000000000100000000000
	if (mask & imm)                          //if imm is negative extand it withe ones, else with zeros 
		extanded += 0xFFFFF000;              //111111111111111111111111xxxxxxxxxxxx
	return extanded;
}

//converts a neg number to pos in two's compliment represantation
int neg_to_pos(signed int num)
{
	num = abs(num);
	signed int mask = 0xffffffff;            //1111111111111111111111111111111111111111
	num = num ^ mask;                        //invert all bits
	num = num + 1;                            //turn into tow's compliment form
	return num;
}


// this function extracts one byte from number
unsigned int GetByte(unsigned int num, int place)
{
	unsigned int mask = 0xf;
	mask = mask << (place * 4);
	num = (num & mask);
	num = num >> (place * 4);
	return (num);
}


// this function creates a struct Command from a string in memory
Command line_to_command(unsigned int inst) {
	Command cmd;
	cmd.OpCode = (GetByte(inst, 7) * 16) + GetByte(inst, 6);
	cmd.rd = GetByte(inst, 5);
	cmd.rs = GetByte(inst, 4);
	cmd.rt = GetByte(inst, 3);
	cmd.imm = (GetByte(inst, 2) * 16 * 16) + (GetByte(inst, 1) * 16) + GetByte(inst, 0);

	//handle all out of bounds future problems
	if (cmd.OpCode < 7 || cmd.OpCode == 14 || cmd.OpCode == 17)        //if opcode arithmetic we need to check few expations
	{
		if (cmd.rd > 15 || cmd.rt > 15 || cmd.rs > 15 || cmd.rd == 1)
		{
			cmd = put_stall(cmd);
		}
		if (cmd.OpCode > 6 && cmd.OpCode < 13)                        //if opcode branch we need to check few expations
		{
			if (cmd.rd > 15 || cmd.rt > 15 || cmd.rs > 15)
			{
				cmd = put_stall(cmd);
			}
		}
		if (cmd.OpCode == 13)                                         // check only cmd.rd
		{
			if (cmd.rd > 15)
			{
				cmd = put_stall(cmd);
			}
		}
		if (cmd.OpCode == 15 || cmd.OpCode == 18)                     //if opcode sw check only registers
		{
			if (cmd.rd > 15 || cmd.rt > 15 || cmd.rs > 15) 
			{
				cmd = put_stall(cmd);
			}
		}
		if (cmd.OpCode > 19) 
		{                                        //how to handle error opcode that not exist
			cmd = put_stall(cmd);
		}
	}
	return cmd;
}


//these are all of the register level functions theat will be used in the simulation
void add(int* regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] + regs[cmd.rt];
}

void sub(int* regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] - regs[cmd.rt];
}

void and(int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] & regs[cmd.rt];
}

void or (int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] | regs[cmd.rt];
}

void xor(int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] ^ regs[cmd.rt];
}

void mul(int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] * regs[cmd.rt];
}

void sll(int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] << regs[cmd.rt];
}

void sra(int* regs, Command cmd)
{
	int mask = regs[cmd.rs] >> 31 << 31 >> (regs[cmd.rt]) << 1;
	regs[cmd.rd] = mask ^ (regs[cmd.rs] >> regs[cmd.rt]);
}

void srl(int* regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] >> regs[cmd.rt];
}

int beq(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] == regs[cmd.rt])
		return pc = regs[cmd.rd];
	else
		pc++;
	return pc;
}

int bne(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] != regs[cmd.rt])
	{
		pc = regs[cmd.rd];
		return pc;
	}
	else
	{
		pc++;
		return pc;
	}
}

int blt(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] < regs[cmd.rt])
	{
		pc = regs[cmd.rd];
		return pc;
	}
	else
	{
		pc++;
		return pc;
	}
}

int bgt(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] > regs[cmd.rt])
	{
		pc = regs[cmd.rd];
		return pc;
	}
	else
	{
		pc++;
		return pc;
	}
}

int ble(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] <= regs[cmd.rt])
	{
		pc = regs[cmd.rd];
		return pc;
	}
	else
	{
		pc++;
		return pc;
	}
}

int bge(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] >= regs[cmd.rt])
	{
		pc = regs[cmd.rd];
		return pc;
	}
	else
	{
		pc++;
		return pc;
	}
}

int jal(int* regs, Command cmd, int pc)
{
	regs[15] = pc + 1;
	pc = regs[cmd.rd];
	return pc;
}

void lw(int * regs, Command cmd, unsigned int * mem)
{
	if (regs[cmd.rs] + regs[cmd.rt] < MEMORY_SIZE)
		regs[cmd.rd] = mem[regs[cmd.rs] + regs[cmd.rt]];
}

void sw(int * regs, Command cmd, unsigned int * mem)
{
	if (regs[cmd.rs] + regs[cmd.rt] < MEMORY_SIZE)
		mem[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}

int reti(int* reg_io, int pc, int* reti_on)        //we use boolean flag to knowit reti is on or off
{
	if (*reti_on != 0)                             //if its on turn it off
		*reti_on = 0;
	else
		*reti_on = 1;                              //iff its 1 return it

	return reg_io[7];
}

void in(int* io_regs, int* regs, Command cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt] < 18)
	{
		regs[cmd.rd] = io_regs[regs[cmd.rs] + regs[cmd.rt]];
	}
}

void out(int* io_regs, int* regs, Command cmd, int* disk, int* mem)
{
	if (regs[cmd.rs] + regs[cmd.rt] < 18)
	{
		if ((regs[cmd.rs] + regs[cmd.rt]) == 14)
		{
			io_regs[14] = regs[cmd.rd];
			disk_handel(disk, io_regs, mem);
		}
	}
	else
		io_regs[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}

int halt()
{
	printf("Halt has been activated, exiting simulation");
	return -1;
}

// excution function for all the relevent opcode
// after every excution we have to check that $zero doesn't change his value 
int OPcode_To_execute(int regs[], int io_regs[], int pc, Command cmd, unsigned int * mem, int * disk, int* reti_flag)
{
	switch (cmd.OpCode)
	{
	case 0:
	{                  //add opcode
		add(regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;         //maybe need to check all zero case?
	}
	case 1:
	{                  //sub opcode
		sub(regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 2:
	{                  //and opcode
		and (regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 3:
	{                  //or opcode
		or (regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 4:
	{                  //xor opcode
		xor (regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 5:
	{                  //mul opcode
		mul(regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 6:
	{                  //sll opcode
		sll(regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 7:
	{                  //sra opcode
		sra(regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 8:
	{                  //srl opcode (need to verify)
		srl(regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 9:
	{                  //beq opcode

		pc = beq(regs, cmd, pc);
		regs[0] = $ZERO;
		break;
	}
	case 10:
	{                  //bne opcode
		pc = bne(regs, cmd, pc);
		regs[0] = $ZERO;
		break;
	}
	case 11:
	{                  //blt opcode
		pc = blt(regs, cmd, pc);
		regs[0] = $ZERO;
		break;
	}
	case 12:
	{                 //bgt opcode
		pc = bgt(regs, cmd, pc);
		regs[0] = $ZERO;
		break;
	}
	case 13:
	{                 //ble opcode
		pc = ble(regs, cmd, pc);
		regs[0] = $ZERO;
		break;
	}
	case 14:
	{                 //bge opcode
		pc = bge(regs, cmd, pc);
		regs[0] = $ZERO;
		break;
	}
	case 15:
	{                 //jal opcode
		pc = jal(regs, cmd, pc);
		regs[0] = $ZERO;
		break;
	}
	case 16:
	{                 //lw opcode
		lw(regs, cmd, mem);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 17:
	{                 //sw opcode
		sw(regs, cmd, mem);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 18:
	{                 //reti command
		pc = reti(io_regs, pc, reti_flag);
		break;
	}
	case 19:
	{                 //in command
		in(io_regs, regs, cmd);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 20:
	{                 //out command
		out(io_regs, regs, cmd, disk, mem);
		regs[0] = $ZERO;
		pc++;
		break;
	}
	case 21:
	{                  //halt command, we need to exit simulator
		pc = halt();
		break;
	}

	}
	return pc;
}

//timer function
void timer(int* io_regs)
{
	if (io_regs[11] == 1)
	{
		if (io_regs[12] == io_regs[13])
		{
			io_regs[3] = 1;
			io_regs[12] = 0;
		}
		else
			io_regs[12]++;
	}
}

// how to handel write\read from disk
void disk_handel(int* disk, int * io_regs, int* mem)
{
	if (io_regs[17] == 0) {
		io_regs[11] = 1;// enable timer	
		io_regs[13] = 1024;// set timermax to 1024
		io_regs[17] = 1;//disk is now busy
		switch (io_regs[14])
		{
		case 0:
		{
			io_regs[11] = 0;// disable timer	
			io_regs[13] = 0;// set timermax to 0
			io_regs[17] = 0;//disk is now free
			break;
		}
		case 1:
		{
			int i = 0;
			for (i = 0; i < 128; i++) {
				if ((io_regs[16] + i < MEMORY_SIZE) && (((128 * io_regs[15]) + i) < STORAGE_SIZE))
					mem[io_regs[16] + i] = disk[(128 * io_regs[15]) + i];
			}
			break;
		}
		case 2:
		{
			int i = 0;
			for (i = 0; i < 128; i++) {
				if ((io_regs[16] + i < MEMORY_SIZE) && (((128 * io_regs[15]) + i) < STORAGE_SIZE))
					disk[(128 * io_regs[15]) + i] = mem[io_regs[16] + i];
			}
			break;
		}
		}
	}
	if (io_regs[3] == 1) { // disk is done his work after 1024 cycles
		io_regs[14] = 0;
		io_regs[17] = 0;
		io_regs[4] = 1;
		io_regs[11] = 0;
		io_regs[3] = 0;
	}
}

//function that update the irq2status register
void update_irq2(int* io_regs, int* irq2, int counter)
{
	io_regs[5] = 0;
	if (counter > MEMORY_SIZE) 
	{
		io_regs[5] = 0;
	}
	else if (irq2[counter] != 0) 
	//else if (irq2[counter] != NULL)
	{
		io_regs[5] = 1;
	}
}


// this function creates regout file
void create_regout(int regs[], char file_name[]) {
	FILE* fp_regout;

	fp_regout = fopen(file_name, "w"); // open new file
	if (fp_regout == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	for (int i = 2; i <= 15; i++) // print registers to file
	{
		fprintf(fp_regout, "%08X\n", regs[i]);
	}
	fclose(fp_regout); // close file
}

// this function creates memout file
void create_memout(unsigned int * mem, char file_name[]) {
	FILE* fp_memout;
	fp_memout = fopen(file_name, "w"); // open new file
	if (fp_memout == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	for (int i = 0; i < MEMORY_SIZE; i++) // print memory to file
	{
		fprintf(fp_memout, "%08X\n", *mem);
		mem++;
	}
	fclose(fp_memout); // close file
}

// this function creates diskout file
void create_diskout(unsigned int * disk, char file_name[]) {
	FILE* fp_diskout;
	fp_diskout = fopen(file_name, "w"); // open new file
	if (fp_diskout == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	for (int i = 0; i < STORAGE_SIZE; i++) // print memory to file
	{
		fprintf(fp_diskout, "%08X\n", *disk);
		disk++;
	}
	fclose(fp_diskout); // close file
}

//create the cycles.txt file
void create_cycles(int counter, char file_name[]) {
	FILE* fp_cycles;
	fp_cycles = fopen(file_name, "w");
	if (fp_cycles == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	char c_counter[8] = { 0 };
	sprintf(c_counter, "%d", counter);//print the counter to file
	fputs(c_counter, fp_cycles);
	fclose(fp_cycles); // close file
}

// this function prepares a string to print to trace file
void create_line_for_trace(char line_for_trace[], int regs[], int pc, unsigned int inst, int imm)
{
	int i;
	char inst_line[9];
	char pc_char[10] = { 0 };
	char temp_reg_char[9] = { 0 };
	sprintf(pc_char, "%08X", pc);
	sprintf(inst_line, "%08X", inst);
	sprintf(line_for_trace, pc_char); //add pc to line
	sprintf(line_for_trace + strlen(line_for_trace), " ");
	sprintf(line_for_trace + strlen(line_for_trace), inst_line); //add opcode to line
	sprintf(line_for_trace + strlen(line_for_trace), " ");

	for (i = 0; i < 15; i++) { //add registers to line
		int temp_reg = 0;
		if (i == 1)// for imm
		{
			sprintf(temp_reg_char, "%08X", IntExtand(imm));//change to hex
			sprintf(line_for_trace + strlen(line_for_trace), temp_reg_char);//add to line
			sprintf(line_for_trace + strlen(line_for_trace), " ");
			continue;
		}
		if (regs[i] < 0)
			temp_reg = neg_to_pos(regs[i]);
		else
			temp_reg = regs[i];
		sprintf(temp_reg_char, "%08X", temp_reg);//change to hex
		sprintf(line_for_trace + strlen(line_for_trace), temp_reg_char);//add to line
		sprintf(line_for_trace + strlen(line_for_trace), " ");
	}

	//add last register to line (without space at the end)
	int temp_reg = 0;
	if (regs[i] < 0)
		temp_reg = neg_to_pos(regs[i]);
	else
		temp_reg = regs[i];
	sprintf(temp_reg_char, "%.8X", temp_reg);
	sprintf(line_for_trace + strlen(line_for_trace), temp_reg_char);
}

// create function that will colect data for hwregtrace
void create_line_for_hwregtrace(char line_for_hwregtrace[], int io_regs[], int regs[], int counter, Command cmd)
{
	char counter_char[10] = { 0 };
	char temp_reg_char[10] = { 0 };
	sprintf(counter_char, "%d", counter);
	sprintf(line_for_hwregtrace, counter_char); //add counter to line
	sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), " ");
	if (cmd.OpCode == 17)
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "READ "); //add read to line
	else
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "WRITE ");//add write to line
	switch (regs[cmd.rs] + regs[cmd.rt])
	{
	case 0:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq0enable "); //add register name to line
		break;
	}
	case 1:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq1enable "); //add register name to line
		break;
	}
	case 2:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq2enable "); //add register name to line
		break;
	}
	case 3:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq0status "); //add register name to line
		break;
	}
	case 4:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq1status "); //add register name to line
		break;
	}
	case 5:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq2status "); //add register name to line
		break;
	}
	case 6:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irqhandler "); //add register name to line
		break;
	}
	case 7:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irqreturn "); //add register name to line
		break;
	}
	case 8:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "clks "); //add register name to line
		break;
	}
	case 9:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "leds "); //add register name to line
		break;
	}
	case 10:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "display "); //add register name to line
		break;
	}
	case 11:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "timerenable "); //add register name to line
		break;
	}
	case 12:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "timercurrent "); //add register name to line
		break;
	}
	case 13:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "timermax "); //add register name to line
		break;
	}
	case 14:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "diskcmd "); //add register name to line
		break;
	}
	case 15:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "disksector "); //add register name to line
		break;
	}
	case 16:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "diskbuffer "); //add register name to line
		break;
	}
	case 17:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "diskstatus "); //add register name to line
		break;
	}
	}
	if (cmd.OpCode == 17)
	{
		sprintf(temp_reg_char, "%08X", io_regs[regs[cmd.rs] + regs[cmd.rt]]);
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), temp_reg_char); //add data to line
	}
	else
	{
		sprintf(temp_reg_char, "%08X", regs[cmd.rd]);
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), temp_reg_char); //add data to line
	}
}

//create display.txt
void create_line_for_display(char line_for_display[], int regs[], int io_regs[], int cycles, Command cmd)
{
	char clk_cycles[10];
	char curr_display[10];
	sprintf(clk_cycles, "%d", cycles);
	sprintf(curr_display, "%08X", regs[cmd.rd]);
	sprintf(line_for_display, clk_cycles); //add clk cycles to line
	sprintf(line_for_display + strlen(line_for_display), " ");// add space 
	sprintf(line_for_display + strlen(line_for_display), curr_display); //add current display to line
}

//create line for leds.txt file
void create_line_for_leds(char line_for_leds[], int regs[], int io_regs[], int cycles, Command cmd)
{
	char clk_cycles[10];
	char curr_leds[10];
	sprintf(clk_cycles, "%d", cycles);
	sprintf(curr_leds, "%08X", regs[cmd.rd]);
	sprintf(line_for_leds, clk_cycles); //add clk cycles to line
	sprintf(line_for_leds + strlen(line_for_leds), " ");// add space 
	sprintf(line_for_leds + strlen(line_for_leds), curr_leds); //add leds to line
}

// put stall when the comaand is not valid
Command put_stall(Command cmd)
{
	cmd.OpCode = 0;
	cmd.rd = 0;
	cmd.rs = 0;
	cmd.rt = 0;
	cmd.imm = 1;
	return cmd;
}

void Error_Handler(int id)
{
	switch (id)
	{
	case 1:
	{
		printf("Error while initializing simulation due to 'memin' read issue");
			break;
	}
	case 2:
	{
		printf("Error while initializing loading simulation due to 'diskin' read issue");
			break;
	}
	case 3:
	{
		printf("Error while initializing loading simulation due to 'irq2in' read issue");
			break;
	}
	case 4:
	{
		printf("Error while loading simulation due to a NULL fp_trace");
			break;
	}
	case 5:
	{
		printf("Error while loading simulation due to a NULL fp_hwregtrace");
			break;
	}
	case 6:
	{
		printf("Error while loading simulation due to a NULL fp_leds");
			break;
	}
	case 7:
	{
		printf("Error while loading simulation due to a NULL fp_display");
			break;
	}
	}
	exit(1);
}

int main(int argc, char* argv[])
{
	int regs[REG_NUM_SIZE] = { 0 };                                                          //initialize register
	int io_regs[REG_IO_SIZE] = { 0 };                                                        //initialize input output register
	int counter = 0;                                                                         //initialize counter
	int pc = 0;                                                                              //initialize pc
	int* pc_ptr = &pc;;                                                                      //initialize pc pointer
	int reti = 1;                                                                            //setting reti to 1
	int* reti_flag = &reti;                                                                  //initialize flag for interrupt to know if reti done
	unsigned int mem[MEMORY_SIZE] = { 0 };                                                   //initialize memory
	unsigned int disk[STORAGE_SIZE] = { 0 };                                                 //initialize disk
	unsigned int irq2[MEMORY_SIZE] = { 0 };                                                  //initialize irq 2

	//initializing Error handler
	if (read_memin(mem, argv[1]) != 0)
		Error_Handler(1);
	if (read_diskin(disk, argv[2]) != 0)
		Error_Handler(2);
	if (read_irq2in(irq2, argv[3]) != 0)
		Error_Handler(3);

	FILE* fp_trace;                                                                          //define pointer for writing trace file
	FILE* fp_hwregtrace;                                                                     //define pointer for writing hwregtrace file
	FILE* fp_leds;                                                                           //define pointer for writing leds file
	FILE* fp_display;                                                                        //define pointer for writing display file
	fp_trace = fopen(argv[6], "w");
	fp_hwregtrace = fopen(argv[7], "w");
	fp_leds = fopen(argv[9], "w");
	fp_display = fopen(argv[10], "w");

	//loading Error handler
	if (fp_trace == NULL)
		Error_Handler(4);
	if (fp_hwregtrace == NULL)
		Error_Handler(5);
	if (fp_leds == NULL)
		Error_Handler(6);
	if (fp_display == NULL)
		Error_Handler(7);

	// Execution

	unsigned int inst;                                                                       //define instruction number
	while (pc > 0)                                                                           //use the pc as flag for halt function
	{

		if (pc <= MEMORY_SIZE - 1)
			inst = mem[pc];
		Command cmd = line_to_command(inst);                                                 //create Command struct
		if ((io_regs[0] && io_regs[3]) || (io_regs[1] && io_regs[4]) || (io_regs[2] && io_regs[5]))
		{
			cmd = interrupt_handler(io_regs, regs, cmd, mem, pc_ptr, reti_flag);            //we have irq==1 and need to handle it	
			if (pc <= MEMORY_SIZE - 1)
				inst = mem[pc];
		}
		char line_for_trace[200] = { 0 };                                                    //create line for trace file
		char line_for_leds[20] = { 0 };                                                      //create line for leds file
		char line_for_display[20] = { 0 };                                                   //create line for display file
		char line_for_hwregtrace[100] = { 0 };                                               //create line for hwregtrace file
		regs[1] = IntExtand(cmd.imm);                                                        //first we do sign extend to immiediate
		update_irq2(io_regs, irq2, counter);                                                 //update irq2status register
		timer(io_regs);                                                                      //check if timer is enable
		if (io_regs[17] == 1)                                                                //check if disk is still busy
			disk_handel(disk, io_regs, mem);
		if (cmd.OpCode == 17 || cmd.OpCode == 18)
		{
			create_line_for_hwregtrace(line_for_hwregtrace, io_regs, regs, counter, cmd);    //append to trace file
			fprintf(fp_hwregtrace, "%s\n", line_for_hwregtrace);
		}
		create_line_for_trace(line_for_trace, regs, pc, inst, cmd.imm);                      //append to trace file
		fprintf(fp_trace, "%s\n", line_for_trace);
		if ((regs[cmd.rs] + regs[cmd.rt]) == 9 && cmd.OpCode == 18) {
			if (regs[cmd.rd] != io_regs[regs[cmd.rs] + regs[cmd.rt]])
			{
				create_line_for_leds(line_for_leds, regs, io_regs, counter, cmd);            //append to leds file
				fprintf(fp_leds, "%s\n", line_for_leds);
			}
		}
		if ((regs[cmd.rs] + regs[cmd.rt]) == 10 && cmd.OpCode == 18) {
			if (regs[cmd.rd] != io_regs[regs[cmd.rs] + regs[cmd.rt]])
			{
				create_line_for_display(line_for_display, regs, io_regs, counter, cmd);      //append to display file
				fprintf(fp_display, "%s\n", line_for_display);
			}
		}
		pc = OPcode_To_execute(regs, io_regs, pc, cmd, mem, disk, reti_flag);                //execute instruction
		io_regs[8] = counter++;                                                              //clk cycle counter
	}
	create_memout(mem, argv[4]);                                                             //create memout file
	create_regout(regs, argv[5]);                                                            //create regout file
	create_cycles(counter, argv[8]);                                                         //create cycles file
	create_diskout(disk, argv[11]);                                                          //create cycles file
	fclose(fp_trace);                                                                        //close trace file
	fclose(fp_hwregtrace);                                                                   //close hwregtrace file
	fclose(fp_leds);                                                                         //close leds file
	fclose(fp_display);                                                                      //close display file
	return 0;
}
