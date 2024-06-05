/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Authors:
 *    Marty Kraimer, Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch, mrkraimer@comcast.net
 */

#include "epics4Requester.h"
#include "caQtDM_Lib_global.h"

namespace epics { namespace caqtdm { namespace epics4 {

void Epics4Requester::message(std::string const & message,epics::pvData::MessageType messageType)
{
    char * mes = (char *)message.c_str();
    //do this, to prevent spurious messages when closing the application
    if(strstr(mes,"DISCONNECTED") != (char*) Q_NULLPTR) return;
    if(strstr(mes,"DESTROYED") != (char*) Q_NULLPTR) return;

    switch (messageType) {
    case epics::pvData::infoMessage:
          if(messageWindow != (MessageWindow *) Q_NULLPTR) messageWindow->postMsgEvent(QtInfoMsg, mes);
          break;
    case epics::pvData::warningMessage:
          if(messageWindow != (MessageWindow *) Q_NULLPTR) messageWindow->postMsgEvent(QtWarningMsg, mes);
          break;
    case epics::pvData::errorMessage:
          if(messageWindow != (MessageWindow *) Q_NULLPTR) messageWindow->postMsgEvent(QtCriticalMsg, mes);
          break;
    case epics::pvData::fatalErrorMessage:
          if(messageWindow != (MessageWindow *) Q_NULLPTR) messageWindow->postMsgEvent(QtFatalMsg, mes);
          break;
    }
}

}}}

