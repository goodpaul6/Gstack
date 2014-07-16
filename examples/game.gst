# game.gst -- guess a number between 1 and 100 

0 create input_value
0 create guess_value

define isdigit
{
	create ch
	if { ch '0' @gte ch '9' @lte @and }
		true
	else
		false
}

define read
{
	@strbuf.new create buf
	@getchar create ch
	while { ch '\n' @equals @not }
	{
		if { ch @isdigit }
		{
			while { ch @isdigit }
			{
				buf ch @strbuf.char
				@getchar set ch
			}
			
			buf @strbuf.tostring @tonumber set input_value
			break
		}
		else 
		{
			while { ch @isdigit @not }
			{
				@getchar set ch
			}
		}
	}
}

define main
{
	"guess a number between 1 and 100:\n" @io.puts
	@random.seed
	100 @random.float @mul @floor 1 @add
	set guess_value
	
	while { true }
	{
		@read
		if { input_value guess_value @equals } 
			break
		else
		{	
			if { input_value guess_value @lt }
				"too low!\n" 
			if { input_value guess_value @gt }
				"too high!\n"
			@io.puts
			
			"try again\n" @io.puts
		}
	}
	
	"you got it!\n" @io.puts
}

@main
