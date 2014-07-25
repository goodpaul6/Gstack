# class.gst -- a helper file containing utilties for mimicking an object-oriented system

# creates and returns an (extensible) "object" which contains a 'type' "member"
# you can add to the hash using the other methods as necessary
define object.new
{
	@hash.new create obj
	obj "type" "object" @hash.put
	obj
}

# calls a method in the class or does nothing if the method doesn't exist
define object.call
{
	create meth
	create obj
	obj meth @hash.exists create exists

	if { exists }
	{ obj meth @hash.get @call }
}

# adds a member (or sets a member) to/in the class with the object given, name given and the value given (in that order)
define object.member 
{
	create value
	create name
	create obj

	obj name @hash.exists create exists

	if { exists }
	{ obj name value @hash.set }
	else
	{ obj name value @hash.put }
}

# gets a member in an object (given member name) or throws an error if it doesn't have said member
define object.getmem
{
	create name 
	create obj
}