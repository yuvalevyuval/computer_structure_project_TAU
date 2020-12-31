#define _CRT_SECURE_NO_WARNINGS
/*
			   Tel Aviv University
			 Faculty  of engineering



		 computer structure final project:



					   A
					S.I.M.P.
				   processor.








Yuval Dori                               Yuval Levy
203244066                                 312238207



	  last modified   19:14:05  29.12.2020
*/

// define all are constats and libraries trughout the simulation

#define REG_IO_SIZE 18
#define REG_NUM_SIZE 16
#define LINE_MAX_SIZE 9
#define CLK_Hz 1024
#define MEMORY_SIZE 4096
#define STORAGE_SIZE 16384 
#define $ZERO 0
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//define Command structure - a register that holds 5 fields and its easy to acsses each of them on the software level
typedef struct Command
{
	unsigned int imm;          //39:20 bits   - 20 bits representing a number for calculations
	unsigned int OpCode;       //19:12 bits   - operation identifier
	unsigned int rd;           //11:08 bits   - destination register address
	unsigned int rs;           //07:04 bits   - source register address
	unsigned int rt;           //03:00 bits   - temporery register address
}Command;

//behold all of our gloriuos function
int read_imemin(unsigned int* mem, char * address);       // no
int read_dmemin(unsigned int* mem, char * address);
int read_diskin(unsigned int* disk, char * address);     // no
int read_irq2in(unsigned int* irq2, char * address);     // no
int file_to_array(unsigned int* arr, char* path);
int IntExtand(int imm);
unsigned int GetByte(unsigned int num, int pos);
Command line_to_command(unsigned int inst);
void add(int* regs, Command cmd);
void sub(int* regs, Command cmd);
void and(int* regs, Command cmd);
void or (int* regs, Command cmd);
void sll(int* regs, Command cmd);
void sra(int* regs, Command cmd);
void srl(int* regs, Command cmd);
int beq(int* regs, Command cmd, int pc);
int bne(int* regs, Command cmd, int pc);
int blt(int* regs, Command cmd, int pc);
int bgt(int* regs, Command cmd, int pc);
int ble(int* regs, Command cmd, int pc);
int bge(int* regs, Command cmd, int pc);
int jal(int* regs, Command cmd, int pc);
void lw(int* regs, Command cmd, unsigned int * mem);
void sw(int* regs, Command cmd, unsigned int * mem);
int reti(int* io_regs, int pc, int* reti_flag);
void in(int* io_regs, int* regs, Command cmd);
void out(int* io_regs, int* regs, Command cmd, int* disk, int* mem);
int OPcode_To_execute(int regs[], int io_regs[], int pc, Command cmd, unsigned int * mem, int * disk, int* reti_flag);
void timer(int* io_regs);
void disk_handler(int* disk, int* io_regs, int* mem);
void update_irq2(int* io_regs, int* irq2, int counter);
int ABS(signed int num);
void export_regout(int regs[], char file_name[]);
void create_memout(unsigned int* mem, char file_name[]);
void export_diskout(unsigned int* disk, char file_name[]);
void export_cycles(int counter, char file_name[]);
void create_line_for_trace(char line_for_trace[], int regs[], int pc, unsigned int inst, int imm);
void create_line_for_hwregtrace(char line_for_hwregtrace[], int io_regs[], int regs[], int counter, Command cmd);
void create_line_for_monitor(char line_for_display[], int regs[], int io_regs[], int cycles, Command cmd);
void create_line_for_monitor_yuv(char line_for_display[], int regs[], int io_regs[], int cycles, Command cmd);
void create_line_for_leds(char line_for_leds[], int regs[], int io_regs[], int cycles, Command cmd);
Command interrupt_handler(int* reg_io, int* regs, Command cmd, int* memory, int* PC, int* reti_on);
Command Invalid_cmd(Command cmd);

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

