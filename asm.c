
#define _CRT_SECURE_NO_DEPRECATE 
#define IMEM_LINE 5
#define IMEM_SIZE 1024
#define DMEM_LINE 8
#define DMEM_SIZE 4096
#define MAX_LINE 500
#define MAX_LABEL 50

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> 
#include <malloc.h>
#include <stdbool.h>

// section 1 - structs definition //

// defines the struct that will hold an instruction command
typedef struct _command 
{
	char opcode;
	char rd;
	char rs;
	char rt;
	int imm;
}*Command;

// defines the struct that will hold a .word command 
typedef struct word_command 
{
	int address;
	int data;
}*Word;

// defined the struct that will hold a label command  
typedef struct label_address 
{
	char *label;
	int line;
}*Label_and_Address;


// section 2 - functions declarations //

// functions dedicated to 'scan_reg_line' function
int assign_opcode(char *opcode);
int assign_register(char *reg);
int from_imm_to_int(char *imm);
Command scan_reg_line(char* line);

// functions dedicated to 'scan_word_line' function
int from_str_to_int(char *str);
Word scan_word_line(char* line);

// functions dedicated to 'scan_label_line' function
Label_and_Address scan_label_line(char* line);

// other functions
int type_of_line(char* line);
int is_negative(char* str);
int neg_to_pos(signed int num);
void remove_spaces(char* clean_label, const char* label);
int give_address_of_a_label(Label_and_Address *lab, int n, char* label);

// free memory and error functions
void free_commands(Command* cmd, int n);
void free_labels(Label_and_Address* labels, int n);
void free_words(Word* words, int n);
void mem_file_close(Command* commands, int i,
	Label_and_Address* labels, int j,
	Word* words, int w,
	FILE* asm, int error);


////////////////////////
/// section 3 - main ///
////////////////////////


int main(int argc, char *argv[])
// part a - initialization
// part b - reading from program.asm file
// part c - writing to imemin.txt file
// part d - writing to dmemin.txt file
// part e - free memory and quit safely
{
	/// part a - initialization ///
	char line[MAX_LINE];
	char *currlabel = NULL;
	int i = 0;  // lines counter, used in while-loop
	int j = 0;
	int c = 0;  // commands counter
	int l = 0;  // labels counter
	int w = 0;  // .word counter
	char instruction_memin[IMEM_SIZE][IMEM_LINE];	// size of imemin
	char data_memin[DMEM_SIZE][DMEM_LINE];	// size of dmemin
	int last_instruction = 0;
	int last_data = 0; 
	char transition_string[9];
	FILE *asm;  // reading file
	FILE *imemin;  // writing file
	FILE *dmemin;  // writing file
	Command *commands;
	Word *words;
	Label_and_Address *labels;  // maybe change???

	// array of the commands data, as read from asm file
	commands = (Command*)malloc(IMEM_SIZE * sizeof(Command));
	if (commands == NULL)
	{
		printf("Error: memory was not allocated properlly.");
		return 1;
	}

	// array of the Labels data
	labels = (Label_and_Address*)malloc(IMEM_SIZE * sizeof(Label_and_Address));
	if (labels == NULL) 
	{
		free(commands);
		printf("Error: memory was not allocated properlly.");
		return 1;
	}
	
	// // array of the '.word' data
	words = (Word*)malloc(IMEM_SIZE * sizeof(Word));
	if (words == NULL) 
	{
		free(commands);
		free(labels);
		printf("Error: memory was not allocated properlly.");
		return 1;
	}

	// opening assembly program
	asm = fopen(argv[1], "r");
	if (asm == NULL) 
	{
		free(commands);
		free(labels);
		free(words);
		printf("The file was not opened successfully.");
		exit(1);
	}

	while (fgets(line, 40, asm)!=NULL)
		printf("%s", line);
	fclose(asm);
	
	/// part b - reading from program.asm file ///
	while (fgets(line, MAX_LINE, asm) != NULL)
	{
		int line_type = 0;
		line_type = type_of_line(line);  // 0 - command, 1 - .word, 2 - label 
		switch (line_type)
		{
		case 0: // line is reg command
		{
			commands[i] = scan_reg_line(line);
			if (commands[i] == NULL)  // mem allocation problem
				// exit(1)
				mem_file_close(commands, i, labels, j, words, w, asm, 1);

			i++;
		}
		case 1:  // line is .word command
		{
			words[w] = scan_word_line(line);
			if (words[w] == NULL)  // mem allocation problem
				// exit(1)
				mem_file_close(commands, i, labels, j, words, w, asm, 1);
			w++;
			continue;
		}
		case 2:  // line is label command	
		{
			labels[l] = scan_label_line(line);
			if (labels[l] == NULL)  // mem allocation problem
				// exit(1)
				mem_file_close(commands, i, labels, j, words, w, asm, 1);
			l++;
		}	
		}
	}

	//loop for the .word commands
	for (int k = 0; k < w; k++)  // goes over words array
	{
		sprintf(transition_string, "%08X", words[k]->data);
		strncpy(data_memin[words[k]->address], transition_string, 8);
	}
	for (int k = 0; k < DMEM_SIZE; k++)  // goes over data_memin array, and fill empty lines with 0's
		if (data_memin[k][0] != NULL)
		{
			sprintf(transition_string, "%08X", 0);
			strncpy(data_memin[words[k]->address], transition_string, 8);
		}


	/// part c - writing to imemin.txt file ///
	imemin = fopen(argv[2], "w");
	if (imemin == NULL)
		// exit(1)
		mem_file_close(commands, i, labels, j, words, w, asm, 1);
	for (int k = 0; k < IMEM_SIZE; k++)
		fprintf(imemin, "%c%c%c%c%c\n", instruction_memin[k][0], instruction_memin[k][1], instruction_memin[k][2], 
										instruction_memin[k][3], instruction_memin[k][4]);
	fprintf(imemin, EOF);
	fclose(imemin);

	/// part d - writing to dmemin.txt file ///
	dmemin = fopen(argv[3], "w");
	if (dmemin == NULL)
		// exit(1)
		mem_file_close(commands, i, labels, j, words, w, asm, 1);
	for (int k = 0; k < DMEM_SIZE; k++)
		fprintf(dmemin, "%c%c%c%c%c%c%c%c\n", data_memin[k][0], data_memin[k][1], data_memin[k][2], data_memin[k][3],
											  data_memin[k][4], data_memin[k][5], data_memin[k][6], data_memin[k][7]);
	fprintf(dmemin, EOF);
	fclose(dmemin);

	/// part e - free memory and quit safely ///
	mem_file_close(commands, i, labels, j, words, w, asm, 0);  // '0' stands for no memory or files problems =)
	
	// Great Success ! //
	return 0;
}



