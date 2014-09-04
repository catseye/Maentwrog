/*
 * rpn.c
 * 
 * RPN calculator - May 1993 Chris Pressey
 * 
 * Compilation : acc rpn.c -o rpn
 * 
 * Usage : rpn rpn-expressions		; executes and exits
 *         rpn				; goes into interactive mode
 *         rpn <rpn-file		; runs rpn-file through rpn
 * 
 * To do : add size (size stack), sum (sum stack), mean (mean stack), sd (take
 * standard deviation of stack)
 * 
 * This work is in the public domain.  See the file UNLICENSE for more info.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

struct stack			/* stack structure, for values on stack */
{
  double val;
  struct stack *next;
} *head;			/* head of stack */

struct word			/* word structure, for words (pre- and user- */
{				/* defined) */
  char name[80];		/* name of word */
  char macro[80];		/* macro (user-defined; only executed if
				 * fcn==0) */
  int fcn;			/* built-in fcn or 0 for user-defined fcn */
  struct word *next;
} *whead;			/* head of words */

struct vari			/* variable structure, for variables */
{
  char name[80];		/* name of variable */
  double value;			/* values of variable (doubles only) */
  struct vari *next;
} *vhead;			/* head of variables */

/* prototypes */

/* word-handling */
struct word *addword(char *name, char *macro, int fcn);
struct word *lookup(char *name);
void initwords();
void makeword();

/* variable-handling */
struct vari *addvari(char *name);
struct vari *getvari(char *name);
void setvari(char *name, double value);

/* stack-handling */
void push(double val);
double pop();

/* functions */
void dofunc(struct word * w);
double factorial(double p);
double fibonacci(double p);
void words();
void vars();

/* parsing and interpreting */
void process(char *s);
void procstr(char *s);

main(argc, argv)
  int argc;
  char **argv;
{
  char s[80];
  int i;

  head = NULL;			/* init */
  whead = NULL;
  vhead = NULL;
  initwords();
  if (argc != 1)
  {
    for(i=2;i<=argc;i++) procstr(argv[i-1]);
    exit(0);
  }
  i=scanf("%s", s);		/* process commands/values from stdin */
  while (!feof(stdin))
  {
    process(s);
    i=scanf("%s", s);
  }
}

/*
 * processes the word in s according to parsing rules.
 *
 * if word starts with a digit or a -digit, it is converted to a float and put
 * on the stack.
 * 
 * if word starts with a = then the variable following equals is assigned the
 * last value popped off the stack.
 * 
 * if word starts with a * a variable is defined using that word.
 * 
 * if word is in words list, that function is executed.
 * 
 * if word is in variables list, the value of it is pushed onto the stack.
 * 
 * otherwise generates an error.
 */
void process(char *s)
{
  struct word *w;
  struct vari *v;

  if (isdigit(s[0]) || ((s[0] == '-') && (isdigit(s[1]))))
    push(atof(s));
  else if (s[0] == '=')
    setvari(s + 1, pop());
  else if ((s[0] == '*') && (isalpha(s[1])))
    addvari(s + 1);
  else if (w = lookup(s))
    dofunc(w);
  else if (v = getvari(s))
    push(v->value);
  else
    printf("unknown command '%s'\n", s);
}

/*
 * processes each word in the string s.
 */
void procstr(char *s)
{
  char *h;
  char *g;
  h = strdup(s);
  g = strtok(h, " ");
  while (g)
  {
    process(g);
    g = strtok(NULL, " ");
  }
  free(h);			/* called with strdup(), so we must free */
}

/*
 * adds a unique word to the list of words.
 */
struct word *addword(char *name, char *macro, int fcn)
{
  struct word *new;
  for (new = whead; new; new = new->next)
    if (!strcmp(new->name, name))
    {
      printf("already exists\n");
      return NULL;
    }
  new = (struct word *) malloc(sizeof(struct word));
  strcpy(new->name, name);
  strcpy(new->macro, macro);
  new->fcn = fcn;

  new->next = whead;
  whead = new;
  return new;
}

/*
 * attempts to find the word 'name' in the words list.  returns NULL if it
 * could not be found.
 */
struct word *lookup(char *name)
{
  struct word *l = whead;
  struct word *k = NULL;

  while (l)
  {
    if (!strcmp(name, l->name))
    {
      k = l;
      l = NULL;
    } else
      l = l->next;
  }
  return (k);
}

/*
 * initialize the words list with all the built-in words.
 */
void initwords()
{
  addword("bye", "", 200);
  addword("vars", "", 101);
  addword("words", "", 100);
  addword(";", "", 81);
  addword(":", "", 80);

  addword("sd", "", 53);
  addword("mean", "", 52);
  addword("sum", "", 51);
  addword("size", "", 50);

  addword("!", "", 13);
  addword("fib", "", 14);

  addword("rnd", "", 40);

  addword("sign", "", 34);
  addword("abs", "", 33);
  addword("round", "", 32);
  addword("frac", "", 31);
  addword("int", "", 30);

  addword("phi", "", 27);
  addword("pi", "", 26);
  addword("atan", "", 25);
  addword("acos", "", 24);
  addword("asin", "", 23);

  addword("tan", "", 22);
  addword("cos", "", 21);
  addword("sin", "", 20);

  addword("inv", "", 12);
  addword("exp", "", 11);
  addword("log", "", 10);
  addword("ln", "", 9);
  addword("sqrt", "", 8);
  addword("sqr", "", 7);

  addword("^", "", 6);

  addword(".", "", 1);

  addword("/", "", 5);
  addword("*", "", 4);
  addword("-", "", 3);
  addword("+", "", 2);
}

