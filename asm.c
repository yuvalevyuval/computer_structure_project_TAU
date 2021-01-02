
#define _CRT_SECURE_NO_DEPRECATE 
#define IMEM_LINE 5
#define IMEM_SIZE 1024
#define DMEM_LINE 8
#define DMEM_SIZE 4096
#define MAX_LINE 501
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
int is_negative(char* str);
int from_str_to_int(char *str);
int convert_neg_hexa_to_int(char *str);
Word scan_word_line(char* line);

// functions dedicated to 'scan_label_line' function
Label_and_Address scan_label_line(char* line);

// other functions
int type_of_line(char* line);
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
	int i = 0;  // lines counter in asm file
	int j = 0;  // labels counter
	int c = 0;  // commands counter
	int l = 0;  // lines counter in output file
	int w = 0;  // .word counter
	char instruction_memin[IMEM_SIZE][IMEM_LINE];	// size of imemin
	char data_memin[DMEM_SIZE][DMEM_LINE];	// size of dmemin
	int last_instruction = 0;
	int last_data = 0; 
	char sub_line[MAX_LINE];
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
	
	// array of the '.word' data
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
	// i increases only for a 'command', 'label' or 'label+command' lines
	{
		int line_type = 0;
		line_type = type_of_line(line);  // 0 - command, 1 - .word, 2 - only comment
		switch (line_type)
		{
		case 2: // line is contains only a comment - continue
			continue;
		case 1:  // line is .word command
		{
			words[w] = scan_word_line(line);
			if (words[w] == NULL)  // mem allocation problem
				// exit(1)
				mem_file_close(commands, i, labels, j, words, w, asm, 1);
			w++;
			continue;
		}
		case 0: // line is reg command
		{
			commands[i] = scan_reg_line(line);
			if (commands[i] == NULL)  // mem allocation problem
				// exit(1)
				mem_file_close(commands, i, labels, j, words, w, asm, 1);

			i++;
		}
		}
		/*case 2:  // line is label command	
		{
			while (line[j] != ':')
				j++;
			strncpy(sub_line, line, j+1);  // copies label + ':' to sub-line 
			sub_line[j + 2] = '\0';
			labels[l] = scan_label_line(sub_line);
			if (labels[l] == NULL)  // mem allocation problem
				// exit(1)
				mem_file_close(commands, i, labels, j, words, w, asm, 1);
			l++;

			// take care of the case which we have a command after the label 
			while (j <= strlen(line))
			{

			}
			*/	
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
	fclose(imemin);

	/// part d - writing to dmemin.txt file ///
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
	int f = 0;
	int len = strlen(line);
	while (f <= len)
	{
		if (isspace(line[f]))
		{
			f++;
			continue;
		}
		if (line[f] == '.')  // .word
			return 1;
		if (line[f] == '#')  // only a comment
			return 2;
		else
			return 0;  // regular line
	}
}

// function that reads a line and convert it to a command struct
Command scaning_line(char* line)
{
	bool onlyLabel = 0;
	int j = 0;
	int len = strlen(line);
	int m;
	char *curr_word;
	char counter_word = 0; // number of words we have found
	Command cm;

	curr_word = (char*)malloc(500);
	if (curr_word == NULL) {
		printf("memory allocation error");
		return NULL;
	}
	cm = (Command)malloc(sizeof(*cm));
	if (cm == NULL) {
		free(curr_word);
		printf("memory allocation error");
		return NULL;
	}
	cm->immediate_label = NULL;
	cm->line_with_label_only = NULL;
	cm->opcode = 21;
	for (int i = 0; i < strlen(line); i++)
	{
		if (counter_word != 1 && (counter_word != 5 || (counter_word == 5 && j == 0)))
		{
			if (isspace(line[i]))
				continue;
		}

		if (onlyLabel && line[i] == '#')
			break;

		if (counter_word == 1 && (isspace(line[i])))
		{
			curr_word[j] = '\0';
			if (strcmp(curr_word, ".word") == 0)
			{
				free(curr_word);
				cm->opcode = 20; // .word command
				return cm;
			}
			j = 0;
			cm->opcode = assign_opcode(curr_word);
			counter_word++;
			continue;
		}

		if (line[i] == ':')// if we meet ":" it means its a Label line
		{
			onlyLabel = 1;
			if (j > 0)
				curr_word[j] = '\0';
			j = 0;
			cm->line_with_label_only = (char*)malloc(strlen(curr_word + 1));
			if (cm->line_with_label_only == NULL) {
				free(curr_word);
				free(cm);
				printf("memory allocation error");
				return NULL;
			}

			strcpy(cm->line_with_label_only, line); //copy the label to the field line_with_label_only
			cm->line_with_label_only[strlen(line) - 2] = '\0';
			counter_word = 0;
			cm->opcode = 21;
			continue;
		}
		if (line[i] == ',')
		{
			curr_word[j] = '\0';
			j = 0;

			switch (counter_word)
			{
			case 2:
			{cm->rd = assign_register(curr_word); } //assign register number to rd
			case 3:
			{cm->rs = assign_register(curr_word); } //assign register number to rs
			case 4:
			{cm->rt = assign_register(curr_word); } //assign register number to rt
			}

			counter_word++;
			continue;
		}

		if ((counter_word == 5) && (isspace(line[i]) || line[i] == '#' || i == len - 1)) //if we get to the fifth word of the line
		{
			if ((i == len - 1) && (isspace(line[len - 1]) || (line[len - 1] != '#'))) // extreme case- if there is no '\n'
			{
				curr_word[j] = line[i];
				curr_word[j + 1] = '\0';
				if (isalpha(curr_word[0]))
					cm->immediate_label = curr_word;
				else
					cm->immediate = from_imm_to_int(curr_word);
				continue;
			}
			curr_word[j] = '\0';
			j = 0;
			if (isalpha(curr_word[0]))
				cm->immediate_label = curr_word;
			else
				cm->immediate = from_imm_to_int(curr_word);
			break;
		}
		if (j == 0 && counter_word == 0) {
			counter_word++;
		}
		curr_word[j] = line[i];
		j++;
	}

	return cm;
}

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
// assumes neg number based in hexa, 8 chars
{
	int i;
	int step = 0;
	signed int res = 0;
	// char res[33];
	for (i = 9; i > 1; i--)
	{
		step = (7 - (i - 2)) * 4;
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

//convert a line to a .word command 
Word scan_word_line(char* line)
{
	Word word;
	char *curr_word;
	int counter_word = 0;
	int j = 0;  // curr_word counter
	unsigned int i = 0;  // line counter
	int size = strlen(line);

	curr_word = (char*)malloc(500);
	if (curr_word == NULL)
	{
		printf("memory allocation error.");
		return NULL;
	}
	word = (Word)malloc(sizeof(Word));
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
				word->data = from_str_to_int(curr_word);
				break;
			}
			curr_word[j] = '\0';
			j = 0;
			if (counter_word == 2)							//we found the address
				word->address = from_str_to_int(curr_word);
			else if (counter_word == 3)						//we found the data
				word->data = from_str_to_int(curr_word);
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

