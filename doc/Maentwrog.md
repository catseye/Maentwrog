The Maentwrog Programming Language
==================================

This is a rudimentary specification for the Maentwrog programming
language.

This information, and the example programs in this distribution, were
taken from the esolangs.org wiki page for Maentwrog, written by
User:Marinus and (like all esowiki articles) placed in the public
domain. Thanks Marinus!

### Syntax ###

A Maentwrog program consists of a series of words, separated by
whitespace. Words can contain any character except whitespace. The way
these words are executed depends on the character they begin with.

-   A word that consists of digits is taken as an integer and pushed. A
    minus sign may be used to make negative numbers, so `25` and `-14`
    are words that push 25 and -14 onto the stack. Extra characters at
    the end of the word are allowed, so `25abc` and `25.14` both also
    push 25.
-   A word that isn't a number is treated as either a function or a
    variable. If the word is defined as a function, it is executed; if
    it's a variable, its current value is pushed to the stack. Using an
    undefined word results in an error, though this doesn't stop further
    execution of the program.
-   To define a word as a function, use the syntax
    `: new-word words to execute ;` (as in Forth). Redefining a word is
    not allowed, and neither are nested function definitions.
-   To define a word as a variable, use the syntax `*varname`. This must
    be done before using a variable.

Additionally, words (except number words) can take one of a list of
prefixes, changing how the word is executed.

    Prefix   Action               Example     Result
    -----    ------               -------     ------
    =        Assign to variable   =foo        A value popped from the stack is
                                              assigned to the variable foo.
    @        If                   @bye        Pop value, stop the program (see
                                              predefined words) if it isn't 0.
    [        While                [xyz        Pop value, if it's not 0 execute
                                              word xyz, then pop another value
                                              and do it again; continue until a
                                              0 is popped.
    $        Repeat               $.          Pop value, then output that many
                                              values from the stack.

#### Predefined words ####

    Word           Stack effect   Description
    ----           ------------   -----------
    bye                           Stop program immediately.
    rem ... ;                     Comment. (Ignore all words between rem and ;.)
    : word ... ;                  Define a new word.
    debug                         Turn on debugging (outputs all words executed).
    vars                          Output a list of currently defined variables
                                    and their values.
    words                         Output a list of currently defined words.
    alloc            n | ptr      Allocate memory for n C longs, returns a pointer.
    free           ptr | -        Free memory at pointer.
    size             - | n        Push stack size.
    dup              a | a a      Duplicate top of stack.
    swap           a b | b a      Swap the two topmost stack values.
    pop              a | -        Remove top value from stack.
    get            ptr | value    Push value at pointer to stack (C `*ptr`).
    put            p v | -        Store value at pointer (C `*ptr = val`).
    rnd              - | n        Push random value.
    >              a b | (a>b)    Push 1 if a is greater than b, else 0.
    <              a b | (a<b)    Push 1 if a is less than b, else 0.
    ==             undefined      Undefined.
    .                n | -        Pop a value, output as integer, adding a newline.
    ..               n | -        Pop a value, output as an ASCII character.
    mod            a b | (a%b)    Modulo.
    +              a b | (a+b)    Addition.
    -              a b | (a-b)    Subtraction.
    *              a b | (a*b)    Multiplication.
    /              a b | (a/b)    Division, result is rounded towards 0.