/*
 * makes a word, reading between the : and ;, defining the new word's macro,
 * and adds it.
 */
void makeword()
{
  char s[80];
  char t[80];
  char y[180];
  int i;
  i=scanf("%s", s);
  strcpy(y, "");
  i=scanf("%s", t);
  while (strcmp(t, ";"))
  {
    strcat(y, t);
    strcat(y, " ");
    i=scanf("%s", t);
  }
  addword(s, y, 0);
}

/*
 * pushes a value onto the stack.
 */
void push(double val)
{
  struct stack *s;
  s = (struct stack *) malloc(sizeof(struct stack));
  s->val = val;
  s->next = head;
  head = s;
}

/*
 * pops a value off the stack. generates error and returns 0.0 in case of
 * underflow.
 */
double pop()
{
  double v;
  struct stack *s = head;
  if (s)
  {
    v = head->val;
    head = head->next;
    free(s);
    return v;
  } else
  {
    printf("stack underflow\n");
    return 0.0;
  }
}

/*
 * adds a unique variable to the vari list.
 */
struct vari *addvari(char *name)
{
  struct vari *v;
  for (v = vhead; v; v = v->next)
    if (!strcmp(v->name, name))
    {
      printf("already exists\n");
      return NULL;
    }
  v = (struct vari *) malloc(sizeof(struct vari));
  strcpy(v->name, name);
  v->next = vhead;
  vhead = v;
}

/*
 * gets the value of a variable off the variable list.
 */
struct vari *getvari(char *name)
{
  struct vari *l = vhead;
  struct vari *k = NULL;

  while (l)
  {
    if (!strcmp(name, l->name))
    {
      k = l;
      l = NULL;
    } else
      l = l->next;
  }
  return (k);
}

/*
 * sets the value of a variable
 */
void setvari(char *name, double value)
{
  struct vari *l = vhead;
  while (l)
  {
    if (!strcmp(name, l->name))
    {
      l->value = value;
      l = NULL;
    } else
      l = l->next;
  }
}

/*
 * calculates p!
 */
double factorial(double p)
{
  if (p <= 1.0)
    return p;
  else
    return (p * factorial(p - 1.0));
}

/*
 * calculates fib(p) (fib(1)==1, fib(2)==1, fib(n)==fib(n-1)+fib(n-2))
 */
double fibonacci(double p)
{
  if (p <= 2.0)
    return 1.0;
  else
    return (fibonacci(p - 1.0) + fibonacci(p - 2.0));
}

/*
 * lists all words in bf's words list
 */
void words()
{
  struct word *w;
  for (w = whead; w; w = w->next)
    printf("%s ", w->name);
  printf("\n");
}

/*
 * list all variables in bf's vari list
 */
void vars()
{
  struct vari *v;
  for (v = vhead; v; v = v->next)
    printf("%-16s %f\n", v->name, v->value);
}

/*
 * execute function of a word.  if w->fcn is 0, it will run processtr on the
 * word.  otherwise, a built-in function will be called. see the rpn
 * documentation on built-in functions.
 */
void dofunc(struct word * w)
{
  double a, b;
  switch (w->fcn)
  {
    case 1:			/* output (.) */
      printf("%f\n", pop());
      break;
    case 2:			/* add (+) */
      push(pop() + pop());
      break;
    case 3:			/* subtract (-) */
      a = pop();
      b = pop();
      push(b - a);
      break;
    case 4:			/* multiply (*) */
      push(pop() * pop());
      break;
    case 5:			/* divide (/) */
      a = pop();
      b = pop();
      push(b / a);
      break;
    case 6:			/* exponential (^) */
      a = pop();
      b = pop();
      push(pow(b, a));
      break;
    case 7:			/* square (sqr) */
      a = pop();
      push(a * a);
      break;
    case 8:			/* square root (sqrt) */
      push(sqrt(pop()));
      break;
    case 9:			/* natural logarithm (ln) */
      push(log(pop()));
      break;
    case 10:			/* log base 10 (log) */
      push(log10(pop()));
      break;
    case 11:			/* e^x (exp) */
      push(exp(pop()));
      break;
    case 12:			/* inversion (inv) */
      push(1.0 / pop());
      break;
    case 13:			/* factorial (!) */
      push(factorial(pop()));
      break;
    case 14:			/* fibonacci (fib) */
      push(fibonacci(pop()));
      break;
    case 20:			/* sine (sin) */
      push(sin(pop()));
      break;
    case 21:			/* cosine (cos) */
      push(cos(pop()));
      break;
    case 22:			/* tangent (tan) */
      push(tan(pop()));
      break;
    case 23:			/* arcsine (asin) */
      push(asin(pop()));
      break;
    case 24:			/* arccosine (acos) */
      push(acos(pop()));
      break;
    case 25:			/* arctangent (atan) */
      push(atan(pop()));
      break;
    case 26:			/* pi (pi) */
      push(3.1415926);
      break;
    case 80:			/* define word (: ... ;) */
      makeword();
      break;
    case 81:			/* null (;) */
      break;
    case 100:			/* list known words (words) */
      words();
      break;
    case 101:			/* list variables (vars) */
      vars();
      break;
    case 200:			/* exit bf (bye) */
      exit(0);
      break;
    default:			/* user-defined word */
      procstr(w->macro);
  }
}
