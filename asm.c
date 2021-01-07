
#define _CRT_SECURE_NO_DEPRECATE 
#define IMEM_LINE 6
#define IMEM_SIZE 1024
#define DMEM_LINE 8
#define DMEM_SIZE 4096
#define MAX_LINE 501
#define MAX_LABEL 51

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
	char opcode[3];
	int rd;
	int rs;
	int rt;
	int imm;
	char line_with_label[MAX_LABEL]; // if there is a label at the begining of the line
	char immediate_label[MAX_LABEL]; // if the immediate is a label
}_command, *Command;

// defines the struct that will hold a .word command 
typedef struct _word 
{
	int address;
	int data;
}_word, *Word;

// defined the struct that will hold a label command  
typedef struct _label
{
	char label[MAX_LABEL];
	int line;
}_label, *Label_and_Address;


// section 2 - functions declarations //

// functions dedicated to 'scan_word_line' function
int is_negative(char* str, int type);
int from_str_to_int(char *str, int type);
int convert_neg_hexa_to_int(char *str, int type);
Word scan_word_line(char* line);

// functions dedicated to 'scan_reg_line' function
int assign_opcode(char *opcode);
int assign_register(char *reg);
Command scan_reg_line(char* line);

// other functions
int type_of_line(char* line);
// void remove_spaces(char* clean_label, const char* label);
int give_address_of_a_label(Label_and_Address *labels, char* curr_label);

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
	int i = 0;  // lines counter for imemin
	int j = 0;  // labels counter
	size_t k = 0;
	int c = 0;  // commands counter
	int w = 0;  // .word counter
	// char instruction_memin[IMEM_SIZE][IMEM_LINE];	// size of imemin
	char data_memin[DMEM_SIZE][DMEM_LINE];	// size of dmemin
	char transition_string[MAX_LINE];
	FILE *asm;  // reading file
	FILE *imemin;  // writing file
	FILE *dmemin;  // writing file
	Command *commands;
	Word *words;
	Label_and_Address *labels;  

	// array of the commands data, as read from asm file
	commands = (Command)malloc(IMEM_SIZE * sizeof(_command));
	if (commands == NULL)
	{
		printf("Error: memory was not allocated properlly.");
		return 1;
	}

	// array of the Labels data
	labels = (Label_and_Address)malloc(IMEM_SIZE * sizeof(_label));
	if (labels == NULL) 
	{
		free(commands);
		printf("Error: memory was not allocated properlly.");
		return 1;
	}
	
	// array of the '.word' data
	words = (Word)malloc(IMEM_SIZE * sizeof(_word));
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
		printf("Program encountered a problem while accessing the assembly file.");
		exit(1);
	}

	
	/// part b - reading from program.asm file ///
	while (fgets(line, MAX_LINE, asm) != NULL)
	// i increases only for a 'command', 'label' or 'label+command' lines
	{
		int line_type = 0;
		line_type = type_of_line(line);  // 0 - command, 1 - .word, 2 - only comment
		switch (line_type)
		{
		case 2: // line contains only a comment - continue
		{	continue; }
		case 1:  // line is .word command
		{
			words[w] = scan_word_line(line);
			w++;
			continue;
		}
		case 0: // line is reg command
		{
			commands[i] = scan_reg_line(line);
			if (strcmp(commands[i]->opcode, "16") == 0)  // is an only label line
			{
				Label_and_Address curr_label = (Label_and_Address)malloc(sizeof(*curr_label));
				if (labels[j] == NULL)
					// exit(1)
				 	mem_file_close(commands, i, labels, j, words, w, asm, 1);
				strcpy(curr_label->label, commands[i]->line_with_label);
				curr_label->line = c;
				labels[j] = curr_label;
				j++; i++;
				continue;
			}
			else  // regular line
			{
				if (commands[i]->line_with_label[0] != '0')  // had also a label in beginning of the line
				{
					Label_and_Address curr_label = (Label_and_Address)malloc(sizeof(*curr_label));
					if (labels[j] == NULL)
						// exit(1)
						mem_file_close(commands, i, labels, j, words, w, asm, 1);
					strcpy(curr_label->label, commands[i]->line_with_label);
					curr_label->line = c;
					labels[j] = curr_label;
					free(curr_label);
					j++; c++;
				}
				if (commands[i]->rd == '1' || commands[i]->rs == '1' || commands[i]->rt == '1')  // has imm
					c++;
			}
			c++;  i++;
		}
		}
	}
	

	/// part c - writing to imemin.txt file ///
	imemin = fopen(argv[2], "w");
	if (imemin == NULL)
		// exit(1)
		mem_file_close(commands, i, labels, j, words, w, asm, 1);

	// 2nd loop - filling in the commands in imemin file
	for (k = 0; k < i; k++)
	{
		// handle only label line
		if (strcmp(commands[k]->opcode, "16") == 0)  // only label
		{
			commands[k]->imm = give_address_of_a_label(labels, commands[k]->line_with_label);
			fprintf(imemin, "%05X\n", commands[k]->imm);  // max adress is 1023 - 0x003ff 
			continue;
		}
		if (commands[k]->line_with_label[0] != '0')  // had also a label in beginning of the line
		{
			commands[k]->imm = give_address_of_a_label(labels, commands[k]->line_with_label);
			fprintf(imemin, "%05X\n", commands[k]->imm);  // max adress is 1023 - 0x003ff 
		}
		// 5 hexa of the command itself
		fprintf(imemin, "%s%c%c%c\n", commands[k]->opcode, commands[k]->rd, commands[k]->rs, commands[k]->rt);
		// check for imm
		if (commands[k]->rd == '1' || commands[k]->rs == '1' || commands[k]->rt == '1')  // has imm
		{
			if (commands[k]->immediate_label[0] != '0')  // imm is a label
			{
				commands[k]->imm = give_address_of_a_label(labels, commands[k]->immediate_label);
				fprintf(imemin, "%05X\n", commands[k]->imm);  // max adress is 1023 - 0x003ff 
			}
			else									     // imm is a int
			{
				sprintf(transition_string, "%08X", commands[k]->imm);
				fprintf(imemin, "%c%c%c%c%c\n", toupper(transition_string[3]), toupper(transition_string[4]), toupper(transition_string[5]),
										  toupper(transition_string[6]), toupper(transition_string[7]));
			}
		}
	}
	fclose(imemin);
	
	/// part d - writing to dmemin.txt file ///
	//loop for the .word commands
	for (int x = 0; x < DMEM_SIZE; x++)
		for (int y = 0; y < DMEM_LINE; y++)
			data_memin[x][y] = 48;  // initialize with zeros '0'
	for (int k = 0; k < w; k++)  // goes over words array
	{
		sprintf(transition_string, "%08X", words[k]->data);
		strncpy(data_memin[words[k]->address], transition_string, 8);
	}

	dmemin = fopen(argv[3], "w");
	if (dmemin == NULL)
		// exit(1)
		mem_file_close(commands, i, labels, j, words, w, asm, 1);

	for (int k = 0; k < DMEM_SIZE; k++)
		fprintf(dmemin, "%c%c%c%c%c%c%c%c\n", data_memin[k][0], data_memin[k][1], data_memin[k][2], data_memin[k][3],
											  data_memin[k][4], data_memin[k][5], data_memin[k][6], data_memin[k][7]);
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
// returns 1 if line is a .word command
// returns 2 if line is a only a comment
// returns 0 if regular line
{
	size_t len = strlen(line);
	size_t f = 0;
	while (f <= len)
	{
		if (isspace(line[f]))
		{
			f++;
			continue;
		}
		if (line[f] == '.')  // .word
			return 1;
		if (line[f] == '#' || line[f] == '\0')  // only a comment
			return 2;
		else
			return 0;
	}
	return 0;
}

