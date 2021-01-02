
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
#include <inttypes.h>

typedef struct word_command
{
	int address;
	int data;
}*Word;

int is_negative(char* str);
long change_sign(long num);
int from_str_to_int(char *str);
int convert_neg_hexa_to_int(char *str);
Word scan_word_line(char* line);


int main(int argc, char *argv[])
{
	// open file through argv example
	/*FILE *fp = NULL;
	char line[500];
	fp = fopen(argv[1], "r");
	if (fp == NULL) 
	{
		
		printf("The file was not opened successfully.");
		return(1);
	}
	fgets(line, 40, fp);
	fclose(fp);
	printf("%s %s", "just in case\n", line);
	return 0;
	*/
	Word word1;
	Word word2;
	// long imm;
	word1 = (Word)malloc(sizeof(Word));
	if (word1 == NULL)
	{
		printf("memory allocation error.");
		return NULL;
	}
	word2 = (Word)malloc(sizeof(Word));
	if (word2 == NULL)
	{
		free(word1);
		printf("memory allocation error.");
		return NULL;
	}
	word2 = scan_word_line("	.word 0x100 0xffffffff # a \n");
	printf("\nthe address is: %d , and the data is: %d\n", word2->address, word2->data);
	
	return 0;
}


/////////// functions //////////

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

//convert a negative int to a positive int
long change_sign(long num)
{
	if (num == 0x80000000)
	{
		printf("number was out of range for sign change. 0 returned");
		return 0;
	}
	long number = num;
	// num = abs(num);
	long mask = 0xffffffff;
	number = number ^ mask;
	number++;
	return number;
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

// converts hexa to signed int
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


// scan word
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

