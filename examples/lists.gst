# lists.gst -- testing lists

@list.new create map
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append
map [ 1 1 1 1 1 1 1 1 1 1 ] @list.newblock @list.append

define map.debug
{
	create map
	0 create y
	while { y map @list.length @lt }
	{
		0 create x
		while { x map y @list.get @list.length @lt }
		{
			map y @list.get x @list.get @io.putn @newline
			x 1 @add set x
		}
		y 1 @add set y
	}
}

map @map.debug 