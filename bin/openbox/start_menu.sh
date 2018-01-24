#! /bin/bash
DISPLAYH=`xdpyinfo  | sed 's/^ *dimensions: *[0-9]*x\([0-9]*\).*/\1/;t;d'`
PANELH=`sed -e 's/^panel_size = [0-9]*\% \([0-9]*\).*/\1/;t;d' ~/.config/tint2/tint2rc`
MOUSEX="0"
MOUSEY=$(($DISPLAYH-$PANELH))
RESTORE=`xdotool getmouselocation 2> /dev/null | sed -e 's/x://' -e 's/y://' -e 's/ screen:.*$//'`
POSITION=`sed -e 's/^panel_position = \([a-z]*\).*/\1/;t;d' ~/.config/tint2/tint2rc`
if [ $POSITION = top ] ; then
MOUSEY=$PANELH
else
MOUSEY=$(($DISPLAYH-$PANELH))
fi
xdotool mousemove $MOUSEX $MOUSEY
xdotool key Menu
xdotool mousemove $RESTORE