int file_to_array(unsigned int* arr, char * path)
{
	FILE *fp = fopen(path, "r");                                      // open file
	if (!fp)                                                          // handle error
		return -1;

	char line[LINE_MAX_SIZE];                                         // read file line by line and turn it into array
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp))               //while the file is not ended or reached max line 
	{
		if ((strcmp(line, "\n") == 0) || (strcmp(line, "\0") == 0))   // ignore white spaces
			continue;
		arr[i] = strtol(line, NULL, 16);
		i++;
	}
	
	fclose(fp);                                                       // closing the file
	return 0;
}


//read imemin line by line and store it on "path" array
int read_imemin(unsigned int* imem, char * path)
{
	FILE *fp = fopen(path, "r");                                // open memin file
	if (!fp)                                                    // handle error
		return -1;

	// read imemin file line by line and turn it into array
	char line[LINE_MAX_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp) && (i <= MEMORY_SIZE))         //while the file is not ended or reached max line
	{
		if ((strcmp(line,"\n")==0) || (strcmp(line,"\0")==0))  // ignore white spaces
			continue;
		imem[i] = strtol(line, NULL, 16);
		i++;
	}
	fclose(fp);                                                 // closing the file
	return 0;
}

//read dmemin line by line and store it on "path" array
int read_dmemin(unsigned int* dmem, char * path)
{
	FILE *fp = fopen(path, "r");                                // open memin file
	if (!fp)                                                    // handle error
		return -1;

	// read dmemin file line by line and turn it into array
	char line[LINE_MAX_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp) && ( i <= STORAGE_SIZE))         
	{                                                           //while the file is not ended or reached max line
		if ((strcmp(line, "\n") == 0) || (strcmp(line, "\0") == 0))  // ignore white spaces
			continue;
		dmem[i] = strtol(line, NULL, 16);
		i++;
	}
	fclose(fp);                                                 // closing the file
	return 0;
}

//read diskin line by line and store it on "disk" array
int read_diskin(unsigned int* disk, char * address)
{
	FILE *fp = fopen(address, "r");                             // open diskin file
	if (!fp)                                                    // handle error
		return - 1;                                             //if an Error accoured, we return -1

	char line[LINE_MAX_SIZE];                                   // read diskin file line by line and turn it into array
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp) && (i <= STORAGE_SIZE))
	{
		if ((strcmp(line, "\n") == 0) || (strcmp(line, "\0") == 0))  // ignore white spaces
			continue;
		disk[i] = strtol(line, NULL, 16);
		i++;
	}
	fclose(fp); // close file
	return 0;
}


