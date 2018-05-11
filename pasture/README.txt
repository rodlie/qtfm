#####################
Build
#####################

To build qtFM from source run the following in this directory:
 
>qmake		(or maybe 'qmake-qt4' depending on your distro)
>make



#####################
Install
#####################

If you install qtFM through your distro as a package then everything should be taken care of already.

If you build qtFM from source you can simply run the binary from the build directory, or to install it
run the following:

>sudo make install


This will basically:

 - copy the qtfm binary to '/usr/bin'
 - copy 'qtfm.desktop' to '/usr/share/applications'
 - copy './images/qtfm.png' to '/usr/share/pixmaps'
 - copy translations to '/usr/share/qtfm'
 - copy docs to '/usr/share/doc/qtfm' 


#####################
Run
#####################

Command Line Parameters:

 $folder	startup folder		- qtFM will start in this folder. Starts in $HOME by default.  
 -d		daemon mode		- starts qtFM and remains running in the background.

				  
Try adding:
 
 sleep 4s && qtfm -d &

to your '.xinitrc' file to start automatically at login.



#####################
Icon Themes
#####################

qtFM will try to detect your icon theme by first asking the system, which should work fine under full
Gnome/KDE environments.  For some reason Xfce always reports 'gnome' icon theme. (See below)

If this fails it will then try to read '~/.gtkrc-2.0' file for 'gtk-icon-theme-name'.
If this fails it will then try to read '~/.config/gtk-3.0/settings.ini' file for 'gtk-fallback-icon-theme'.

If all else fails it will then look for either 'gnome' or 'oxygen' in '/usr/share/icons/*'.

You can override all of the above and force it to use your desired icon theme by adding a new key
to the [General] section of '~/.config/qtfm/qtfm.conf' called 'forceTheme'.

eg.
	forceTheme=Tango

nb. The name must match the theme folder exactly (ie. case sensitive)



#####################
Single-Click Navigation
#####################

Single-click navigation can be enabled by adding a key called 'singleClick' to the general section of the qtfm.conf 
configuration file located in '~/.config/qtfm'.

There are two modes you can enable:

 singleClick=1		single click activation of directories only
 singleClick=2		everything is single click



#####################
Real Mime Types
#####################

In 'Detail' view the 'Type' column shows the file extension by default.  Alternatively you can show the actual
mime type for the files, like 'image/png', by adding the following key to the qtfm.conf configuration file.

 realMimeTypes=1

Note this can potentially slow down browsing slightly in detail mode.

This also affects 'Custom actions', ie. with this enabled you use the mimetype instead of the suffix in the
'Filetype' column when creating custom actions.  



#####################
Custom Actions
#####################

Filetype:

 Options for the filetype are:

 $suffix	-	matches files of that type.  Multiple suffix can be separated by commas. eg 'png,jpg'.
 $folder	-	$folder is a specific folder name. Matches only that folder. eg 'Trash'
 $parent 	-	$parent is the full path to a folder. Matches everything below this, files and folders. eg. '/media'	
 folder		-	matches all folders.
 *		-	matches everything, all files and folders.

 Note: File specific custom actions are always treated like wildcards.

 ie. 	video	- will match video/mpeg, video/mp4, video/avi in realMimeType mode.
 	jp	- will match *.jpg, *.jpeg etc.


Text:

 This is the text that will appear in the menu for your custom action.  It can be any descriptive text you like.

 There is one special case:- if you set the text to 'Open' it will override the default xdg-open association for that
 filetype.  This custom action will then be used as the default for 'double-click' and 'right-click->open' commands.

 You can group custom actions together by using a ' / ' in the text.  Items with the same prefix will be grouped in
 a submenu.

 eg. for images;

	jpg,png		Open with / gqview
	jpg,png		Open with / gimp
	

Icon:

 The name of any icon from your current theme.  Double-click the entry to bring up the icon chooser dialog.


Command:

 This is the custom command you want to run.  You can also call any external shell script from the custom action.
 Use an external shell script if you want to perform complicated actions that require more than a single line command.

 The parameter substitutions that can be passed to the command are:

 %f	-	selected filenames
 %F	-	selected filenames with full path
 %n	-	current filename (ie. the file you right-clicked on)

 %iVar	-	Input variable - used to input a run-time variable contained in the custom action.
		'Var' can be any text you like and is used as the hint for the input dialog.
		 eg. '%iPassword'
			
 []	-	tick the checkbox to monitor the output of the command. qtFM will be notified and report stdout and stderr
		messages after the command has completed.


Examples of some useful custom actions:

Filetype	Text			Icon			Command
--------	----			----			-------
gz,bz2		Extract here		package-x-generic	tar xf %f
png,jpg		Edit in Gimp		applications-graphics	gimp %f
mp3,ogg		Open			audio-x-generic		xmms %f
mp3,ogg		Enqueue			audio-x-generic		xmms -e %f
folder		Term here		terminal		urxvt -cd %F
Trash		Empty trash		dialog-warning		emptyTrash.sh
/media		Unmount			drive-harddisk		sudo umount %f
*		Trash			user-trash-full		mv -b %f ~/Trash
*		Compress / gz		package-x-generic	tar czf %n.tar.gz %f
*		Compress / bz2		filesave		tar cjf %n.tar.bz2 %f
*		Compress / 7z		preferences-other	7za a -p%iPassword %n.7z %f



#####################
Translations
#####################

Many thanks to the following for contributing translations:

 - Chinese	zh	A. Lee
 - Chinese	zh_TW	Cheng-Hong Wu
 - Danish	da	Peter J
 - French	fr	Diablo150
 - German	de	quax		
 - Italian	it	zereal
 - Polish	pl	Michał
 - Russian	ru	Alexey
 - Serbian	sr	Mladen
 - Spanish	es	AntumDeluge
 - Swedish	sv	Swanson

If you would like to contribute to the translations you can grab the 'qtfm_XX.ts' template
in the source, rename it to your country code (qtfm_fr.ts) and then edit it with Qt's 'linguist'
is easiest.  Send me the completed ts file and I'll add it to the package.



#####################
Tip & Tricks
#####################

- If you 'Edit filetype' on a folder and set the handler to qtFM you can then 'right-click->Open'
  on a folder to open in a new instance.

- Middle-click folders or bookmarks to open in a new tab. Ctrl+middle-click to open in a new instance.

- Middle-click tabs to close.

- Ctrl+drag&drop will copy instead of move.

- Click anywhere in the address bar path to navigate from there. Ctrl+click or middle-click will go
  immediately to that folder (ala breadcrumbs).

- Right-click on the status bar will bring up the 'View' options.  Useful if you have hidden the main menu.

- When you copy things in qtFM it places the path(s) in the selection buffer so you can then middle-click in
  a text editor for example to 'paste as text'.

- You can also use sequential shortcuts like vi or emacs instead of the combination shortcuts (ctrl+x).
  In the shortcuts editor just seperate them with commas. eg. 'a,b,c'

- If you want the fastest possible startup time, don't have any image files (png,jpg etc) in the start folder,
  because it will have to load the thumb cache immediately. Or use daemon mode!


