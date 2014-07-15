Gstack
======

Gstack is a dynamic, interpreted, stack based, garbage collected programming language. It's main purpose is embedding. It provides a simple interface for exposing native methods to the interpreter. 

The syntax of the language is straightforward, with a lack of extreme syntactical sugar which can lead to a low amount of clarity.

A basic hello world program in Gstack would look as follows:

"hello world" @debug.println

As clearly visible, there are no brackets surrounding a call to a function. Quite simply put, every peice of syntax which is not a call or a higher order expression is a value. Said value is pushed onto the stack, and can be manipulated by native or user-defined functions.

Variables are declared using the 'create' keyword followed by the name of the variable. Before a variable is created, there must be a value on top of the stack, if you wish for the value of the variable to be 'NULL', simply use the value 0, and set it to the desired value later (by using the 'set' keyword followed by the name of the previously created variable)

variable example:

10 create value   # created a variable called value with the value of 10
"20" set value    # set the value of the variable to a string ("20")

TO BE CONTINUED...