//read irg2in line by line and store it on "irq2" array
int read_irq2in(unsigned int* irq2, char * address)
{
	FILE *fp = fopen(address, "r");                              // open diskin file
	if (!fp)                                                     // handle error
		return -1;                                               //if an Error accoured, we return -1

	char line[LINE_MAX_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp))
	{
		if ((strcmp(line,"\n")== 0) || (strcmp(line,"\0")== 0))
			continue;                                                // ignore white spaces
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
int ABS(signed int num)
{
	num = abs(num);
	signed int mask = 0xffffffff;            //1111111111111111111111111111111111111111
	num = num ^ mask;                        //invert all bits
	num = num + 1;                           //turn into tow's compliment form
	return num;
}

//extracts one byte from number and a given position
unsigned int GetByte(unsigned int num, int pos)
{
	unsigned int mask = 0xf;                 //using 2 way shifting and a mask on the 4*pos LSB we get the desired byte
	mask = mask << (4 * pos);
	num = (num & mask);
	num = num >> (4 * pos);
	return (num);
}

//we Pharse on a line and using GetByte extract its commands
Command line_to_command(unsigned int inst)
{
	// a line for example : 047A5000   op = 04, rd = 7, rs = A, rt = 5, imm = 000
	Command cmd;                                                                         //initialize an empty commant to fill with the line from memory
	cmd.OpCode = (GetByte(inst, 7) * 16) + GetByte(inst, 6);                             //opcode is in the 2 first bytes of the line  
	cmd.rd = GetByte(inst, 5);                                                           //rd register is in the 5th byte of the line  
	cmd.rs = GetByte(inst, 4);                                                           //rs register is in the 4th byte of the line  
	cmd.rt = GetByte(inst, 3);                                                           //rt register is in the 3rd byte of the line  
	cmd.imm = (GetByte(inst, 2) * 16 * 16) + (GetByte(inst, 1) * 16) + GetByte(inst, 0); //the 3rd lsb bits are saved for imm 

	//in case we have any non valid input in the given memory, we use Invalid_cmd() to handle Errors
	if ((cmd.OpCode < 7) || (cmd.OpCode == 14) || (cmd.OpCode == 17))     //if opcode arithmetic we need to check few expations
	{
		if ((cmd.rd > 15) || (cmd.rt > 15) || (cmd.rs > 15) || (cmd.rd == 1) || (cmd.OpCode > 19))
			cmd = Invalid_cmd(cmd);

		if ((cmd.OpCode == 13) && (cmd.rd > 15))                          // check only cmd.rd
			cmd = Invalid_cmd(cmd);

		if ((cmd.OpCode > 6) && (cmd.OpCode < 13))                        //if opcode branch we need to check few expations
		{
			if ((cmd.rd > 15) || (cmd.rt > 15) || (cmd.rs > 15))
				cmd = Invalid_cmd(cmd);
		}
		if ((cmd.OpCode == 15) || (cmd.OpCode == 18))                     //if opcode sw check only registers
		{
			if ((cmd.rd > 15) || (cmd.rt > 15) || (cmd.rs > 15))
				cmd = Invalid_cmd(cmd);
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
		pc = regs[cmd.rd];
	else
		pc++;

	return pc;
}
int blt(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] < regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;

	return pc;
}
int bgt(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] > regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;

	return pc;
}
int ble(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] <= regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;

	return pc;
}
int bge(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] >= regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;

	return pc;
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
			disk_handler(disk, io_regs, mem);
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
int OPcode_To_execute(int regs[], int io_regs[], int pc, Command cmd, unsigned int * mem, int * disk, int* reti_flag)
{
	switch (cmd.OpCode) // after every excution we have to check that $zero doesn't change his value
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
		pc = halt();   //pc is now -1
		break;
	}
	}
	return pc;
}

void timer(int* io_regs)
{
	if (io_regs[11] == 1)                   //io_regs[11] is the timer enabler
	{
		if (io_regs[12] == io_regs[13])     // need to zero the timer
		{
			io_regs[3] = 1;
			io_regs[12] = 0;
		}
		else
			io_regs[12]++;                  //just increment the timer
	}
}

// how to handel write and read from the disk
void disk_handler(int* disk, int * io_regs, int* mem)
{
	if (io_regs[17] == 0) {
		io_regs[11] = 1;                  //enable timer	
		io_regs[13] = CLK_Hz;             //set timermax to 1024
		io_regs[17] = 1;                  //disk is now busy
		switch (io_regs[14])              // io_regs[14] is set to be regs[cmd.rd] in "out" command
		{
		case 0:
		{
			io_regs[11] = 0;              //disable timer	
			io_regs[13] = 0;              //set timermax to 0
			io_regs[17] = 0;              //disk is now free
			break;
		}
		case 1:
		{
			int i = 0;
			for (i = 0; i < 128; i++)
			{
				if ((io_regs[16] + i < MEMORY_SIZE) && (((128 * io_regs[15]) + i) < STORAGE_SIZE))
					mem[io_regs[16] + i] = disk[(128 * io_regs[15]) + i];
			}
			break;
		}
		case 2:
		{
			int i = 0;
			for (i = 0; i < 128; i++)
			{
				if ((io_regs[16] + i < MEMORY_SIZE) && (((128 * io_regs[15]) + i) < STORAGE_SIZE))
					disk[(128 * io_regs[15]) + i] = mem[io_regs[16] + i];
			}
			break;
		}
		}
	}
	if (io_regs[3] == 1)         // disk is done his work after 1024 cycles
	{
		io_regs[14] = 0;
		io_regs[17] = 0;
		io_regs[4] = 1;
		io_regs[11] = 0;
		io_regs[3] = 0;
	}
}

