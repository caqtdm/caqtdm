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
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
*/

// this function allows to let a non designable property to be visible
bool inactiveButVisible() { return false;}

#ifndef MOBILE

// this function gives you the possibility to tell the designer that a property was changed
// that way it will save the property to the ui file

bool updatePropertyEditorItem(QWidget *w, QString propertyToChange)
{
    //printf("class=%s propertyToChange=%s\n", qasc(this->objectName()), qasc(propertyToChange));
    bool retVal = false;
    int propertyIndex;
    QDesignerFormWindowInterface *formWindow = 0;
    formWindow = QDesignerFormWindowInterface::findFormWindow(w);
    if (formWindow) {
        retVal = true;
        QDesignerFormEditorInterface *formEditor = formWindow->core();
        if(formEditor) {
            QExtensionManager *extensionManager = formEditor->extensionManager();
            if(extensionManager) {
                QDesignerPropertySheetExtension *propertySheet = qt_extension<QDesignerPropertySheetExtension*>(extensionManager, w);
                propertyIndex = propertySheet->indexOf(propertyToChange);

                propertySheet->setChanged(propertyIndex, true);
            }
        }
    }
    if(!retVal)  printf("caQtDM -- internal error for class=%s propertyToChange=%s not found\n", qasc(this->objectName()), qasc(propertyToChange));
    return retVal;
}
#else
bool updatePropertyEditorItem(QWidget *w, QString propertyToChange)
{
    Q_UNUSED(w);
    Q_UNUSED(propertyToChange);
    return false;
}

#endif

