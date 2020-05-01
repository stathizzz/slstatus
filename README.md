## Stathizzz's build of slstatus

Here's my build of slstatus, implementing some unique features.

### FAQ

> What is slstatus?

This is suckless, a minimalistic statusbar for dwm! Regarding the original features check their website.

> Why another fork? 

This is not just a fork, but added functionality. After experimenting with other statusbars, I found them a little glitchy or more cpu intensive than I wanted. Hence the slstatus fork.. it's suckless all the way!!


### Display killer feature

- Double status bar
	Have you ever considered adding functionality on your statusbar but didn't go with it due to sparse space? Don't worry, you can get a double status bar. The amount of time each status bar is shown is controlled through the config file. 


### Functionality Features

New functionality is added:
- Alsa volume percentage 
	For operating systems having ALSA sound libs, the volume percentage can be given through it, which is fairly more stable. Do note though the asound library should be linked and alsa headers should be in the path
- MPD music data !
	If you use mpd for listening to music, you can get a song's data on the status bar. Do note you need to link the mpdclient library and mpd headers be in the path 


