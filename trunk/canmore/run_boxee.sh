#!/bin/sh

# collect crash logs
if [ -f /opt/local/bin/collect_logs ]; then
    /opt/local/bin/collect_logs
fi

export LD_LIBRARY_PATH=/opt/local/lib:/opt/local/qt-4.7/lib:/opt/local/qt/lib:/lib:/usr/lib:/usr/local/lib:/opt/boxee/system/players/flashplayer:/opt/boxee/system/players/dvdplayer
export MOZ_PLUGIN_PATH=/opt/boxee/system/players/flashplayer/plugins
export WEBKIT_PLUGIN_PATH=/opt/boxee/system/players/flashplayer/plugins
export QWS_DISPLAY=intelce:bgcolor=#00000000
export QWS_KEYBOARD=intelceir:repeatRate=500                                    
export QT_QWS_FONTDIR=/opt/local/qt/lib/fonts
export QT_PLUGIN_PATH=/opt/local/qt/plugins                                     
export PYTHON_SANDBOX=1
export FLASH_CIRC_FILE_MAX_SIZE_MB=75

cd /opt/boxee
LD_PRELOAD=system/bxwrapper.so ./Boxee