//update the irq2status register
void update_irq2(int* io_regs, int* irq2, int counter)
{
	io_regs[5] = 0;                 //??? maybe delete this line
	if (counter > MEMORY_SIZE)      //if we passed memory size then zero the irq2status register
		io_regs[5] = 0;
	// maybe - else if (irq2[counter] != NULL
	else if (irq2[counter] != 0)    //if we haven't maxed the counter and it is not zero, set the irq2status register to 1
		io_regs[5] = 1;

}

// this function creates regout file
void export_regout(int regs[], char file_name[]) {
	FILE* fp_regout;
	fp_regout = fopen(file_name, "w"); // open a new file
	if (fp_regout == NULL)             // handle error
	{
		printf("Error opening regout file");
		exit(1);
	}
	for (int i = 2; i <= 15; i++)      // print registers to file
	{
		fprintf(fp_regout, "%08X\n", regs[i]);
	}
	fprintf(fp_regout, "non condition writing onto regout\n");
	fclose(fp_regout);                 // close file
}

// this function creates memout file
void create_memout(unsigned int * mem, char file_name[]) {
	FILE* fp_memout;
	fp_memout = fopen(file_name, "w");     // open a new file
	if (fp_memout == NULL)                 // handle error
	{
		printf("Error opening memout file");
		exit(1);
	}
	for (int i = 0; i < MEMORY_SIZE; i++)  // print memory to file
	{
		fprintf(fp_memout, "%08X\n", *mem);
		mem++;
	}
	fprintf(fp_memout, "non condition writing onto memout\n");
	fclose(fp_memout);                     // close file
}

// this function creates diskout file
void export_diskout(unsigned int * disk, char file_name[]) {
	FILE* fp_diskout;
	fp_diskout = fopen(file_name, "w");         // open a new file
	if (fp_diskout == NULL)                     // handle error
	{
		printf("Error opening diskout file");
		exit(1);
	}
	for (int i = 0; i < STORAGE_SIZE; i++)      // print disk to file
	{
		fprintf(fp_diskout, "%08X\n", *disk);
		disk++;
	}
	fprintf(fp_diskout, "non condition writing onto diskout\n");
	fclose(fp_diskout);                         // close file
}

//create the cycles.txt file
void export_cycles(int counter, char file_name[]) {
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
	fprintf(fp_cycles, "non condition writing onto cycles\n");
	fclose(fp_cycles); // close file
}

// this function prepares a string to print to trace file
void create_line_for_trace(char Trace_Line[], int regs[], int pc, unsigned int inst, int imm)
{
	// sould be: PC INST R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 R15 \n
	int i;
	char inst_line[9];
	char pc_char[10] = { 0 };
	char temp_reg_char[9] = { 0 };
	sprintf(pc_char, "%08X", pc);
	sprintf(inst_line, "%08X", inst);
	sprintf(Trace_Line, pc_char);                                       //add pc to line
	sprintf(Trace_Line + strlen(Trace_Line), " ");                      //add space
	sprintf(Trace_Line + strlen(Trace_Line), inst_line);                //add opcode to line
	sprintf(Trace_Line + strlen(Trace_Line), " ");                      //add space

	for (i = 0; i < 16; i++)                                            //add registers to line
	{
		int temp_reg = 0;
		if (i == 1)                                                     // for imm
		{
			sprintf(temp_reg_char, "%08X", IntExtand(imm));             //change to hex
			sprintf(Trace_Line + strlen(Trace_Line), temp_reg_char);    //add to line
			sprintf(Trace_Line + strlen(Trace_Line), " ");
			continue;
		}
		if (regs[i] < 0)
			temp_reg = ABS(regs[i]);
		else
			temp_reg = regs[i];
		sprintf(temp_reg_char, "%08X", temp_reg);                       //change to hex
		sprintf(Trace_Line + strlen(Trace_Line), temp_reg_char);        //add to line
		if (i < 16)                                                       //no space for last register
			sprintf(Trace_Line + strlen(Trace_Line), " ");
	}
	/*	int temp_reg = 0;
		if (regs[i] < 0)
			temp_reg = ABS(regs[i]);
		else
			temp_reg = regs[i];
		sprintf(temp_reg_char, "%.8X", temp_reg);
		sprintf(line_for_trace + strlen(line_for_trace), temp_reg_char);*/
}

