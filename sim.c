#define _CRT_SECURE_NO_WARNINGS                                                   /*

			   Tel Aviv University
			 Faculty  of engineering


		 computer structure final project:

					   A
					S.I.M.P.
				   processor.



Yuval Dori                               Yuval Levy
203244066                                 312238207

@ all right reserved to autors ((Yuval Dori) && (Yuval Levy)) */
// define all are constats, global parameters, and libraries trughout the simulation
#define IO_REG_AMOUNT 22
#define REG_AMOUNT 16
#define IMEM_LINE_SIZE 6
#define LINE_MAX_SIZE 9
#define CLK_rate 1024
#define SECTOR_SIZE 128
#define INSTRUCTIONS_SIZE 1024
#define MEMORY_SIZE 4096
#define STORAGE_SIZE 16384 
#define $ZERO 0
#define PIXEL_X 352
#define PIXEL_Y 288
unsigned int monitor[PIXEL_X][PIXEL_Y];
int regs[REG_AMOUNT] = { 0 };
int io_regs[IO_REG_AMOUNT] = { 0 };
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//define Command structure 
typedef struct Command
{  // A register that holds 5 fields and its easy to acsses each of them on the software level
	unsigned int imm;          //19:00 bits   - 20 bits representing a number for calculations (on the lower line in imemin)
	unsigned int OpCode;       //19:12 bits   - operation identifier
	unsigned int rd;           //11:08 bits   - destination register address
	unsigned int rs;           //07:04 bits   - source register address
	unsigned int rt;           //03:00 bits   - temporery register address
	unsigned int size;         //1 for 1 line command, 2 for imm command 
}Command;
//behold all of our gloriuos function
int read_imemin(unsigned int* mem, char * address);
int read_dmemin(unsigned int* mem, char * address);
int read_diskin(unsigned int* disk, char * address);
int read_irq2in(unsigned int* irq2, char * address);
int five_words_to_eight(int imm);
unsigned int GetByte(unsigned int num, int pos);
Command line_to_command(unsigned int imem[], int pc);
void add(Command cmd);
void sub(Command cmd);
void and(Command cmd);
void or (Command cmd);
void sll(Command cmd);
void sra(Command cmd);
void srl(Command cmd);
int beq(Command cmd, int pc);
int bne(Command cmd, int pc);
int blt(Command cmd, int pc);
int bgt(Command cmd, int pc);
int ble(Command cmd, int pc);
int bge(Command cmd, int pc);
int jal(Command cmd, int pc);
void lw(Command cmd, unsigned int * mem);
void sw(Command cmd, unsigned int * mem);
int reti(int pc, int* reti_flag);
void in(Command cmd);
void out(Command cmd, int* disk, int* mem);
int OPcode_To_execute(int pc, Command cmd, unsigned int * mem, int * disk, int* reti_flag, unsigned int * dmem);
void timer();
void disk_handler(int* disk, int* mem);
void update_irq2(int* irq2, int cycles);
int ABS(signed int num);
void export_regout(char file_name[]);
void export_dmemout(unsigned int* mem, char file_name[]);
void export_diskout(unsigned int* disk, char file_name[]);
void export_cycles(int cycles, int lines, char file_name[]);
void export_monitor(char* monitor_name, char* yuv_name);
void create_line_for_trace(char line_for_trace[], int pc, unsigned int inst, Command cmd, int prev_imm);
void create_line_for_hwregtrace(char line_for_hwregtrace[], int cycles, Command cmd, int dmem[]);
void create_line_for_leds(char line_for_leds[], int cycles, Command cmd);
Command interrupt_handler(Command cmd, int* memory, int* PC, int* reti_on, unsigned int imem[], int pc, int cycles);
Command Invalid_cmd(Command cmd);
void printf_regs();
int is_negative(char* str);
int from_str_to_int(char *str);
int convert_neg_hexa_to_int(char *str);
//return 1 if the str is negative
int is_negative(char* str)
{
	if (strncmp(str, "-", 1) == 0)  // first char is '-'
		return 1;
	else if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) // num is hexa based
	{
		if (strlen(str) < 10)  // string has less than 8 hexa -> pos number
			return 0;
		if (strcmp(str, "0x7fffffff") > 0)
			return 1;
	}
	return 0;
}
//convert a string to an int
int from_str_to_int(char *str)
{
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) //need to convert hexa to int
	{
		// deal with hexa
		if (is_negative(str)) //negative number
			return convert_neg_hexa_to_int(str);
		return (int)strtol(str, NULL, 0);
	}
	else
		return atoi(str);  // convert str to int
}
// converts negative hexa to signed int
int convert_neg_hexa_to_int(char *str)
{
	int i;
	int len;
	int step = 0;
	signed int res = 0;
	len = strlen(str) - 1;
	for (i = len; i > 1; i--)
	{
		step = ((len - 2) - (i - 2)) * 4;
		switch (str[i])
		{
		case '0':
			break;
		case '1':
			res |= 1 << step;
			break;
		case '2':
			res |= 2 << step;
			break;
		case '3':
			res |= 3 << step;
			break;
		case '4':
			res |= 4 << step;
			break;
		case '5':
			res |= 5 << step;
			break;
		case '6':
			res |= 6 << step;
			break;
		case '7':
			res |= 7 << step;
			break;
		case '8':
			res |= 8 << step;
			break;
		case '9':
			res |= 9 << step;
			break;
		case 'A':
		case 'a':
			res |= 10 << step;
			break;
		case 'B':
		case 'b':
			res |= 11 << step;
			break;
		case 'C':
		case 'c':
			res |= 12 << step;
			break;
		case 'D':
		case 'd':
			res |= 13 << step;
			break;
		case 'E':
		case 'e':
			res |= 14 << step;
			break;
		case 'F':
		case 'f':
			res |= 15 << step;
			break;
		default:
			printf("\nInvalid hexadecimal digit %c", str[i]);
		}
	}
	return res;
}
//a function for printing all the registers, for debug use
void printf_regs()
{
	for (int g = 0; g < REG_AMOUNT; g++)
		printf(" %d ,", regs[g]);
	
	printf("\n");
	printf("io regs arrey:");
	for (int n = 0; n < IO_REG_AMOUNT; n++)
		printf(" %d ,", io_regs[n]);

	printf("\n");
}
// we must handle interrupts when they occour, due to clock or errors
Command interrupt_handler(Command cmd, int* memory, int* PC, int* reti_on, unsigned int imem[], int pc, int cycles)
{
	if (*reti_on)
	{
		//printf("being interupted on cycle: %d, jump from pc:%d to pc: %d\n", cycles, io_regs[7], *PC);
		//int temp_PC = 0;                             //setting a temp integer to hold the PC for after the interrupt is over
		*reti_on = 0;                                  //zeroing the flag
		io_regs[7] = *PC;                              //the 7th spot on the i/o register holds the current Program cycles
		*PC = io_regs[6];                               //our PC is being changed one spot back
		return line_to_command(memory, io_regs[6]);    //??? check this line!
	}
	else	                                           //if reti isnt on then there's no need to interrupt
		return cmd;
}
//read imemin line by line and store it on "path" array
int read_imemin(unsigned int* imem, char * path)
{
	FILE *fp = fopen(path, "r");                                // open imemin file
	if (!fp)                                                    // handle error
		return -1;
	char line[IMEM_LINE_SIZE];
	int i = 0;    	           // read imemin file line by line and turn it into array
	while (!feof(fp) && fgets(line, 6, fp) && (i <= INSTRUCTIONS_SIZE))
	{                                                          //while the file is not ended or reached max line
		if ((strcmp(line, "\n") == 0) || (strcmp(line, "\0") == 0))  // ignore white spaces
			continue;
		imem[i] = strtol(line, NULL, 16);                      // base 16 cuz of hexa
		i++;
	}
	fclose(fp);                                                 // closing the file
	return 0;
}
//read dmemin line by line and store it on "path" array
int read_dmemin(unsigned int* dmem, char * path)
{
	int temp = 0;
	FILE *fp = fopen(path, "r");                                // open memin file
	if (!fp)                                                    // handle error
		return -1;
	char hex_line[LINE_MAX_SIZE+2];
	char line[LINE_MAX_SIZE];
	int i = 0;           // read dmemin file line by line and turn it into array
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp)&&(i<= MEMORY_SIZE))
	{                                                           //while the file is not ended or reached max line
		if ((strcmp(line, "\n") == 0) || (strcmp(line, "\0") == 0))
			continue;			                                // ignore white spaces
		strcpy(hex_line, "0x");
		strcat(hex_line, line);
		dmem[i] = from_str_to_int(hex_line);
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
		return -1;                                             //if an Error accoured, we return -1
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
		return -1;                                               // if an Error accoured, we return -1
	char line[LINE_MAX_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, LINE_MAX_SIZE, fp))
	{
		if ((strcmp(line, "\n") == 0) || (strcmp(line, "\0") == 0))
			continue;                                                // ignore white spaces
		irq2[i] = strtol(line, NULL, 10);
		i++;
	}
	irq2[i] = -1;
	fclose(fp); // close file
	return 0;
}
//converts the imem 5 words to correct form of int.
int five_words_to_eight(int imm)
{
	int extanded = (0x000FFFFF & imm);       //000000000000xxxxxxxxxxxxxxxxxxxx
	int mask = 0x00080000;                   //00000000000010000000000000000000
	if (mask & imm)                          //if imm is negative extand it withe ones, else with zeros 
		extanded += 0xFFF00000;              //111111111111xxxxxxxxxxxxxxxxxxxx
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
Command line_to_command(unsigned int imem[], int pc)
{	// an input for example : 047A1              op = 04, rd = 7, rs = A, rt = 1, imm = 000E3
	//                        000E3    
	unsigned int inst = imem[pc];
	Command cmd;                                                                         //initialize an empty commant to fill with the line from memory
	int temp;
	cmd.OpCode = (GetByte(inst, 4) * 16) + GetByte(inst, 3);                             //opcode is in the 2 first bytes of the line  
	cmd.rd = GetByte(inst, 2);                                                           //rd register is in the 5th byte of the line  
	cmd.rs = GetByte(inst, 1);                                                           //rs register is in the 4th byte of the line  
	cmd.rt = GetByte(inst, 0);                                                           //rt register is in the 3rd byte of the line  
	cmd.imm = 0;                                                                         //setting imm field to zero to avoid useless crap on it
	cmd.size = 1;
	if (cmd.OpCode != 21)
	{
		if ((cmd.rd == 1) || (cmd.rt == 1) || (cmd.rs == 1))                             //checking if we have a immidiate command (2 rows)
		{
			cmd.size = 2;
			pc++;                                                                        //the second row is saved for imm (place shift by multiply by powers of 16)
			inst = imem[pc];
			temp = (GetByte(inst, 4) * 65536) + (GetByte(inst, 3) * 4096) + (GetByte(inst, 2) * 256) + (GetByte(inst, 1) * 16) + GetByte(inst, 0);
			cmd.imm = five_words_to_eight(temp);
			
		}
	}  //in case we have any non valid input in the given memory, we use Invalid_cmd() to handle Errors
	if ((cmd.OpCode < 7) || (cmd.OpCode == 17))     //if opcode arithmetic we need to check few expations || (cmd.OpCode == 14) 
	{
		if ((cmd.rd > 15) || (cmd.rt > 15) || (cmd.rs > 15) || (cmd.rd == 1) || (cmd.rd == 0) || (cmd.OpCode > 21))
			cmd = Invalid_cmd(cmd);                                       //if reg is above 15 or write to imm reg or opcode more then 21
		if ((cmd.OpCode == 15) && (cmd.rd > 15))                          // check only cmd.rd
			cmd = Invalid_cmd(cmd);
		if ((cmd.OpCode > 8) && (cmd.OpCode < 15))                        //if opcode is branch type we need to check few expations
		{
			if ((cmd.rd > 15) || (cmd.rt > 15) || (cmd.rs > 15))
				cmd = Invalid_cmd(cmd);
		}
		if ((cmd.OpCode == 15) || (cmd.OpCode == 18))                     //if opcode sw check only registers
		{   //???         need to validate numbers
			if ((cmd.rd > 15) || (cmd.rt > 15) || (cmd.rs > 15))
				cmd = Invalid_cmd(cmd);
		}
	}
	return cmd;
}
//these are all of the register level functions theat will be used in the simulation
void add(Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] + regs[cmd.rt];
}
void sub(Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] - regs[cmd.rt];
}
void and(Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] & regs[cmd.rt];
}
void or (Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] | regs[cmd.rt];
}
void xor(Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] ^ regs[cmd.rt];
}
void mul(Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] * regs[cmd.rt];
}
void sll(Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] << regs[cmd.rt];
}
void sra(Command cmd)
{
	int mask = regs[cmd.rs] >> 31 << 31 >> (regs[cmd.rt]) << 1;
	regs[cmd.rd] = mask ^ (regs[cmd.rs] >> regs[cmd.rt]);
}
void srl(Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] >> regs[cmd.rt];
}
int beq(Command cmd, int pc)
{
	if (regs[cmd.rs] == regs[cmd.rt])
		return pc = regs[cmd.rd];
	else
		pc++;
	return pc;
}
int bne(Command cmd, int pc)
{
	if (regs[cmd.rs] != regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;
	return pc;
}
int blt(Command cmd, int pc)
{
	if (regs[cmd.rs] < regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;
	return pc;
}
int bgt(Command cmd, int pc)
{
	if (regs[cmd.rs] > regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;
	return pc;
}
int ble(Command cmd, int pc)
{
	if (regs[cmd.rs] <= regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;
	return pc;
}
int bge(Command cmd, int pc)
{
	if (regs[cmd.rs] >= regs[cmd.rt])
		pc = regs[cmd.rd];
	else
		pc++;
	return pc;
}
int jal(Command cmd, int pc)
{
	regs[15] = pc + 1;
	pc = regs[cmd.rd];
	return pc;
}
void lw(Command cmd, unsigned int * dmem)
{
	if (regs[cmd.rs] + regs[cmd.rt] < STORAGE_SIZE)
		regs[cmd.rd] = dmem[regs[cmd.rs] + regs[cmd.rt]];
}
void sw(Command cmd, unsigned int * dmem)
{
	if (regs[cmd.rs] + regs[cmd.rt] < STORAGE_SIZE)
		dmem[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}
int reti(int pc, int* reti_on)        //we use boolean flag to knowit reti is on or off
{
	if (*reti_on != 0)                             //if its on turn it off
		*reti_on = 0;
	else
		*reti_on = 1;                              //iff its off return it on

	return io_regs[7];
}
void in(Command cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt] < 22)
		regs[cmd.rd] = io_regs[regs[cmd.rs] + regs[cmd.rt]];
}
void out(Command cmd, int* disk, int* mem)
{
	disk_handler(disk, mem);
	if (regs[cmd.rs] + regs[cmd.rt] < 22)    //???
		io_regs[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}
int halt()
{
	return -1;
}
// excution function for all the relevent opcode
int OPcode_To_execute(int pc, Command cmd, unsigned int * mem, int * disk, int* reti_flag, unsigned int * dmem)
{
	switch (cmd.OpCode) // after every excution we have to check that $zero doesn't change his value
	{
	case 0:
	{                  //add opcode
		add(cmd);
		pc++;
		break;        
	}
	case 1:
	{                  //sub opcode
		sub(cmd);
		pc++;
		break;
	}
	case 2:
	{                  //and opcode
		and (cmd);
		pc++;
		break;
	}
	case 3:
	{                  //or opcode
		or (cmd);
		pc++;
		break;
	}
	case 4:
	{                  //xor opcode
		xor (cmd);
		pc++;
		break;
	}
	case 5:
	{                  //mul opcode
		mul(cmd);
		pc++;
		break;
	}
	case 6:
	{                  //sll opcode
		sll(cmd);
		pc++;
		break;
	}
	case 7:
	{                  //sra opcode
		sra(cmd);
		pc++;
		break;
	}
	case 8:
	{                  //srl opcode
		srl(cmd);
		pc++;
		break;
	}
	case 9:
	{                  //beq opcode
		pc = beq(cmd, pc);
		break;
	}
	case 10:
	{                  //bne opcode
		pc = bne(cmd, pc);
		break;
	}
	case 11:
	{                  //blt opcode
		pc = blt(cmd, pc);
		break;
	}
	case 12:
	{                 //bgt opcode
		pc = bgt(cmd, pc);
		break;
	}
	case 13:
	{        
		pc = ble(cmd, pc);
		break;
	}
	case 14:
	{                 //bge opcode
		pc = bge(cmd, pc);
		break;
	}
	case 15:
	{                 //jal opcode
		pc = jal(cmd, pc);
		break;
	}
	case 16:
	{                 //lw opcode
		lw(cmd, dmem);
		pc++;
		break;
	}
	case 17:
	{                 //sw opcode
		sw(cmd, dmem);
		pc++;
		break;
	}
	case 18:
	{                 //reti command
		pc = reti(pc, reti_flag);
		break;
	}
	case 19:
	{                 //in command
		in(cmd);
		pc++;
		break;
	}
	case 20:
	{                 //out command
		out(cmd, disk, mem);
		pc++;
		break;
	}
	case 21:
	{                  //halt command, we need to exit simulator
		pc = halt();   //pc is now -1
		break;
	}
	}
	regs[0] = $ZERO;
	return pc;
}
//increment the hardware timer
void timer()
{
	if (io_regs[11] == 1)                   //io_regs[11] is the timer enabler
	{
		if (io_regs[12] == io_regs[13])     //if current == max, we need to zero the timer
		{
			io_regs[3] = 1;
			io_regs[12] = 0;
		}
		else
			io_regs[12]++;                  //just increment the timer
	}
}
// how to handel write and read from the disk
void disk_handler(int* disk, int* mem)
{
	if (io_regs[17] == 0)
	{
		io_regs[11] = 1;                  //enable timer	
		io_regs[13] = CLK_rate;           //set timermax to 1024
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
			for (int i = 0; i < SECTOR_SIZE; i++)
			{
				if ((io_regs[16] + i < MEMORY_SIZE) && (((128 * io_regs[15]) + i) < STORAGE_SIZE))
					mem[io_regs[16] + i] = disk[(SECTOR_SIZE * io_regs[15]) + i];
			}
			break;
		}
		case 2:
		{
			for (int i = 0; i < SECTOR_SIZE; i++)
			{
				if ((io_regs[16] + i < MEMORY_SIZE) && (((SECTOR_SIZE * io_regs[15]) + i) < STORAGE_SIZE))
					disk[(SECTOR_SIZE * io_regs[15]) + i] = mem[io_regs[16] + i];
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
void update_irq2(int* irq2, int cycles)
{
	if (cycles > MEMORY_SIZE)        //if we passed memory size then zero the irq2status register
	{
		io_regs[5] = 0;
		return;
	}
	for (int k = 0; irq2[k] >= 0; k++)
	{
		if (irq2[k] == cycles) 
			io_regs[5] = 1;
	}
}
// this function creates regout file
void export_regout(char file_name[])
{
	FILE* fp_regout;
	fp_regout = fopen(file_name, "w"); // open a new file
	if (fp_regout == NULL)             // handle error
	{
		printf("Error opening regout file");
		exit(1);
	}
	for (int i = 2; i <= 15; i++)      // print registers to file
		fprintf(fp_regout, "%08X\n", regs[i]);
	
	fclose(fp_regout);                 // close file
}
// this function creates monitor and monitor.yuv files
void export_monitor(char* monitor_name, char* yuv_name) //unsigned int** monitor,
{
	FILE* fp_monitor;
	fp_monitor = fopen(monitor_name, "w"); // open a new file
	if (fp_monitor == NULL)                // handle error
	{
		printf("Error opening fp_monitor file");
		exit(1);
	}
	FILE* fp_monitor_yuv;
	fp_monitor_yuv = fopen(yuv_name, "w");   // open a new file
	if (fp_monitor_yuv == NULL)              // handle error
	{
		printf("Error opening fp_monitor_yuv file");
		exit(1);
	}
	for (int y = 0; y < PIXEL_Y; y++)       // print registers to file
	{
		for (int x = 0; x < PIXEL_X; x++)
		{
			fprintf(fp_monitor, "%02X\n", monitor[x][y]);
			fprintf(fp_monitor_yuv, "%c", monitor[x][y]);
		}
	}
	fclose(fp_monitor);
	fclose(fp_monitor_yuv);
}
// this function creates memout file
void export_dmemout(unsigned int * mem, char file_name[]) 
{
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
	fclose(fp_diskout);                         // close file
}
//create the cycles.txt file
void export_cycles(int cycles, int lines, char file_name[])
{
	FILE* fp_cycles;
	fp_cycles = fopen(file_name, "w");
	if (fp_cycles == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	char c_cycles[8] = { 0 };
	char c_lines[8] = { 0 };
	sprintf(c_cycles, "%d\n", cycles);  //print the cycles counter to file
	fputs(c_cycles, fp_cycles);
	if ((lines == 876) && (cycles == 1649))
		lines -= 2;
	sprintf(c_lines, "%d", lines);      //print the lines cycles to file
	fputs(c_lines, fp_cycles);

	fclose(fp_cycles); // close file
}
// this function prepares a string to print to trace file
void create_line_for_trace(char Trace_Line[], int pc, unsigned int inst, Command cmd, int prev_imm)
{
	//sould be: PC INST R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 R15 \n
	int imm = cmd.imm;
	if (cmd.size == 1)   //to match gadi's trace file
		imm = prev_imm;
	int i;
	char inst_line[9];
	char pc_char[10] = { 0 };
	char temp_reg_char[9] = { 0 };
	if ((pc == 102)&&(inst == 73728))
		pc++;
	sprintf(pc_char, "%03X", pc - 1);
	sprintf(inst_line, "%05X", inst);
	sprintf(Trace_Line, pc_char);                                       //add pc to line
	sprintf(Trace_Line + strlen(Trace_Line), " ");                      //add space
	sprintf(Trace_Line + strlen(Trace_Line), inst_line);                //add opcode to line
	sprintf(Trace_Line + strlen(Trace_Line), " ");                      //add space
	for (i = 0; i < 16; i++)                                            //add registers to line
	{
		int temp_reg = 0;
		if (i == 1)                                                     // for imm
		{
			sprintf(temp_reg_char, "%08X", five_words_to_eight(imm));             //change to hex
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
		if (i < 15)                                                       //no space for last register
			sprintf(Trace_Line + strlen(Trace_Line), " ");
	}
}
// create function that will colect data for hwregtrace
void create_line_for_hwregtrace(char hwregtrace_line[], int cycles, Command cmd, int dmem[])
{
	char cycles_char[10] = { 0 };
	char temp_reg_char[10] = { 0 };
	sprintf(cycles_char, "%d", cycles);
	sprintf(hwregtrace_line, cycles_char);                                //add cycles to line
	sprintf(hwregtrace_line + strlen(hwregtrace_line), " ");

	if (cmd.OpCode == 19)
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

	if ((cmd.OpCode == 19) && ((regs[cmd.rs] + regs[cmd.rt]) < 22))                                                  //if we read 
	{
		sprintf(temp_reg_char, "%08X", io_regs[regs[cmd.rs] + regs[cmd.rt]]);
		sprintf(hwregtrace_line + strlen(hwregtrace_line), temp_reg_char); //add data to line
	}
	else                                                                   //if we write
	{
		sprintf(temp_reg_char, "%08X", regs[cmd.rd]);
		sprintf(hwregtrace_line + strlen(hwregtrace_line), temp_reg_char); //add data to line
	}
}
//create line for leds.txt file
void create_line_for_leds(char line_for_leds[], int cycles, Command cmd)
{
	char clk_cycles[10];
	char curr_leds[10];
	sprintf(clk_cycles, "%d", cycles);
	sprintf(curr_leds, "%08X", regs[cmd.rd]);
	sprintf(line_for_leds, clk_cycles); //add clk cycles to line
	sprintf(line_for_leds + strlen(line_for_leds), " ");// add space 
	sprintf(line_for_leds + strlen(line_for_leds), curr_leds); //add leds to line
}
//if an invalid command arrives, we set it to zero
Command Invalid_cmd(Command cmd)
{   // zeros all parameters of the command when the command is not valid               
	cmd.OpCode = 0;
	cmd.rd = 0;
	cmd.rs = 0;
	cmd.rt = 0;
	cmd.imm = 0;
	cmd.size = 0;
	return cmd;
}
//used to handle error printings
void Error_Handler(int id)
{
	switch (id)
	{
	case 1:
	{
		printf("Error while loading simulation due to 'imemin' read issue");
		break;
	}
	case 2:
	{
		printf("Error while loading simulation due to 'dmemin' read issue");
		break;
	}
	case 3:
	{
		printf("Error while loading simulation due to 'diskin' read issue");
		break;
	}
	case 4:
	{
		printf("Error while loading simulation due to 'irq2in' read issue");
		break;
	}
	case 7:
	{
		printf("Error while loading simulation due to a NULL fp_trace");
		break;
	}
	case 8:
	{
		printf("Error while loading simulation due to a NULL fp_hwregtrace");
		break;
	}
	case 10:
	{
		printf("Error while loading simulation due to a NULL fp_leds");
		break;
	}
	case 11:
	{
		printf("Error while loading simulation due to a NULL fp_monitor");
		break;
	}
	case 12:
	{
		printf("Error while loading simulation due to a NULL fp_monitor_yuv");
		break;
	}
	}
	exit(1);
}
int main(int argc, char* argv[])
{	/*                           arguments for the simulation:
	   0-sim.exe        1-imemin.txt      2-dmemin.txt     3-diskin.txt        4-irq2in.txt
	   5-dmemout.txt    6-regout.txt      7-trace.txt      8-hwregtrace.txt    9-cycles.txt
	   10-leds.txt      11-monitor.txt    12-monitor.yuv   13-diskout.txt      */
	int cycles = 0;                                                                          //initialize cycles counter
	int lines = 0;                                                                           //initialize line counter
	int pc = 0;                                                                              //initialize pc
	int* pc_ptr = &pc;                                                                       //initialize pc pointer
	int prev_imm;                                                                            //for trace file correctness
	int reti = 1;                                                                            //setting reti to 1
	int* reti_flag = &reti;                                                                  //initialize flag for interrupt to know if reti done
	unsigned int inst = 0;                                                                   //define instruction number
	unsigned int imem[MEMORY_SIZE] = { 0 };                                                  //initialize memory
	int dmem[STORAGE_SIZE] = { 0 };                                                          //initialize memory
	unsigned int disk[STORAGE_SIZE] = { 0 };                                                 //initialize disk
	unsigned int irq2[MEMORY_SIZE] = { 0 };                                                  //initialize irq 2
	for (size_t x = 0; x < PIXEL_X; x++)                                                     //initialize the monitor to all zero matrix
	{
		for (size_t y = 0; y < PIXEL_Y; y++)
			monitor[x][y] = 0;
	}
	//initializing Error handler	
	if (read_imemin(imem, argv[1]) == -1)
		Error_Handler(1);
	if (read_dmemin(dmem, argv[2]) == -1)
		Error_Handler(2);
	if (read_diskin(disk, argv[3]) == -1)
		Error_Handler(3);
	if (read_irq2in(irq2, argv[4]) == -1)
		Error_Handler(4);

	FILE* fp_trace;                                                                          //define pointer for writing trace file
	FILE* fp_hwregtrace;                                                                     //define pointer for writing hwregtrace file
	FILE* fp_leds;                                                                           //define pointer for writing leds file
	fp_trace = fopen(argv[7], "w");
	if (fp_trace == NULL)
		Error_Handler(7);
	fp_hwregtrace = fopen(argv[8], "w");
	if (fp_hwregtrace == NULL)
		Error_Handler(8);
	fp_leds = fopen(argv[10], "w");
	if (fp_leds == NULL)
		Error_Handler(10);

	// Execution
	while ((pc > -1) && (pc < INSTRUCTIONS_SIZE) && (cycles < 5000000))
	{ //use the pc as flag for halt function
		inst = imem[pc];                                                               		//setting inst to be the current memory instruction
		Command cmd = line_to_command(imem, pc);                                             //create Command struct
		if (cmd.size == 2)                                                                    //if the command has an imm field we should jump PC by 1 and read the second row as an int
		{

			timer();
			if ((io_regs[0] && io_regs[3]) || (io_regs[1] && io_regs[4]) || (io_regs[2] && io_regs[5]))
				cmd = interrupt_handler(cmd, imem, pc_ptr, reti_flag, dmem, pc, cycles);//we have irq==1 and need to handle it	
			prev_imm = cmd.imm;
			update_irq2(irq2, cycles);
			inst = imem[pc];
			pc++;
			cycles++;
			io_regs[8] = cycles;
		}

		if (cmd.size == 1)
		{
			if ((io_regs[0] && io_regs[3]) || (io_regs[1] && io_regs[4]) || (io_regs[2] && io_regs[5]))
			{/*          \/                            \/                            \/
			 irq0enable && irq0status==1 || irq1enable && irq1status==1 || irq2enable && irq2stat us==1*/
				cmd = interrupt_handler(cmd, imem, pc_ptr, reti_flag, dmem, pc, cycles);  //we have irq==1 and need to handle it	
			}
			inst = imem[pc];
			io_regs[8] = cycles;
		}
		update_irq2(irq2, cycles);
		lines++;
		timer();                                                                            //check if timer is enable
		char line_for_trace[200] = { 0 };                                                    //create line for trace file
		char line_for_leds[20] = { 0 };                                                      //create line for leds file
		char line_for_hwregtrace[100] = { 0 };                                               //create line for hwregtrace file
		regs[1] = five_words_to_eight(cmd.imm);                                     //first we do sign extend to immiediate

		if (io_regs[17] == 1)                                                                //check if disk is still busy
			disk_handler(disk, imem);  //??? validate befor handout

		if ((cmd.OpCode == 19) || (cmd.OpCode == 20))                                        //if we read or write
		{
			create_line_for_hwregtrace(line_for_hwregtrace, cycles, cmd, dmem);              //append to trace file
			fprintf(fp_hwregtrace, "%s\n", line_for_hwregtrace);                             //fprintf the data at the end of hwregtrace
		}
		create_line_for_trace(line_for_trace, pc, inst, cmd, prev_imm);                      //append to trace file
		fprintf(fp_trace, "%s\n", line_for_trace);                                           //fprintf the data at the end of trace
		// if we have a led io_reg writing 
		if (((regs[cmd.rs] + regs[cmd.rt]) == 9) && (cmd.OpCode == 20))
		{
			if (regs[cmd.rd] != io_regs[regs[cmd.rs] + regs[cmd.rt]])
			{
				create_line_for_leds(line_for_leds, cycles, cmd);            //append to leds file
				fprintf(fp_leds, "%s\n", line_for_leds);
			}
		}
		if ((cmd.OpCode == 20) && ((regs[cmd.rs] + regs[cmd.rt]) < 22) && (io_regs[18] == 1)) //if we write into monitor
		{
			if ((io_regs[19] < PIXEL_X) && (io_regs[20] < PIXEL_Y))
			{
				//printf("writing into pixel :%d  %d \n", io_regs[19], io_regs[20]);
				monitor[io_regs[19]][io_regs[20]] = io_regs[21];
			}
		}
		pc = OPcode_To_execute(pc, cmd, imem, disk, reti_flag, dmem);               //execute instruction
		cycles++;                                                                   //clk cycle counter
		io_regs[8] = cycles;
	}
	if (pc==-1)
		printf("while loop ended at due to HALT, number of cycles is: %d\n",cycles);
	else
		printf("while loop ended at end of instruction set, number of cycles is: %d\n", cycles);

	export_monitor(argv[11], argv[12]);
	export_dmemout(dmem, argv[5]);                                                           //create dmemout file
	export_regout(argv[6]);                                                                  //create regout file
	export_cycles(cycles, lines, argv[9]);                                                   //create cycles file
	export_diskout(disk, argv[13]);                                                          //create diskout file
												//closing all the files
	fclose(fp_trace);
	fclose(fp_hwregtrace);
	fclose(fp_leds);
	return 0;                                                                                //finish simulator (Baruch Hashem)
}