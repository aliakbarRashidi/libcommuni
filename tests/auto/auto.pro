######################################################################
# Communi
######################################################################

TEMPLATE = subdirs

# IrcCore
SUBDIRS += irc
SUBDIRS += ircconnection
SUBDIRS += irccommand
SUBDIRS += ircmessage
SUBDIRS += ircnetwork

# TODO: IrcModel
#SUBDIRS += ircbuffer
#SUBDIRS += ircbuffermodel
#SUBDIRS += ircchannel
#SUBDIRS += ircuser
#SUBDIRS += ircusermodel

# IrcUtil
SUBDIRS += irccommandparser
# TODO: SUBDIRS += irccompleter
SUBDIRS += irclagtimer
SUBDIRS += ircpalette
SUBDIRS += irctextformat
