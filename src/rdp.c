/*

   rdp.c

   a recursive decent parser (origins now lost, sorry)

   Chris Pressey, March, like, 7th, 1994

   This program is by no means complete, but it's as far as I'll
   go in the general case.  It should be no problem adapting it
   to do all sorts of really demented things.

   This work is in the public domain.  See the file UNLICENSE for more info.

 */

#define THIS_PROGRAM_IS_NOT_REALLY_AS_CRAPPY_AS_YOU_THINK

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define T_DELIMITER     0	/* delimeter token */
#define T_VARIABLE      1
#define T_CONSTANT      2

#define T_LENGTH        80

int toktype;			/* type of last token */
char token[T_LENGTH];		/* what gettoken returns - a token */
char prog[T_LENGTH];		/* the string to parse */

int t;				/* a 'pointer' to the pos in prog[] */
int result;			/* result of calculations */
int vars[26];			/* variables */

int level2 (int result);	/* recursion levels.  level 1 is no */
int level3 (int result);	/* longer needed (it assigned vars) */
int level4 (int result);
int level5 (int result);
int level6 (int result);
int primitive (int result);

/*-------------------------------------------------------------------------*/

int 
isdelimeter (char ch)
{
  switch (ch)
    {
    case ' ':
    case '+':
    case '-':
    case '/':
    case '*':
    case '%':
    case '=':
    case '>':
    case '<':
    case '(':
    case ')':
    case '$':
    case '!':
    case '&':
    case '|':
    case '^':
      return 1;
      break;

    default:
      return 0;
    }
}

void 
gettoken ()
{
  char temp[T_LENGTH];

  strcpy (token, "");
  token[1] = (char) 0;

  while (isspace (prog[t]))
    t++;
  if (prog[t] == '$')
    strcpy (token, "$");
  if (isdelimeter (prog[t]))
    {
      toktype = T_DELIMITER;
      token[0] = prog[t++];
    }
  else
    {
      if (isalpha (prog[t]))
	{
	  toktype = T_VARIABLE;
	  while (!(isdelimeter (prog[t])))
	    {
	      token[strlen (token) + 1] = (char) 0;
	      token[strlen (token)] = prog[t++];
	    }
	}
      else if (isdigit (prog[t]))
	{
	  toktype = T_CONSTANT;
	  while (!(isdelimeter (prog[t])))
	    {
	      token[strlen (token) + 1] = (char) 0;
	      token[strlen (token)] = prog[t++];
	    }
	}
    }
}

void 
putback ()
{
  t -= strlen (token);
}

void 
serror (int s)
{
  switch (s)
    {
    case 1:
      printf ("syntax error\n");
      break;
    case 2:
      printf ("mismatched parentheses\n");
      break;
    case 3:
      printf ("expression absent\n");
      break;
    }
}

/*
   if you define a new operator, use

   return (argument op operand)

   if you define a function (eg. sin(x)), use

   return (func(operand))

   if you define a backwards function (eg. x!) use

   return (func(argument))

   unary operators (such as -5 and !(a=b)) are not handled here, but are
   handled in level5()

 */
int 
arith (char op, int argument, int operand)
{
  switch (op)
    {
    case '+':
      return (argument + operand);	/* addition */
    case '-':
      return (argument - operand);	/* subtraction */
    case '*':
      return (argument * operand);	/* multiplication */
    case '/':
      return (argument / operand);	/* division */
    case '%':
      return (argument % operand);	/* modulus */
    case '=':
      return (argument == operand);	/* equality */
    case '>':
      return (argument > operand);	/* superiority */
    case '<':
      return (argument < operand);	/* inferiority */
    case '&':
      return (argument & operand);	/* binary AND */
    case '|':
      return (argument | operand);	/* binary OR */
    case '^':
      return (argument ^ operand);	/* binary XOR */
    }
}

int 
findvar (char *s)
{
  return vars[(int) toupper (s[0]) - (int) 'A'];
}

int 
getexp (int result)
{
  gettoken ();
  if (strlen (token))
    result = level2 (result);
  else
    serror (3);
  return result;
}

int 
level2 (int result)
{
  char op;
  int hold = 0;

  result = level3 (result);
  op = token[0];
  while ((op == '+') || (op == '-'))
    {
      gettoken ();
      hold = level3 (hold);
      result = arith (op, result, hold);
      op = token[0];
    }
  return result;
}

int 
level3 (int result)
{
  char op;
  int hold = 0;

  result = level4 (result);
  op = token[0];
  while ((op == '*') || (op == '/'))
    {
      gettoken ();
      hold = level4 (hold);
      result = arith (op, result, hold);
      op = token[0];
    }
  return result;
}

int 
level4 (int result)
{
  int hold = 0;
  char ch;

  result = level5 (result);
  ch = token[0];
  switch (ch)
    {
    case '=':
    case '>':
    case '<':
    case '&':
    case '|':
    case '^':
      gettoken ();
      hold = level4 (hold);
      result = arith (ch, result, hold);
      break;
    }
  return result;
}

int 
level5 (int result)
{
  char op;
  int hold = 0;

  op = ' ';
  if ((toktype == T_DELIMITER) && ((token[0] == '+') || (token[0] == '-')))
    {
      op = token[0];
      gettoken ();
    }
  result = level6 (result);
  if (op == '-')
    result *= (-1);
  if (op == '!')
    result = !result;
  return result;
}

int 
level6 (int result)
{
  if ((token[0] == '(') && (toktype == T_DELIMITER))
    {
      gettoken ();
      result = level2 (result);
      if (token[0] != ')')
	serror (2);
      gettoken ();
    }
  else
    result = primitive (result);
  return result;
}

int 
primitive (int result)
{
  if (toktype == T_CONSTANT)
    result = atoi (token);
  else if (toktype == T_VARIABLE)
    result = findvar (token);
  else
    serror (1);
  gettoken ();
  return result;
}

int 
main (int argc, char **argv)
{
  {
    int c;
    for (c = 0; c <= 25; c++)
      vars[c] = 0;
  }
  t = 0;

  strcpy (prog, argv[1]);
  strcat (prog, "$");
  result = getexp (result);
  printf ("%d\n", result);
  exit (0);




#ifdef THIS_PROGRAM_IS_NOT_REALLY_AS_CRAPPY_AS_YOU_THINK
}
#else
}
#endif

