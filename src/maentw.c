/*
 * maentw.c
 *
 * Maentwrog (RPN calculator & simple interpreted language)
 * - derived from rpn, Aug 1993 Chris Pressey
 * - updated Jul 1997 Chris Pressey, fixed minor bugs
 * - updated Jul 1998 Chris Pressey, fixed more minor bugs
 * -         and ANSI C-ized: now case sensitive
 * - updated Jul 2010 Chris Pressey, buildability w/gcc & pcc
 * Usage : maentw maentw-expressions   executes and exits
 *         maentw                      goes into interactive mode
 *         maentw <maentwrog-file      runs file through maentw
 *
 * This work is in the public domain.  See the file UNLICENSE for more info.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEFSIZE 1024

struct stack                    /* stack structure, for values on stack */
{
  signed long val;
  struct stack *next;
} *head;                        /* head of stack */

struct word                     /* word structure, for words (pre- and user- */
{                               /* defined) */
  char name[80];                /* name of word */
  char *macro;                  /* macro (user-defined; only executed if
                                 * fcn==0) */
  int fcn;                      /* built-in fcn or 0 for user-defined fcn */
  struct word *next;
} *whead;                       /* head of words */

struct vari                     /* variable structure, for variables */
{
  char name[80];                /* name of variable */
  signed long value;                    /* values of variable (signed longs only) */
  struct vari *next;
} *vhead;                       /* head of variables */

int debug=0;

/* prototypes */

/* word-handling */
struct word *addword(const char *, const char *, int);
struct word *lookup(const char *);
void initwords(void);
void makeword(void);

/* variable-handling */
struct vari *addvari(char *);
struct vari *getvari(char *);
void setvari(char *, signed long);

/* stack-handling */
void push(signed long);
signed long pop(void);

/* functions */
void dofunc(struct word *);
void words(void);
void vars(void);
signed long sizestack(void);

/* parsing and interpreting */
char *strdupe(const char *);
void process(char *);
void procstr(char *);

/* entry point */
int main(int, char **);

int main(argc, argv)
  int argc;
  char **argv;
{
  char s[80];
  int i;

  head = NULL;                  /* init */
  whead = NULL;
  vhead = NULL;
  initwords();
  if (argc != 1)
  {
    for(i=2;i<=argc;i++) procstr(argv[i-1]);
    exit(0);
  }
  scanf("%s", s);               /* process commands/values from stdin */
  while (!feof(stdin))
  {
    process(s);
    scanf("%s", s);
  }
  return 0;
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
 * if word starts with a @, the stack is popped, is checked for the boolean
 * value, and if true, executes it. (if.)
 *
 * if word starts with a $, the stack is popped, and command is repeated
 * that number of times.
 *
 * if word starts with a [, the stack is popped, is checked for the boolean
 * value, and if true, executes it.  Then goes back and pops the stack
 * again (while.)
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
  int i;

  if(debug) printf("%s ", s);

  if (isdigit((int)s[0]) || (s[0] == '-' && isdigit((int)s[1])))
    push(atoi(s));
  else if (s[0] == '=')
    setvari(s + 1, pop());
  else if ((s[0] == '*') && (isalpha((int)s[1])))
    addvari(s + 1);
  else if (s[0] == '@')
  {
    if (pop())
      if ((w = lookup(s + 1)))
        dofunc(w);
  }
  else if (s[0] == '$')
  {
    for(i=pop();i;i--)
      if ((w = lookup(s + 1)))
        dofunc(w);
  }
  else if (s[0] == '[')
  {
    for(;;)
    {
      if (pop())
      {
        if ((w = lookup(s + 1)))
          dofunc(w);
      } else break;
    }
  }
  else if ((w = lookup(s)))
    dofunc(w);
  else if ((v = getvari(s)))
    push(v->value);
  else
    printf("unknown command '%s'\n", s);
}

char *strdupe(const char *s)
{
  char *t = malloc(strlen(s) + 1);
  strcpy(t, s);
  return t;
}

/*
 * processes each word in the string s.
 * strtok doesn't work with recursion :-(
 */
void procstr(char *s)
{
  char *h=strdupe(s);
  char *g, *gg;
  g = h;

  for (;;)
  {
    gg = g;
    while (!isspace((int)gg[0])&&gg[0])
      gg++;
    if (!gg[0])
      break;
    gg[0] = 0;

    process(g);

    gg[0] = ' ';
    g = gg;
    while (isspace((int)g[0])&&g[0])
      g++;
  }

  free(h);                      /* called with strdupe(), so we must free */
}

/*
 * adds a unique word to the list of words.
 */
struct word *addword(const char *name, const char *macro, int fcn)
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
  new->macro = strdupe(macro);
  new->fcn = fcn;

  new->next = whead;
  whead = new;
  return new;
}