// create function that will colect data for hwregtrace
void create_line_for_hwregtrace(char hwregtrace_line[], int io_regs[], int regs[], int counter, Command cmd)
{
	char counter_char[10] = { 0 };
	char temp_reg_char[10] = { 0 };
	sprintf(counter_char, "%d", counter);
	sprintf(hwregtrace_line, counter_char);                                //add counter to line
	sprintf(hwregtrace_line + strlen(hwregtrace_line), " ");
	if (cmd.OpCode == 17)
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "READ ");       //add read to line
	else
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "WRITE ");      //add write to line
	switch (regs[cmd.rs] + regs[cmd.rt])
	{
	case 0:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irq0enable "); //add register name to line
		break;
	}
	case 1:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irq1enable "); //add register name to line
		break;
	}
	case 2:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irq2enable "); //add register name to line
		break;
	}
	case 3:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irq0status "); //add register name to line
		break;
	}
	case 4:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irq1status "); //add register name to line
		break;
	}
	case 5:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irq2status "); //add register name to line
		break;
	}
	case 6:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irqhandler "); //add register name to line
		break;
	}
	case 7:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "irqreturn "); //add register name to line
		break;
	}
	case 8:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "clks "); //add register name to line
		break;
	}
	case 9:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "leds "); //add register name to line
		break;
	}
	case 10:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "reserved "); //add register name to line
		break;
	}
	case 11:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "timerenable "); //add register name to line
		break;
	}
	case 12:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "timercurrent "); //add register name to line
		break;
	}
	case 13:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "timermax "); //add register name to line
		break;
	}
	case 14:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "diskcmd "); //add register name to line
		break;
	}
	case 15:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "disksector "); //add register name to line
		break;
	}
	case 16:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "diskbuffer "); //add register name to line
		break;
	}
	case 17:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "diskstatus "); //add register name to line
		break;
	}
	case 18:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "monitorcmd "); //add register name to line
		break;
	}
	case 19:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "monitorx "); //add register name to line
		break;
	}
	case 20:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "monitory "); //add register name to line
		break;
	}
	case 21:
	{
		sprintf(hwregtrace_line + strlen(hwregtrace_line), "monitordata "); //add register name to line
		break;
	}
	}

	if (cmd.OpCode == 17)       //if we read 
	{
		sprintf(temp_reg_char, "%08X", io_regs[regs[cmd.rs] + regs[cmd.rt]]);
		sprintf(hwregtrace_line + strlen(hwregtrace_line), temp_reg_char); //add data to line
	}
	else                        //if we write
	{
		sprintf(temp_reg_char, "%08X", regs[cmd.rd]);
		sprintf(hwregtrace_line + strlen(hwregtrace_line), temp_reg_char); //add data to line
	}
}

//create monitor.txt
void create_line_for_monitor(char line_for_monitor[], int regs[], int io_regs[], int cycles, Command cmd)
{
	char clk_cycles[10];
	char curr_display[10];
	sprintf(clk_cycles, "%d", cycles);
	sprintf(curr_display, "%08X", regs[cmd.rd]);
	sprintf(line_for_monitor, clk_cycles);                                         //add clk cycles to line
	sprintf(line_for_monitor + strlen(line_for_monitor), " ");                     //add space 
	sprintf(line_for_monitor + strlen(line_for_monitor), curr_display);            //add current display to line
}

