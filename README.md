# dcdivx
This is art of FREE Open Source Multimedia Player Designed for the Sega Dreamcast
http://www.moosegate.com/betaboy/dcdivx/

or

http://www.DCDivX.com

FAQ Version 0.65
Updated 03-Jun-2002
by Dan "BetaBoy" Marlin
 

 Description

The DCDivX Player has it's core from the ProjectMayo project called the "Pocket DivX Player". Marc, our 'core' guru Pedro and the rest of the team have worked hard since the beginning on this incredible open source media player for the PocketPC platform.

Now with the help of LiENUS and many other contributors we have taken the CORE of the Pocket DivX Player and created a FREE open source multifunction media player for the Sega Dreamcast that is 100% GPL. A special thanx to Oliver Drobnik, who was the first individual to start the Pocket DivX Player project, William Kinfoussia for all his help and support and to Dan Potter for creating KOS.

Supported Audio/Video Formats
- Audio: MP3 and Layer I + II Audio w/ MAD MP3 decoder
- Video: DivX 3.11, DivX 4.xx, DivX 5.0 + Pro, OpenDivX, XviD, AVI

Possible Support Coming Soon...
Please do not ask when.... these are just being planned for a possible addition at a future date. This is not to say that they WILL or CAN be implemented.
- Audio: OGG
- Video: MPEG-1, VP3

 News

June 3rd - The work Continues
Marc has been working on OGG support and it should be available in the next release ;-). BTW out dcdivx.com host is down... so use www.moosegate.com/betaboy/dcdivx/  till it is fixed... thanx.

April 22nd - BETA 3 IS RELEASED
Mucho hard work by Marc with this release. Look at the change log below to see what's new. But as you will tell once you get your hands on this release, we are almost at RC-1 status ;-)

 
March 25th Update - BETA 2 IS RELEASED
OK... after some help from the DARC developers... Marc has gotten DivX 5.0 with B-Frame support working. This is also a major update because at the DCDivX Players core Marc is making the 'Pocket DivX Player' more modular. This means that audio and video codecs are going to be modular and act like plugins... to be removed and or added if needed.

Why is this being done? Well with the concerns by Marc, Pedro, the entire PDP development team and me, are for the upcoming MPEG-4 licensing terms and how it could possible effect this and other related projects. This way if we are not covered for the MPEG-4 licensing by DivXNetworks... all we have to do is remove the codec DLL and NOT have to do a rewrite of the player because it was integrated and not a plugin.

Also with this release Marc has added the ability for you to create and change the background image. So follow these rules:
- The image must be named background.jpg or backgr~1.jpg
- The image must be located in the \cd\ folder
- The image must be 640X640 (the image lives on top, 640X480 of the image)

 
HELP WANTED!!

We are looking for a few good developers and graphics people to help our cause. Remember this is a GPL'd project to give back to the DC Community. We know from just our short experience on the DC Scene that there are some really talented people out there that could contribute to this project. So if you are interested please contact Marc to see what needs to be worked on or on how you can help out.
 
NOT A DEVELOPER? Are you good with Photoshop and want to show off those skills? How about a shot at making some...
 
- CD Cover Artwork/Labels
- Splash Screens: Intro splash, Please stand by, Loading..., etc
- Background instrumental Music (techno, hip-hop) Must be Original and Copyright free!
- Logos
- Icons
- Intro Videos
- GUI ideas
 
All those type of submissions can be sent HERE. *
 

 DCDivX Player Files

Beta 3

 Released March 22nd 2002

What New with Beta 3?

-  New Loading Video screen
-  Fixed filename overlapping
-  Changed file size column to be in KB to save space
-  Fixed directory navigation sensitivity
-  Disabled FFWD for MP3 files
-  Added 30 second inactivity auto start
-  Modified Start button to play from selected file and then repeat through all files.
-  Added Shuffle/re-Shuffle on right trigger, and disable Shuffle on left trigger.
-  Re-arrange project source to be a little more modular and readable.
 

File Name Description

File Size	
Download

Bootable Nero image

265 kb	Download
Bootable DiskJuggler image

257 kb	Download
MAKE YOUR OWN IMAGE (DCDivX PACK)

251 kb	Download
Original - Unbootable GPL source code - 608 kb

 
Submit a DCDivX Player File or Link HERE *
 

 DCDivX Player FAQ

 
-  DECODING / PLAYBACK
 
Q: Why does my video and audio skip and or freeze?
A: Again play with the encoding settings... try reducing the video bitrate, decimating by 2 or reduces the audio bitrate down to 22khz mono to avoid these skips. If you have found a magical setting that works GREAT share you findings by posting to the PDP forum
 
Q: What audio and video formats does it support?
A: For audio it supports MP3's encoded under 128k and video it supports DivX 4.xx, OpenDivX and XviD encoded videos. There are possible plans to add OGG, MPEG-1 and a  chance to add DivX 3.11 support as well.
 
Q: Does the DCDivX Player support Disc Swapping?
A: Yes, after you have started the DCDivX Player just open the Dreamcast CD tray and put in a new CD with your videos and MP3's.  It will then scan the CD for all the valid file types available files for playback and will then list them for you to choose.

Q: Why does my Dreamcast reboot when I try and play a A/V file?
A: You must have a filename in the 10.3 filename format. For instance, I file name  "vp3_sux.avi" will play, but trying to play "vp4costsalot_doh.avi" and it resets the DreamCast. So for now till it's fixed keep the file names simple 8 characters or less with no spaces or dashes.