/*
 * attempts to find the word 'name' in the words list.  returns NULL if it
 * could not be found.
 */
struct word *lookup(const char *name)
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
  addword("rem", "", 199);
  addword("debug", "", 198);
  addword("vars", "", 101);
  addword("words", "", 100);

  addword("free", "", 91);
  addword("alloc", "", 90);

  addword(";", "", 81);
  addword(":", "", 80);

  addword("size", "", 50);
  addword("dup", "", 51);
  addword("swap", "", 52);
  addword("pop", "", 53);

  addword("get", "", 45);
  addword("put", "", 44);

  addword("rnd", "", 40);

  addword(">", "", 22);
  addword("<", "", 21);
  addword("==", "", 20);

  addword(".", "", 1);
  addword("..", "", 6);

  addword("mod", "", 30);

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
  char *y;
  unsigned int size = DEFSIZE;

  y = (char *)malloc(size);
  scanf("%s", s);
  strcpy(y, "");
  scanf("%s", t);
  while (strcmp(t, ";"))
  {
    if ((strlen(y)+strlen(t))>size)
    {
      printf("out of memory\n");
      exit(0);
    }
    strcat(y, t);
    strcat(y, " ");
    scanf("%s", t);
  }
  {
    char *n = (char *)malloc(strlen(y)+1);
/*  printf("(usage : %d bytes)\n", strlen(y)+1); */
    strcpy(n, y);
    free(y);
    y = n;
  }
  addword(s, y, 0);
}

/*
 * pushes a value onto the stack.
 */
void push(signed long val)
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
signed long pop()
{
  signed long v;
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
    return 0;
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
  return v;
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
void setvari(char *name, signed long value)
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
 * lists all words in mw's words list
 */
void words()
{
  struct word *w;
  for (w = whead; w; w = w->next)
    printf("%s ", w->name);
  printf("\n");
}

signed long sizestack()
{
  signed long total = 0;
  struct stack *s = head;

  while(s)
  {
    total++;
    s=s->next;
  }
  return total;
}

/*
 * list all variables in mw's vari list
 */
void vars()
{
  struct vari *v;
  for (v = vhead; v; v = v->next)
    printf("%-16s %ld\n", v->name, v->value);
}

/*
 * execute function of a word.  if w->fcn is 0, it will run processtr on the
 * word.  otherwise, a built-in function will be called. see the rpn
 * documentation on built-in functions.
 */
void dofunc(struct word * w)
{
  signed long a, b;
  signed long *ax;
  switch (w->fcn)
  {
    case 1:                     /* output (.) */
      printf("%ld\n", pop());
      break;
    case 2:                     /* add (+) */
      push(pop() + pop());
      break;
    case 3:                     /* subtract (-) */
      a = pop();
      b = pop();
      push(b - a);
      break;
    case 4:                     /* multiply (*) */
      push(pop() * pop());
      break;
    case 5:                     /* divide (/) */
      a = pop();
      b = pop();
      push(b / a);
      break;
    case 6:                     /* output ASCII (..) */
      printf("%c", (char)pop());
      break;
    case 20:
      push(!pop() == !pop());
      break;
    case 21:
      push(pop()>pop());
      break;
    case 22:
      push(pop()<pop());
      break;
    case 30:
      a = pop();
      b = pop();
      push(b % a);
      break;
    case 40:
      push(a);                  /* something random */
      break;
    case 44:                    /* put */
      a = pop();
      b = pop();
      if (b % 4)
        printf("must be longword boundary\n"); else
        {
          ax = (signed long *)b;
          *ax = a;
        }
      break;
    case 45:
      a = pop();
      if (a % 4)
        printf("must be longword boundary\n"); else
        {
          ax = (signed long *)a;
          push(*ax);
        }
      break;
    case 50:
      push(sizestack());        /* size of stack (size) */
      break;
    case 51:                    /* duplicate top element (dup) */
      a = pop();
      push(a); push (a); break;
    case 52:                    /* swap top elements (swap) */
      a = pop();
      b = pop();
      push(a); push (b);
      break;
    case 53:                    /* pop element (pop) */
      pop();
      break;
    case 80:                    /* define word (: ... ;) */
      makeword();
      break;
    case 81:                    /* null (;) */
      break;
    case 90:
      a = pop();
      push((signed long)malloc(a*sizeof(signed long)));
      break;
    case 91:
      free((void *)pop());
      break;
    case 100:                   /* list known words (words) */
      words();
      break;
    case 101:                   /* list variables (vars) */
      vars();
      break;
    case 198:
      debug = 1;
      break;
    case 199:                   /* rem */
    {
      char t[80];
      scanf("%s", t);
      while (strcmp(t, ";"))
        scanf("%s", t);
    }
      break;
    case 200:                   /* exit mw (bye) */
      exit(0);
      break;
    default:                    /* user-defined word */
      procstr(w->macro);
  }
}
