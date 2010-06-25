#include <stdio.h>
#include <stdlib.h>
#include <ming.h>

int main(void)
{
	FILE				*sound_file;
	SWFSoundStream 		sound_stream;
	SWFMovie			swf_movie;

	Ming_init();
	swf_movie = newSWFMovieWithVersion(7);
	Ming_setSWFCompression(9);
	SWFMovie_setRate(swf_movie, 12);

	sound_file = fopen("mouse_single_click.mp3", "rb");
	if (NULL == sound_file)
	{
		printf("Something went wrong when opening a mouse click sound file");
		exit(1);
	}
	sound_stream = newSWFSoundStream(sound_file);

	SWFMovie_setSoundStream(swf_movie, sound_stream);
	SWFMovie_nextFrame(swf_movie);
	SWFMovie_save(swf_movie, "mouse_single_click.swf");
	fclose(sound_file);
	exit(0);
}