//return 1 if the str is negative
// type = 0 -> data, 8 hexa long
// type = 1 -> instruction, 5 hexa long
int is_negative(char* str, int type)
{
	if (strncmp(str, "-", 1) == 0)  // first char is '-'
		return 1;
	else if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) // num is hexa based
	{
		if (type)  // 5 hexa
		{
			if (strlen(str) < 7)  // string has less than 8 hexa -> pos number
				return 0;
			if (strcmp(str, "0x7ffff") > 0)
				return 1;
		}
		else  // 8 hexa
		{
			if (strlen(str) < 10)  // string has less than 8 hexa -> pos number
				return 0;
			if (strcmp(str, "0x7fffffff") > 0)
				return 1;
		}
	}
	return 0;
}

//convert a string to an int
// type = 0 -> data, 8 hexa long
// type = 1 -> instruction, 5 hexa long
int from_str_to_int(char *str, int type)
{
	if (type == 1)  // 5 hexa, instruction
	{
		if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) //need to convert hexa to int
		{
			// deal with hexa
			if (is_negative(str, type)) //negative number
			{
				if (convert_neg_hexa_to_int(str, type) == -524288)
					return -524288;
				return (convert_neg_hexa_to_int(str, type) % (1 << 19));
			}
			return (int)strtol(str, NULL, 0) % (1 << 19);
		}
		else
		{
			if (atoi(str) == -524288)
				return -524288;
			return atoi(str) % (1 << 19);  // convert str to int
		}
	}
	else // 8 hexa, data
	{
		if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) //need to convert hexa to int
		{
			// deal with hexa
			if (is_negative(str, type)) //negative number
				return convert_neg_hexa_to_int(str, type);
			return (int)strtol(str, NULL, 0);
		}
		else
			return atoi(str);  // convert str to int
	}
}

