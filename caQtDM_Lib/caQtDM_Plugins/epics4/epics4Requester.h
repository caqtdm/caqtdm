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
#ifndef EPICS4REQUESTER_H
#define EPICS4REQUESTER_H

#include <pv/requester.h>

#include <QObject>
#include "controlsinterface.h"


namespace epics { namespace caqtdm { namespace epics4 {

class Epics4Requester;
typedef std::tr1::shared_ptr<Epics4Requester> Epics4RequesterPtr;

class epicsShareClass Epics4Requester :
   public epics::pvData::Requester,
   public std::tr1::enable_shared_from_this<Epics4Requester>
{
    MessageWindow *messageWindow;   
public:
    POINTER_DEFINITIONS(Epics4Requester);
    Epics4Requester(MessageWindow *messageWindow)
    : messageWindow(messageWindow)
    {}
    virtual ~Epics4Requester(){}
    virtual std::string getRequesterName() { return "epics4";}
    virtual void message(std::string const & message,epics::pvData::MessageType messageType);
};

}}}

#endif  // EPICS4REQUESTER_H
