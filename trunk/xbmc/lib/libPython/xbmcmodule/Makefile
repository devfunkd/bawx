INCLUDES=-I. -I../../../ -I../../../linux -I../../../.. -I../../../../guilib -I../../../utils -I../../../cores -I../../libBoxee -I../../../cores -I../../../../guilib/tinyXML -I../../../cores/dvdplayer/Codecs -I../../../cores/dvdplayer/Codecs/ffmpeg -I../../../lib/libjson/include
CXXFLAGS += -D __STDC_FORMAT_MACROS  -D__STDC_CONSTANT_MACROS

SRCS=action.cpp controlbutton.cpp controlcheckmark.cpp control.cpp controlfadelabel.cpp controlimage.cpp controllabel.cpp controllist.cpp controlprogress.cpp controlspin.cpp controltextbox.cpp dialog.cpp GUIPythonWindow.cpp GUIPythonWindowDialog.cpp GUIPythonWindowXML.cpp GUIPythonWindowXMLDialog.cpp infotagmusic.cpp infotagvideo.cpp keyboard.cpp listitem.cpp player.cpp pyplaylist.cpp PythonPlayer.cpp pyutil.cpp window.cpp winxml.cpp winxmldialog.cpp xbmcguimodule.cpp xbmcmodule.cpp controlgroup.cpp xbmcplugin.cpp controlradiobutton.cpp language.cpp PythonSettings.cpp

LIB=xbmcmodule.a

include ../../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