Q: Why does it take so long to load a video after I select it?
A: We have implement a buffering system within the DCDivX Player for better playback (less skipping). As a matter of fact in future version the delay may even be longer for videos larger than 10MB. We are going to try in later release implement a "Loading.." Splash screen with it showing a buffering percentage if possible.

Q: Will the DCDivX Player EVER support resolutions higher than 496x496?
A: Maybe... but not in the near future. The current decoder CORE cannot handle resolutions larger than 496x496. But a bigger issue is that anything higher then that the Dreamcast hardware can not handle (A/V skips). So do not look for any larger resolution anytime soon.

Q: What buttons control what player functions?
A: Here are the current button functions:
    Pause/Play (A)  Stop (Y)   Next (B)   Previous (X)

Q: What does the bootcd image contain?
A: It contains all the files (codecs, DCDivX player) needed to playback a supported and properly formatted audio or video file.

Q: How does the bootcd work?
A: After burning an image to CDR all you do is insert it into your Dreamcast. It will then boot up with it and show the GUI. Then all you do is SWITCH the current CD with another CD that has the properly formatted audio or video files you want to play.

 
-  ENCODING

Q: How do I encode videos for the DCDivX Player?
A: You can follow the guide I have created for the Pocket DivX Player, here.

Q: Is there going to be a new encoding guide?
A: Yes, I have already begun work on this. Once the standards for both audio and video have been finalized for the DCDivX Player I will release it here and have you make comments on the ProjectMayo forums for any changes that may be needed.
  
Q: What settings do I use to encode with?
A:  For video try anything less than 700kb (i recommend 500 then work your way up), 320X240 or 320x 176 (Marc has also added support for 496X496 but it is suggested that you decimate the framerate by 2). For audio try 22khz mono or 44khz mono MP3. The key is... since it is an alpha release try a little experimentation with different settings to see what works best.

Q: When using virtualdub I get an audio error. What can I do?
A:  The problem is virtualdub sometimes does not like bitrates like 16k. I suggest that you try using the Lame MP3 encoder to separately encode audio.


-  CD BURNING

Q: I am burning a DiscJuggler image and it does not self boot or I get an error when burning it. What going on?
A: We have found that some DJ images do not work with some versions of the DJ. For example the current DJ image for Alpha 2 does not work with DJ 3.5 and errors out sometimes. So the only thing that we can suggest is that you wait for a compatible image to be released or try another software image.
 
Q: How do I make a bootable CD?
A: Well you could always wait for someone to make one ;-) ... but if you are impatient there are two tutorials based off of each other that can help you with the software and the steps needed to make one.
- http://www.hangar-eleven.de/en/devdc-selfboot.html 
-  http://mc.pp.se/dc/cdr.html
 
Q: How can I make an image from the original source files?
A: Dogbert posted a little Nero image tutorial. It's not perfect but works.. Also the process is a long but the results are worth it for a DCDivX boot CD.
 
1. Download the 'Original' source files
2. Use Turrican2k's elf2bin program here to convert the divx.elf --> divx.bin
3. Use the Binary Scrambler from here to convert divx.bin --> a bootable 1ST_READ.BIN
4. Use the IP.BIN creator from here to create the IP.BIN
5. Next put the newly created 1ST_READ.BIN and the IP.BIN together in a folder with a small video or MP3 file to load at boot up
6. Use BurnerO's dir2boot from here to create a Nero image of the whole thing
7. Burn the CD
 
Q: What software do I use to burn with?
A: The truth is there may be several different image packages for you to burn. The most used programs for this are Nero and DiscJuggler. You can find them at the below links.
Nero - http://www.ahead.de 
DiscJuggler -  http://www.padus.com
 
Q: Why does my HP Burner not burn the DJ and Nero image properly?
A:  HP burners can not burn data CDs with short tracks below 300 sectors. This may be addressed in later releases.
 
Q: When burning the image what settings do I use?
A: For both the Nero and the DiscJuggler images use ISO Layer 1, Mode 1.
 
 
-  CONTRIBUTING

Q: How can I contribute to this project?
A: You can post to the PDP forum or contact Marc to see what work needs to be done.
 
Q: How can I access the CVS server?
A: You can access it either via the web interface at http://cvs.projectmayo.com or through a local CVS client. Use the following info for anonymous login.
cvs -d:pserver:anonymous@cvs.projectmayo.com:/home/cvsroot login

Q: How can I make a suggestion?
A: Again, post any of your thoughts on additional feature or options to the PDP forum

 
-  MISCELLANEOUS

Q: Can I customize the DCDivX GUI?
A: Yes, you may do by downloading this picture as a template. Once you are done make sure you include this picture in the root of the DCDivX CD you make.

Q: What license is this released under?
A: The DCDivX Player is released under the GPL license.
  
Q: I am getting errors. Who can I complain too?
A: We would ask that you hold off complaining till we release a 1.0 version ;-) But as above you can post any issues to the PDP forum
 
Q: Why is the FAQ hosted here and the project not on ProjectMayo?
A: Currently the DCDivX Player is homeless.. well almost. Though it's core development  code is currently going to be mirrored by the Pocket DivX Player Project, and you can always find the latest version on the ProjectMayo CVS Server Marc's site or here. The project has yet to determine it's true home. You can always contact me BetaBoy or Marc if you have a proposition.
  

 
 
 * All Submissions are given with the understanding that they are free of any intellectual properties and that the act of submitting the file(s) transfers ownership to the DCDivX Player Project.
 
Copyright © 2002 and CoreCodec Inc.
