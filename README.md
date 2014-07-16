Gstack
======

Gstack is a dynamic, interpreted, stack based, garbage collected programming language. It's purpose is to serve as an
abstraction over c code. Unlike lua or the like, it provides a simple interface for exposing native methods to the interpreter, 
and a simple means to managing metadata (native data). 

The syntax of the language is straightforward, with a lack of extreme syntactical sugar which can lead to a low amount of clarity.

A basic hello world program in Gstack would look as follows:

```
"hello world\n" @io.puts
```

As clearly visible, there are no brackets surrounding a call to a function. Quite simply put, every peice of syntax which is not a call or a higher order expression is a value. Said value is pushed onto the stack, and can be manipulated by native or user-defined functions.

Variables are declared using the 'create' keyword followed by the name of the variable. Before a variable is created, there must be a value on top of the stack, if you wish for the value of the variable to be 'NULL', simply use the value 0, and set it to the desired value later (by using the 'set' keyword followed by the name of the previously created variable)

variable example:

```
10 create value   # created a variable called value with the value of 10
"20" set value    # set the value of the variable to a string ("20")
```

Variables are relative to their scope, and thus, the following:


```
10 create value
{
	20 create value
	value @debug.println
}
value @debug.println
```

will output:

```
20
10
```

If one wished to set the value of the previous 'value' variable, one would use the 'set' keyword instead of the 'create' keyword:


```
10 create value
{
	20 set value
	value @debug.println
}
value @debug.println
```

output:


```
20 
20
```

Arithmetic is done through calls to the native functions:
@add, @sub, @mul, @div, @mod

The standard library will soon be fully documented.

Currently, there are 4 native data structures for use within the language:
Lists, Hash Tables, String Buffers, and Closures

Lists:


```
@list.new create mylist					# will create native list object and bind it to variable 'mylist'
mylist 10 @list.append					# will append the value 10 to the list
mylist 0 @list.get						# will put the value at index 0 in the list onto the stack (the value 10)
mylist @list.length						# will place the length of the list on stack as a number value
mylist 20 0 @list.set					# will set the value at index 0 to 20 (the index must be less than the length of the list, and greater than or equal to 0)
mylist @list.depend						# will remove the top item from the list (will fail if list is empty)
mylist 20 @list.resize					# will resize the list to the length of 20 (increasing the capacity if necessary)

@hash.new create mytable				# will create a native hash table object and bind it to the variable 'myhash'
mytable "test" 0 @hash.put				# will put the value 0 under the key 'test' for later retrival
mytable "test" @hash.exists				# will put a boolean value (denoting the existence of a key/value pair) on the stack
mytable "test" 1 @hash.set				# will set the value under the key "test" to 1
mytable "test" @hash.get				# will get the value under the key "test" and place it on the stack
mytable "test" @hash.remove				# will remove the key 'test' and it's value from the table altogether

@strbuf.new create mybuffer				# will create a native string buffer object and bind it to the variable 'mybuffer'
mybuffer 'c' @strbuf.char				# will append a char to the buffer
mybuffer "at\n" @strbuf.str				# will append a string to the buffer
mybuffer @strbuf.tostring @io.puts		# will convert the data in the buffer to a string object and place it onto the stack (and then print it out)
mybuffer @strbuf.clear					# will clear the buffer
mybuffer 10 @tostring @strbuf.str		# will append the number 10 as a string to the buffer

10 create val1
{
	0 create val2
	@closure.new create myclosure 			# will create a native closure object and bind it to the variable 'myclosure'
	myclosure @closure.capture				# will copy all variables in the environment and store them in the closure
	myclosure								# pass it on to the stack 
}
create myclosure							# store the closure in the variable myclosure

# this closure object can now be passed around as data and will keep all objects (at the 
# moment capture was called) alive and accessible.

myclosure "val2" @closure.get				# normally, val2 would have been lost, but the closure preserved it, and its value
myclosure "val2" 20 @closure.set			# you can set values in a closure as well 

# there is no need to free the list or anything else as the garbage collector will notify the native object 
# when it is no longer in use (and thus can be freed). 
```
The dot (.) syntax in Gstack has no semantic meaning, and is used for aesthetic purposes.


TO BE EXPANDED...



