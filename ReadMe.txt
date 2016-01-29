========================================================================
    Symbolic calculus
========================================================================

Test project to estimate C++x11 capabilities

Performs symbolic mathematical calculations, including:

 * 4 arithmetic types: integer, rational, real and complex numbers
 * extendable set of built-in functions: sin, cos, ln, etc.
 * user-defined symbols and functions
 * derivatives and integrals
 * approximate calculations
 * matching, substitution

 Result of calculation can be rendered into mathml or plain-text format.

 Project contains simple parset for mathematical expressions and Win32 GDI 
 render of mathml format.

/////////////////////////////////////////////////////////////////////////////
Author:

Nick Yeogorv, nick.yegorov@gmail.com

/////////////////////////////////////////////////////////////////////////////
Usage:

To use calculation in your project, just include header "calculus.h". If you 
need expression parser, include also header "parser.h".

/////////////////////////////////////////////////////////////////////////////
Other notes:

Project depends on Boost library, especially on boost::variant type.
Reformath library (http://reformath.weebly.com) is used to render mathml.

/////////////////////////////////////////////////////////////////////////////
