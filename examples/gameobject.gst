# gameobject.gst -- a practical representation of game objects in Gstack

@list.new create go.objects
@list.new create go.to_be_removed
@list.new create go.to_be_added

define go.new									# create the gameobject
{
	@hash.new create go
	go "x" 0 @hash.put
	go "y" 0 @hash.put
	go "w" 0 @hash.put
	go "h" 0 @hash.put
	go "name" "default" @hash.put
	go "type" "default" @hash.put
	go "layer" 0 @hash.put
	go "collidable" true @hash.put
	go "update" "go.update" @hash.put
	go "draw" "go.draw" @hash.put
	go
}

define go.update								# default update method for go's
{
												# does nothing
}

define go.draw									# default draw method for go's
{
												# does nothing
}

define go.add									# adds a gameobject to the scene
{
	create go
	go.to_be_added go @list.append
}

define go.remove								# removes the object (passed in) from the scene
{
	create go
	go.to_be_removed go @list.append
}

define go.apply_changes							# add/remove objects from the list
{
	go.to_be_added @list.length create len
	0 create i
	
	0 create go
	
	while { i len @lt }
	{
		go.to_be_added i @list.get set go
		go.objects go @list.append
		i 1 @add set i
	}
	
	go.to_be_removed @list.length set len
	0 set i
	
	0 set go
	
	while { i len @lt }
	{
		go.to_be_removed i @list.get set go
		go.objects go @list.remove
		i 1 @add set i
	}
}

define map.create
{
}