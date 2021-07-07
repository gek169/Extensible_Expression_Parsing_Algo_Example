#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stringutil.h"

static char* read_until_terminator_alloced_modified(FILE* f){
	unsigned char c;
	char* buf;
	unsigned long bcap = 40;
	char* bufold;
	unsigned long blen = 0;
	buf = malloc(40);
	if(!buf) return NULL;
	while(1){
		if(feof(f)){break;}
		c = fgetc(f);
		if(c == '\n' || c=='\r') {break;}
		if(c > 127) continue; /*Pretend it did not happen.*/
		if(c < 8) continue; /*Also didn't happen*/
		if(blen == (bcap-1))	/*Grow the buffer.*/
			{
				bcap<<=1;
				bufold = buf;
				buf = realloc(buf, bcap);
				if(!buf){
					free(bufold); 
					printf("<!UH OH! realloc failure.>\r\n");
					return NULL;
				}
			}
		buf[blen++] = c;
	}
	buf[blen] = '\0';
	return buf;
}


unsigned int vstack[1024];
unsigned long stp = 0;
char c = 0;

char* parse_expr(char*, char*);
void multiply()
{
	unsigned int a=0, b=0, lc = 0;
	if(stp < 2){
		printf("\r\n Error, not enough ops to multiply.");
		exit(1);
	}
	b = vstack[--stp];
	a = vstack[--stp];
	lc = a * b;
	vstack[stp++] = lc;
	printf("apop;bpop;mul;apush; [%u * %u = %u]\r\n", a,b,lc);
}

void divide()
{
	unsigned int a=0, b=0, lc = 0;
	if(stp < 2){
		printf("\r\n Error, not enough ops to divide.");
		exit(1);
	}
	b = vstack[--stp];
	a = vstack[--stp];
	if(b != 0)
		lc = a / b;
	else
		lc = 0;
	vstack[stp++] = lc;
	if(b != 0)
		printf("apop;bpop;mul;apush; [%u / %u = %u]\r\n", a,b,lc);
	else
		printf("apop;bpop;mul;apush; [%u / %u = %u, DIV_BY_ZERO]\r\n", a,b,lc);
}

void addition()
{
	unsigned int a=0, b=0, lc = 0;
	if(stp < 2){
		printf("\r\n Error, not enough ops to add.");
		exit(1);
	}
	b = vstack[--stp];
	a = vstack[--stp];
	lc = a + b;
	vstack[stp++] = lc;
	printf("apop;bpop;add;apush; [%u + %u = %u]\r\n", a,b,lc);
}

void subtraction()
{
	unsigned int a=0, b=0, lc = 0;
	if(stp < 2){
		printf("\r\n Error, not enough ops to sub.");
		exit(1);
	}
	b = vstack[--stp];
	a = vstack[--stp];
	lc = a - b;
	vstack[stp++] = lc;
	printf("apop;bpop;sub;apush; [%u - %u = %u]\r\n", a,b,lc);
}

char* parse_expr(char* in, char* ign_list){
	printf("call to parse_expr on %s\r\n", in);
	while(1){
		unsigned long i;
		if(in[0] == '\0'){
			printf("<nothing>\r\n");
			return in;
		}
		for(i=0; i < strlen(ign_list); i++)
					if(in[0] == ign_list[i])
					{
						printf("<reached %c in the ign_list %s>\r\n", ign_list[i], ign_list);
						return in;
					}
		if(in[0] == /*(*/')'){
			printf("<ERROR> bad parentheses, saw extraneous ending parenthese");
			exit(1);
		}else if(in[0] == '+'){
			unsigned long stp_saved = stp;
			if(c == 0){
				printf("bad prefixing number for binop +");
				exit(1);
			}
			c = 0;
			in++;
			in = parse_expr(in, "+-");
			if(stp != stp_saved +1){
				printf("bad expression, stack pointer is incorrect for binop +");
				printf("\r\nin: %s", in);
				exit(1);
			}
			addition();
		}else if(in[0] == '-'){
			unsigned long stp_saved = stp;
			if(c == 0){
				printf("bad prefixing number for binop -");
				exit(1);
			}
			c = 0;
			in++;
			in = parse_expr(in, "+-");
			if(stp != stp_saved +1){
				printf("bad expression, stack pointer is incorrect for binop -");
				printf("\r\nin: %s", in);
				exit(1);
			}
			subtraction();
		}else if(in[0] == '*'){
			unsigned long stp_saved = stp;
			if(c == 0){
				printf("bad prefixing number for binop *");
				exit(1);
			}
			c = 0;
			in++;
			in = parse_expr(in, "+-*/");
			if(stp != stp_saved +1){
				printf("bad expression, stack pointer is incorrect for binop *");
				printf("\r\nin: %s", in);
				exit(1);
			}
			multiply();
		}else if(in[0] == '/'){
			unsigned long stp_saved = stp;
			if(c == 0){
				printf("bad prefixing number for binop /");
				exit(1);
			}
			c = 0;
			in++;
			in = parse_expr(in, "+-*/");
			if(stp != stp_saved +1){
				printf("bad expression, stack pointer is incorrect for binop /");
				printf("\r\nin: %s", in);
				exit(1);
			}
			divide();
		}else if(isdigit(in[0])){
			if(c){
				vstack[stp-1] *= 10;
				vstack[stp-1] += in[0]-0x30;
				c = in[0];
				printf("push %u;\r\n", vstack[stp-1]);
				in++;
			} else {
				if(!isdigit(in[1]))
					printf("push %u;\r\n", in[0]-0x30);
				c = in[0];
				vstack[stp++] = c-0x30;
				in++;
			}
		} else if(in[0] == '(' /*)*/) {
			char* expr = NULL;
			unsigned long i = 1;
			unsigned long lvl = 1;
			in++; /*Skip over the starting parentheses*/
			printf("Saw opening parentheses...\r\n");
			if(c){
				printf("<ERROR> cannot follow digit with paren.\r\n");
				exit(1);
			}
			for(;i<strlen(in);i++){
				if(in[i] == '('/*)*/) lvl++;
				if(in[i] == /*(*/')') {
					lvl--;
					if(lvl==0) break;
				}
			}
			if(lvl){
				printf("<ERROR> bad parentheses. Expression is %s\r\n", in);
				exit(1);
			}
			expr = str_null_terminated_alloc(in, i);
			printf("Parsed Parenthesized expression is %s\r\n", expr);
			parse_expr(expr, "");
			free(expr);
			in += i+1;
		} else {
			printf("\r\n<ERROR> unrecognized token %c\r\n", in[0]);
			exit(1);
		}
	} /*Eof while.*/
}


int main(){
	char* line = read_until_terminator_alloced_modified(stdin);
	while(strfind(line, " ")!=-1){
		line = str_repl_allocf(line, " ", "");
	}
	while(strfind(line, "\t")!=-1){
		line = str_repl_allocf(line, "\t", "");
	}
	while(strfind(line, "\v")!=-1){
		line = str_repl_allocf(line, "\v", "");
	}
	while(strfind(line, "\f")!=-1){
		line = str_repl_allocf(line, "\f", "");
	}
	while(strfind(line, "\r")!=-1){
		line = str_repl_allocf(line, "\r", "");
	}
	printf("Whitespace Removed:\r\n%s\r\n", line);
	parse_expr(line,"");
	printf("stp = %lu", stp);
}
