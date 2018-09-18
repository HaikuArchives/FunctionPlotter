         File: FunctionPlotter0.9.2.zip
       Author: Michael Pfeiffer
      Contact: k3079e6@lion.cc
      Release: 0.9.2
 Release-Date: 5/7/00
Compatibility: R4.5, R5
        Notes: R4.5 and PPC-Versions have not been tested.
  Description: 

Plots an abitrary number of functions in parameter form.

The graph can be saved to a file as a bitmap, can be printed out
to a printer and can be copied to the clipboard.		 

Includes a small functional programming language to define constants
and functions. 
 

Shortcut Keys for Buttons: 
               Command+F ... Adds the current functions to the list
                             of functions.
			 Command+R ... Replaces the seltected function by the current
						 functions.
               Command+D ... Deletes the selected function from the list of
                             functions.

Mouse Actions inside graph:
			 Shift + any mouse button ... zoom in
               Control + any mouse button ... zoom out
               Holding any mouse button ... moves graph

Every text field may contain an expression. Before a function is plotted, the expressions are evaluated and the number values are put into constants which are named mainly after the field labels but in lower case letters. 
The evaluation order of the text fields is: 
	left, right, top, bottom, gridX, gridY
     from, to, by 

Operators: 
	* (may be omitted), /, -, +, - (sign), ^ (power), (expr) (brackets), 
     % (modulo)

Conditions:
	==, !=, <, <=, >, >=
	& (logical AND), | (OR), ! (NOT) 
	if expression1 then expression2 else expression3;

Definitions in the "Definitions" window only:
	constant = expression;
	function(p1, p2, ..., pn) = expression;

Line comment starts with: #

Supported Functions from the C-Math-Library: 
     sin, cos, tan, asin, acos, atan, sinh, cosh, tanh,
     exp, log, log10, sqrt, ceil, floor, abs.

Predefined Constants: 
	pi, e
Values from text fields:	
	left, right, top, bottom 
	from, to, by 
	gridX, gridY

Examples for expressions: 
     0.5 * x - 2  without '*': 0.5 x - 2
	sin(x) * x   or without '*': sin(x) x
	x^2
    sqrt(x)
    sqrt(1 - x ^ 2)

Examples for constant and function definitions:
	a = 1; b = 2;
	f(x) = x ^ 2;
	fact(n) = if n <= 0 then 1 else n * fact(n-1);

Grammar:
--------
Definitions = { Definition }.
Definition = ident ["(" IdentList ")"] "=" Expression ";".
IdentList = [ident {"," ident}].
Expression = Factor ["+" | "-" Expression].
Factor = Pow [(["*"] | "/" | "%") Factor].
Pow = Literal ["^" Literal].
Literal = {"-"} (ident ["(" Call ] | number | "(" Expression ")" | 
                 "if" Condition "then" Expression "else" Expression ).
Call = "(" [Expr {"," Expr}] ")". 
Condition = AndCond {"|" AndCond}.
AndCond = CompCond {"&" CompCond}.
CompCond = NotCond [("==" | "!=" | "<" | ">" | "<=" | ">=") NotCond].
NotCond = ["!"] Expression.

Literals:
ident = [a-zA-Z][a-zA-Z0-9]*
number = -?[0-9][0-9]*(\.[0-9]*)?(e-?[0-9][0-9]*)?

Limitations:
	call depth = 100
	number of calculated points per function <= 1000

To Do:
------
- improve interpreter 
	- constants folding
	- hash table access to global data (constants and functions)
	- better error messages 
- simpler user interface
- user readable (shorter) format for data files
