#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * caparse.c
 *
 * cellular automata parser handles 1-character values and operators :
 *
 * not          !
 * or           |
 * and          &
 * xor          ^
 * values       a, b, c, ... z
 * parenthesis  (, )
 *
 * This work is in the public domain.  See the file UNLICENSE for more info.
 */

#define OP_OR   0
#define OP_AND  1
#define OP_XOR  2

int caparse(char *string, int values[]);
int doper(int state, int oper, int modder);

main()
{
  int values[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0 };
  char in[255];

  while (!feof(stdin))
  {
    gets(in);
    if (isalpha(in[0])&&in[1]=='=')
      values[in[0]-'a']=(in[2]-'0');
      else
      printf("%d\n", caparse(in, values));
  }

}

int caparse(char *string, int values[])
{

#define sp string[pos]

  int negate = 0;
  int state  = 0;
  int oper   = OP_OR;
  int pos    = 0;

  for (;((sp) && (sp != ')'));pos++)
  {
    if (isalpha(sp))
    {
      state = doper(state, oper, negate ? !values[sp-'a'] : values[sp-'a']);
      negate = 0;
    } else
    switch (sp)
    {
      case '|' : oper = OP_OR; break;
      case '&' : oper = OP_AND; break;
      case '^' : oper = OP_XOR; break;
      case '!' : negate = 1; break;
      case '(' : pos++;
                 state = doper(state, oper,
                         negate ? !caparse(&sp, values) :
                                   caparse(&sp, values));
                 {
                   int bra = 1;
                   while (bra)
                   {
                     if (sp=='(') bra++;
                     if (sp==')') bra--;
                     printf("%d\n", bra);
                     if (bra) pos++;
                   }
                 }
                 break;
      default  : break;
    }
  }
  return state;
}

int doper(int state, int oper, int modder)
{
  switch (oper)
  {
    case OP_OR  : return(state | modder); break;
    case OP_AND : return(state & modder); break;
    case OP_XOR : return(state ^ modder); break;
  }
}