// converts negative hexa to signed int
int convert_neg_hexa_to_int(char *str, int type)
// type = 0 -> data, 8 hexa long
// type = 1 -> instruction, 5 hexa long
{
	int i;
	int len;
	int step = 0;
	signed int res = 0;
	if (type)  // 5 hexa
		res |= (((1 << 12) - 1) << 20);  // set res  = 0xfff00000
	len = strlen(str)-1;
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

// scan word
Word scan_word_line(char* line)
{
	Word word;
	char *curr_word;
	int counter_word = 0;
	int j = 0;  // curr_word counter
	unsigned int i = 0;  // line counter
	size_t size = strlen(line);

	curr_word = (char*)malloc(500);
	if (curr_word == NULL)
	{
		printf("memory allocation error.");
		return NULL;
	}
	word = (Word)malloc(sizeof(*word));
	if (word == NULL)
	{
		free(curr_word);
		printf("memory allocation error.");
		return NULL;
	}
	while (isspace(line[i]))
	{
		i++;
		continue;
	}
	for (i; i < strlen(line); i++)
	{
		if ((counter_word == 0 || counter_word > 3) && isspace(line[i]))
			//ignore spaces in beginning of line, and after data was read
			continue;
		if ((counter_word == 2 || counter_word == 3) && isspace(line[i]) && j == 0)
			//ignore spaces between .word and address, or between address to data
			continue;
		if ((counter_word >= 1) && (isspace(line[i]) || i == size - 1))
		{
			if ((i == size - 1) && (line[size - 1] != '\n') && (counter_word == 3)) // extreme case- if there is no '\n' at the end of the line
			{
				curr_word[j] = line[i];
				curr_word[j + 1] = '\0';
				word->data = from_str_to_int(curr_word, 0);
				break;
			}
			curr_word[j] = '\0';
			j = 0;
			if (counter_word == 2)							//we found the address
			{
				if (is_negative(curr_word, 1))  // 1 standes for 5 hexa imm
					word->address = DMEM_SIZE + (from_str_to_int(curr_word, 0) % (1 << 12));
				else
					word->address = from_str_to_int(curr_word, 0) % DMEM_SIZE;
			}
			else if (counter_word == 3)						//we found the data
				word->data = from_str_to_int(curr_word, 0);
			counter_word++;
			continue;
		}
		if (j == 0 && counter_word == 0)
			counter_word++;
		curr_word[j] = line[i];
		j++;
	}
	free(curr_word);
	return word;
}

//convert the register to its numeric value
int assign_register(char *reg)
{
	const char *regs[] = { "$zero","$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra" };
	int i;
	for (i = 0; i < 16; i++)
		if (strcmp(reg, regs[i]) == 0)
			break;
	if (i < 10)
		return i + 48;  // 0-9 in hexa
	else
		return i + 55;  // A-F in hexa
}

//convert the opcode to its value
int assign_opcode(char *opcode)
{
	const char *opcodes[] = { "add", "sub", "and", "or" , "xor", "mul", "sll", "sra" , "srl", "beq", "bne",
							  "blt", "bgt", "ble", "bge", "jal", "lw" , "sw" , "reti", "in" , "out", "halt" };
	int i;
	for (i = 0; i < 22; i++)
		if (strcmp(opcode, opcodes[i]) == 0)
			break;
	return i;
}

//returns the line of the Label (the line we should jump to)
int give_address_of_a_label(Label_and_Address* labels, char* curr_label)
{
	int i = 0;
	// remove_spaces(lab[i]->label, lab[i]->label);
	// remove_spaces(label, label);
	while (strcmp(labels[i]->label, curr_label) != 0)
		i++;
	return labels[i]->line;
}

// function that reads a line and convert it to a command struct
Command scan_reg_line(char* line)
{
	bool has_label = 0;
	int j = 0;  // letters counter
	size_t len = strlen(line);
	int i = 0;  // line counter
	char *curr_word, curr_opcode[3];
	int counter_word = 0; // number of words we have found
	Command cm;

	curr_word = (char*)malloc(MAX_LINE);
	if (curr_word == NULL)
	{
		printf("memory allocation error.");
		return NULL;
	}
	cm = (Command)malloc(sizeof(*cm));
	if (cm == NULL)
	{
		free(curr_word);
		printf("memory allocation error");
		return NULL;
	}
	// cm->immediate_label = NULL;
	// cm->line_with_label = NULL;
	strcpy(cm->opcode, "00");
	strcpy(cm->line_with_label, "0");
	while (isspace(line[i]))
	{
		i++;
		continue;
	}
	for (i; i < len; i++)
	{
		if (counter_word != 1 && (counter_word != 5 || (counter_word == 5 && j == 0)))
		{
			if (isspace(line[i]) && i + 1 != len)
				continue;
		}
		if (has_label && line[i] == '#')  // end of line
		{
			if (counter_word == 0)
				// only label line - coded as '16'
			{
				cm->opcode[0] = '1';
				cm->opcode[1] = '6';
				cm->opcode[2] = '\0';
				cm->rd = 0; cm->rs = 0; cm->rt = 0;
				cm->imm = 0;
				strcpy(cm->immediate_label, "");
			}
			break;
		}
		if (counter_word == 1 && (isspace(line[i])))
			// handle opcode
		{
			curr_word[j] = '\0';
			j = 0;
			_itoa(assign_opcode(curr_word), curr_opcode, 16);  // convert int opcode to hexa-str opcode
			if (strlen(curr_opcode) == 1)
				cm->opcode[1] = curr_opcode[0];  // opcode is smaller than 16 - 1 letter
			else
				strcpy(cm->opcode, curr_opcode);  // opcode is larger than 16 - 2 letters
			counter_word++;
			continue;
		}

		if (line[i] == ':')// if we meet ":" it means its a Label line
		{
			has_label = 1;
			curr_word[j] = '\0';
			j = 0;
			strcpy(cm->line_with_label, curr_word); //copy the label to the field line_with_label
			counter_word = 0;
			continue;
		}
		if (line[i] == ',')
		{
			curr_word[j] = '\0';
			j = 0;

			switch (counter_word)
			{
			case 2:
				cm->rd = assign_register(curr_word);  // assign register number to rd
			case 3:
				cm->rs = assign_register(curr_word);  // assign register number to rs
			case 4:
				cm->rt = assign_register(curr_word);  // assign register number to rt
			}
			counter_word++;
			continue;
		}

		if ((counter_word == 5) && (isspace(line[i]) || line[i] == '\0') || (line[i] == '#' || i == len - 1)) //if we get to the fifth word of the line
		{
			if ((i == len - 1) && (isspace(line[len - 1]) || (line[len - 1] != '#'))) // extreme case- if there is no '\n'
			{
				curr_word[j] = line[i];
				curr_word[j + 1] = '\0';
				if (isalpha(curr_word[0]))
				{
					strcpy(cm->immediate_label, curr_word);
					cm->imm = 0;
				}
				else
				{
					cm->imm = from_str_to_int(curr_word, 1);
					strcpy(cm->immediate_label, "0");
				}
				continue;
			}
			curr_word[j] = '\0';
			j = 0;
			if (isalpha(curr_word[0]))
			{
				strcpy(cm->immediate_label, curr_word);
				cm->imm = 0;
			}
			else
			{
				cm->imm = from_str_to_int(curr_word, 1);
				strcpy(cm->immediate_label, "0");
			}
			break;
		}
		if (j == 0 && counter_word == 0) {
			counter_word++;
		}
		curr_word[j] = line[i];
		j++;
	}
	free(curr_word);
	if (isalpha(cm->opcode[1]))
		cm->opcode[1] -= 32;
	return cm;
}

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
// error = 0  -> close files and free memo, but no error encountered
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
}