//create monitor.txt
void create_line_for_monitor_yuv(char line_for_monitor_yuv[], int regs[], int io_regs[], int cycles, Command cmd)
{
	char clk_cycles[10];
	char curr_display[10];
	sprintf(clk_cycles, "%d", cycles);
	sprintf(curr_display, "%c", regs[cmd.rd]);
	sprintf(line_for_monitor_yuv, clk_cycles);                                     //add clk cycles to line
	//sprintf(line_for_monitor_yuv + strlen(line_for_monitor_yuv), " ");           //add space 
	sprintf(line_for_monitor_yuv + strlen(line_for_monitor_yuv), curr_display);    //add current display to line
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

Command Invalid_cmd(Command cmd)
{   // zeros all parameters of the command when the command is not valid               
	cmd.OpCode = 0;  
	cmd.rd = 0;
	cmd.rs = 0;
	cmd.rt = 0;
	cmd.imm = 1;
	return cmd;
}

void Error_Handler(int id)
{     // perhaps need to add a close(fp) for each file or the ones above it...
	switch (id)
	{
	case 1:
	{
		printf("Error while initializing simulation due to 'imemin' read issue");
		break;
	}
	case 2:
	{
		printf("Error while initializing loading simulation due to 'dmemin' read issue");
		break;
	}
	case 3:
	{
		printf("Error while initializing loading simulation due to 'diskin' read issue");
		break;
	}
	case 4:
	{
		printf("Error while initializing loading simulation due to 'irq2in' read issue");
		break;
	}
	case 5:
	{
		printf("Error while loading simulation due to a NULL fp_trace");
		break;
	}
	case 6:
	{
		printf("Error while loading simulation due to a NULL fp_hwregtrace");
		break;
	}
	case 7:
	{
		printf("Error while loading simulation due to a NULL fp_leds");
		break;
	}
	case 8:
	{
		printf("Error while loading simulation due to a NULL fp_monitor");
		break;
	}
	case 9:
	{
		printf("Error while loading simulation due to a NULL fp_monitor_yuv");
		break;
	}
	}
	exit(1);
}

int main(int argc, char* argv[])
{
	/*                           arguments for the simulation:
	   0-sim.exe        1-imemin.txt      2-dmemin.txt     3-diskin.txt        4-irq2in.txt
	   5-dmemout.txt    6-regout.txt      7-trace.txt      8-hwregtrace.txt    9-cycles.txt
	   10-leds.txt      11-monitor.txt    12-monitor.yuv   13-diskout.txt      */

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
	if (read_imemin(mem, argv[1]) == -1)
		Error_Handler(1);
	if (read_dmemin(disk, argv[2]) == -1)
		Error_Handler(2);
	if (read_diskin(irq2, argv[3]) == -1)
		Error_Handler(3);
	if (read_irq2in(irq2, argv[4]) == -1)
		Error_Handler(4);

	FILE* fp_trace;                                                                          //define pointer for writing trace file
	FILE* fp_hwregtrace;                                                                     //define pointer for writing hwregtrace file
	FILE* fp_leds;                                                                           //define pointer for writing leds file
	FILE* fp_monitor;                                                                        //define pointer for writing monitor file
	FILE* fp_monitor_yuv;                                                                    //define pointer for writing monitor.yuv file

	fp_trace = fopen(argv[7], "w");
	if (fp_trace == NULL)
		Error_Handler(5);
	fp_hwregtrace = fopen(argv[8], "w");
	if (fp_hwregtrace == NULL)
		Error_Handler(6);
	fp_leds = fopen(argv[10], "w");
	if (fp_leds == NULL)
		Error_Handler(7);
	fp_monitor = fopen(argv[11], "w");
	if (fp_monitor == NULL)
		Error_Handler(8);
	fp_monitor_yuv = fopen(argv[12], "w");
	if (fp_monitor_yuv == NULL)
		Error_Handler(9);
	
	// Execution
	unsigned int inst = 0;                                                                   //define instruction number
	while (pc > 0)                                                                           //use the pc as flag for halt function
	{
		if (pc <= MEMORY_SIZE - 1)
			inst = mem[pc];                                                                  //setting inst to be the current memory instruction
		Command cmd = line_to_command(inst);                                                 //create Command struct
		if ((io_regs[0] && io_regs[3]) || (io_regs[1] && io_regs[4]) || (io_regs[2] && io_regs[5]))
		{
			cmd = interrupt_handler(io_regs, regs, cmd, mem, pc_ptr, reti_flag);             //we have irq==1 and need to handle it	
			if (pc <= MEMORY_SIZE - 1)
				inst = mem[pc];
		}
		char line_for_trace[200] = { 0 };                                                    //create line for trace file
		char line_for_leds[20] = { 0 };                                                      //create line for leds file
		char line_for_monitor[20] = { 0 };                                                   //create line for display file
		char line_for_monitor_yuv[20] = { 0 };                                               //create line for display file
		char line_for_hwregtrace[100] = { 0 };                                               //create line for hwregtrace file
		regs[1] = IntExtand(cmd.imm);                                                        //first we do sign extend to immiediate
		update_irq2(io_regs, irq2, counter);                                                 //update irq2status register
		timer(io_regs);                                                                      //check if timer is enable
		if (io_regs[17] == 1)                                                                //check if disk is still busy
			disk_handler(disk, io_regs, mem);
		if ((cmd.OpCode == 17) || (cmd.OpCode == 18))                                        //if we read or write
		{
			create_line_for_hwregtrace(line_for_hwregtrace, io_regs, regs, counter, cmd);    //append to trace file
			fprintf(fp_hwregtrace, "%s\n", line_for_hwregtrace);                             //fprintf the data at the end of hwregtrace
			fprintf(fp_hwregtrace, "condition writing onto hwregtrace\n");
		}
		create_line_for_trace(line_for_trace, regs, pc, inst, cmd.imm);                      //append to trace file
		fprintf(fp_trace, "%s\n", line_for_trace);                                           //fprintf the data at the end of trace
		fprintf(fp_trace, "condition writing onto trace\n");
		if (((regs[cmd.rs] + regs[cmd.rt]) == 9) && (cmd.OpCode == 18))
		{
			if (regs[cmd.rd] != io_regs[regs[cmd.rs] + regs[cmd.rt]])
			{
				create_line_for_leds(line_for_leds, regs, io_regs, counter, cmd);            //append to leds file
				fprintf(fp_leds, "%s\n", line_for_leds);
				fprintf(fp_leds, "condition writing onto leds\n");
			}
		}
		if ((regs[cmd.rs] + regs[cmd.rt]) == 10 && cmd.OpCode == 18)
		{
			if (regs[cmd.rd] != io_regs[regs[cmd.rs] + regs[cmd.rt]])
			{
				create_line_for_monitor(line_for_monitor, regs, io_regs, counter, cmd);      //append to display file
				fprintf(fp_monitor, "%s\n", line_for_monitor);
				fprintf(fp_monitor, "condition writing onto monitor\n");
				create_line_for_monitor_yuv(line_for_monitor_yuv, regs, io_regs, counter, cmd);
				fprintf(fp_monitor_yuv, "%s\n", line_for_monitor_yuv);
				fprintf(fp_monitor_yuv, "condition writing onto monitor.yuv\n");
			}
		}
		pc = OPcode_To_execute(regs, io_regs, pc, cmd, mem, disk, reti_flag);                //execute instruction
		io_regs[8] = counter++;                                                              //clk cycle counter
	}
	fprintf(fp_monitor_yuv, "non condition writing onto monitor.yuv\n");
	fprintf(fp_leds, "non condition writing onto leds\n");
	fprintf(fp_trace, "non condition writing onto trace\n");
	fprintf(fp_hwregtrace, "non condition writing onto hwregtrace\n");
	fprintf(fp_monitor, "non condition writing onto monitor\n");

	create_memout(mem, argv[5]);                                                             //create memout file
	export_regout(regs, argv[6]);                                                            //create regout file
	export_cycles(counter, argv[9]);                                                         //create cycles file
	export_diskout(disk, argv[13]);                                                          //create diskout file
	fclose(fp_trace);                                                                        //close trace file
	fclose(fp_hwregtrace);                                                                   //close hwregtrace file
	fclose(fp_leds);                                                                         //close leds file
	fclose(fp_monitor);                                                                      //close monitor file
	fclose(fp_monitor_yuv);                                                                  //close monitor.yuv file

	return 0;                                                                                //finish simulator
}