/////////////////////////////
/// section 4 - functions ///
/////////////////////////////


int type_of_line(char* line)
// checks the type of the line and returns:
// 0 - regular command
// 1 - .word command 
// 2 - Label command
{
	int f = 0;
	int type = 0;
	int len = strlen(line);
	while (f < len)
	{
		if (isspace(line[f]))
		{
			f++;
			continue;
		}
		if (line[f] == '$')  // reg command
			return (type);
		if (line[f] == '.')  // .word
			return (type + 1);
		if (line[f] == ':')  // Label
			return (type + 2);
		f++;
	}
	return type;  // regular command
}

//convert the register to its numeric value
int assign_register(char *reg)
{
	const char *regs[] = { "$zero","$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra"};
	int i;
	for (i = 0; i < 16; i++)
		if (strcmp(reg, regs[i]) == 0)
			return i;
}


// free memory and error functions
// freeing of commands memory 
void free_commands(Command* commands, int n)
{
	for (int i = 0; i < n; i++)
		if (commands[i] != NULL)
			free(commands[i]);
	free(commands);
}

// freeing of labels memory 
void free_labels(Label_and_Address* labels, int n)
{
	for (int i = 0; i < n; i++)
		if (labels[i] != NULL)
			free(labels[i]);
	free(labels);
}

// freeing of words memory 
void free_words(Word* words, int n)
{
	for (int i = 0; i < n; i++)
		if (words[i] != NULL)
			free(words[i]);
	free(words);
}

// frees all memory allocated, and closes open files
void mem_file_close(Command* commands,		  int i,
			   Label_and_Address* labels, int j,
			   Word* words,				  int w,
			   FILE* asm, int error)
{
	free_commands(commands, i);
	free_labels(labels, j);
	free_words(words, w);
	fclose(asm);
	if (error == 1)
	{
		printf("Error encountered.\n");
		exit(1);
	}
	// error = 0   means close files and free memo, but no error encountered
}

