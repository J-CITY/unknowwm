#!/bin/sh
# automon.sh -- automatically configures multiple monitors.
#
# I use this script on Arch Linux.  This handles my personal monitor setup
# more correctly and robustly than either Ubuntu or Windows 7, however
# it is only designed to work on my laptop.
#
# On that laptop it always calls "xrandr" using the exactly the same 
# arguments.  The --auto option in xrandr is clever enough to do the rest.  
# One day this script might become more flexible.
#
# My laptop is a Lenovo G550 (Model name = 2958).
#

#OUTS=`xrandr \
#        | awk '/^[[:alnum:]]+ (dis)?connected/ {print echo $1}' \
#        | sort`

# LVDS1 is the laptop's builtin screen and is therefore different.
#OUTP="xrandr --output LVDS1 --auto"
#OLD=LVDS1

#for OUT in $OUTS
#do
        # skip the monitor we already configured.
#        [ "$OUT" == "LVDS1" ] && continue
#        OUTP="$OUTP --output ${OUT} --left-of $OLD"
        #OLD=$OUT

        # Since not every monitor can be to the (direct) left of LVDS1, I
        # wanted to use $OLD to define a nice linked list.  Sure, some of the
        # monitors don't exist, but xrandr seems to treat them as 0x0 in size,
        # which seems smart to me.  Sadly X often crashes when I try this, so
        # instead $OLD is always $LVDS1.  I suspect this will cause crashes if I
        # ever plug more than two monitors in.
#done
xrandr --output HDMI1 --right-of eDP1
#echo ${OUTP[*]}