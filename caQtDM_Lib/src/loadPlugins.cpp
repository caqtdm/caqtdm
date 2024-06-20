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

#include <QDebug>
#include "loadPlugins.h"
#include "qtdefinitions.h"
#include "pathdefinitions.h"
#include "caQtDM_Lib_global.h"

loadPlugins::loadPlugins()
{
}

void loadPlugins::printPlugins(const QMap<QString,ControlsInterface*> interfaces)
{
    if(!interfaces.isEmpty()) {
        QMapIterator<QString, ControlsInterface *> i(interfaces);
        while (i.hasNext()) {
            i.next();
            qDebug() << "Info: plugin" <<  i.key() << "loaded";
        }
    }
}

bool loadPlugins::loadAll(QMap<QString, ControlsInterface*> &interfaces, MutexKnobData *mutexKnobData, MessageWindow *messageWindow,
                          QMap<QString, QString> options)
{
    int nbInterfaces = 0;
#ifndef MOBILE
    //qDebug() << "load dynamic plugins";
    char asc[MAX_STRING_LENGTH];
    QList<QString> allPaths;

    // get the controlsystem plugins from QT_PLUGIN_PATH
    QString pluginPath = (QString)  qgetenv("QT_PLUGIN_PATH");
    QStringList paths = pluginPath.split(pathSeparator);

    for(int i=0; i< paths.count(); i++) {
        QString path = paths[i] + "/controlsystems";
        allPaths.append(path);
     }

    // alternative path
#if defined(__OSX__) || defined(__APPLE__)
    QString alternativePath(qApp->applicationDirPath());
    alternativePath.append("/../PlugIns/controlsystems");
    allPaths.append(alternativePath);
#else
    QString alternativePath(qApp->applicationDirPath());
    alternativePath.append("/controlsystems");
    allPaths.append(alternativePath);
    allPaths.append(QLibraryInfo::location(QLibraryInfo::PluginsPath).append("/controlsystems"));
#endif

    for (int i = 0; i < allPaths.size(); ++i) {
        QString path = allPaths.at(i);
        QDir pluginsDir(path);
        qDebug() << "caQtDM -- Controlsystem plugins: attempt to load from" << pluginsDir.absolutePath();

        QStringList filesInPluginFolder = pluginsDir.entryList(QDir::Files);
        QStringList possiblePlugins;
#ifdef linux
        // Only check .so files, otherwise we get unneccessary errors
        for (int i = 0; i < filesInPluginFolder.size(); ++i) {
            const QString& filename = filesInPluginFolder.at(i);
            if (filename.endsWith(".so", Qt::CaseInsensitive)) {
                possiblePlugins.append(filename);
            }
        }
#elif defined(_WIN32)
        // Only check windows .dll files, otherwise we get unnecessary errors
        for (int i = 0; i < filesInPluginFolder.size(); ++i) {
            const QString& filename = filesInPluginFolder.at(i);
            if (filename.endsWith(".dll", Qt::CaseInsensitive)) {
                possiblePlugins.append(filename);
            }
        }
#elif defined(__OSX__)|| defined(__APPLE__)
        // Only check .dylib files, otherwise we get unneccessary errors
        for (int i = 0; i < filesInPluginFolder.size(); ++i) {
            const QString& filename = filesInPluginFolder.at(i);
            if (filename.endsWith(".dylib", Qt::CaseInsensitive)) {
                possiblePlugins.append(filename);
            }
        }
#endif


        // seems our plugins are located here
        if(possiblePlugins.length() > 0) {
            QString currentPath = pluginsDir.absolutePath();
            snprintf(asc, MAX_STRING_LENGTH, "Controlsystem plugins: attempt to load from %s", qasc(currentPath));
            if(messageWindow != (MessageWindow *) Q_NULLPTR) messageWindow->postMsgEvent(QtInfoMsg, asc);
            foreach (QString fileName, possiblePlugins) {
                //qDebug() << "load " << pluginsDir.absoluteFilePath(fileName);
                QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
                QObject *plugin = pluginLoader.instance();
                if (plugin) {
                    controlsInterface = qobject_cast<ControlsInterface *>(plugin);
                    if (controlsInterface) {
                        controlsInterface->initCommunicationLayer(mutexKnobData, messageWindow, options);
                        interfaces.insert(controlsInterface->pluginName(), controlsInterface);
                        nbInterfaces++;
                    }
                } else {
                    snprintf(asc, MAX_STRING_LENGTH, "%s", qasc(pluginLoader.errorString()));
                    if(messageWindow != (MessageWindow *) Q_NULLPTR) messageWindow->postMsgEvent(QtWarningMsg, asc);
                }
            }
            break;
        }
    }
#else
    //qDebug() << "load static plugins";
    foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
        if (plugin) {
            controlsInterface = qobject_cast<ControlsInterface *>(plugin);
            if (controlsInterface) {
                controlsInterface->initCommunicationLayer(mutexKnobData, messageWindow, options);
                interfaces.insert(controlsInterface->pluginName(), controlsInterface);
                nbInterfaces++;
            }
        }
    }
#endif

    if(nbInterfaces== 0) return false; else return true;
}

