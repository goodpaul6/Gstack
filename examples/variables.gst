# variables.gst -- variables examples

# program 1:

1 create val1
1 create val2 

val1 val2 @add @io.putn
@newline

# output:
# 2

# program 2:

1 create val1
{
	2 create val2
}
val1 val2 @add @io.putn
@newline

# output:
# ERROR! Variables only exist in their blocks (and the blocks within the initial block)

