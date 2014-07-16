# objects.gst -- mimicking an object oriented paradigm
@list.new create objects 								# create a list object and bind it to a variable 'objects'

define object.new
{
	@hash.new create obj								# create a new hash table and bind it to the variable 'obj'
	obj "name" "object" @hash.put						# set the name of the object to "object"
	obj "printname" "object.printname" @hash.put		# assign the value "object.printname" to the key "printname" in the table
	objects obj @list.append							# append the obj onto the objects list
	obj													# push the object onto the stack
}

define object.printname
{
	create self											# the object is passed in
	self "name" @hash.get								# get the name of the object
	@io.puts											# print it out
	@newline											# newline
}

define player.new
{
	@object.new create player							# create an object ('inherit' an object, but it's actually a hash table) and bind it to the variable 'player'
	player "name" "player" @hash.set					# set the name to "player"
	player												# push the object onto the stack
}

define printnames										# print out the names of the objects
{
	objects @list.length create len						# store the length of the list in 'len'
	0 create i											# iterator value 'i'
	0 create obj										# the object (currently a 0 value or null)
	while { i len @lt }									# while i is less than the length of the list
	{
		objects i @list.get	set obj						# set 'obj' to the value at the 'i' index in the 'objects' list
		obj 											# place the object on the stack for the printname function to use
		obj "printname" @hash.get @call					# call the printname function designated by the string stored under the key "printname"
		# the call function takes a string as the name of the function to call (this has many uses)
		i 1 @add set i									# increment i
	}
}

@object.new												# create an object!
@player.new												# create a player object!
@printnames												# print out the names of all objects
