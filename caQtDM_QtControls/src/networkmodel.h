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
 *  Copyright (c) 2010 - 2021
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#ifndef NETWORKMODEL_H
#define NETWORKMODEL_H

#include <iostream>
#include <QApplication>
#include <QtNetwork>
#include <QCompleter>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QCheckBox>
#include <qtcontrols_global.h>

#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
#include <algorithm>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include "JSON.h"
#include "JSONValue.h"
# define QStringLiteral(str) QString::fromUtf8("" str "", sizeof(str) - 1)
#endif

class QTCON_EXPORT NetworkModel: public QStandardItemModel
{
    Q_OBJECT

public:

    void setErrorWidget(QLineEdit *w) {
        msgWidget = w;
    }

    void setCompletionWidget(QCheckBox *w) {
        autoWidget = w;
    }

    void search(const QString & text) {
        filter = text;
        QEventLoop loop;

        // when autocompletion is unchecked, then do nothing
        if(!autoWidget->isChecked()) return;

        QApplication::setOverrideCursor(Qt::WaitCursor);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        disconnect(this, SIGNAL(finished()), &loop, SLOT(quit()));
#else
        disconnect(this,  &NetworkModel::finished, &loop, &QEventLoop::quit);
#endif

        QNetworkRequest request = create_request(text);
        if(m_reply) m_reply->abort();
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        m_reply = manager.get(request);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));
        connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
#else
        connect(m_reply, &QNetworkReply::finished, this, &NetworkModel::onFinished);
        connect(this, &NetworkModel::finished, &loop, &QEventLoop::quit);
#endif

        loop.exec();

        QApplication::restoreOverrideCursor();
    }

private slots:

    void onFinished() {

        QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
        QUrl url = reply->url();
        if(reply->error() != QNetworkReply::NoError) {
            //std::cerr << "onfinished from " << qasc(url.toString()) << " " << reply->error() << " " << reply->errorString().toStdString() << "\n";
            if(reply->error() != QNetworkReply::OperationCanceledError) msgWidget->setText(reply->errorString());
        }
        if (reply->error() == QNetworkReply::NoError) {
            QVector<QString> choices;
            QByteArray response(reply->readAll());
            //std::cerr << "Data " << qasc(QString(response)) << "\n";
            QList<QString> nameList;
            msgWidget->setText("");

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            QList<QVariant> list;
            JSONValue *value = JSON::Parse(response);
            if (value == NULL) msgWidget->setText("failed to parse network reply");
            if (value) {
                //std::cerr << "count" << value->CountChildren() << "\n";
                for(size_t i=0; i<value->CountChildren(); ++i) {
                    JSONValue *key_value = value->Child(i);
                    if (key_value) {
                        QString QValue = QString::fromStdWString(std::wstring (key_value->Stringify()));
                        // get name
                        if(QValue.contains("name")) {
                            JSONObject root;
                            root = key_value->AsObject();
                            if (root.find(L"name") != root.end() && root[L"name"]->IsString()) {
                                QString nameValue = QString::fromStdWString(root[L"name"]->AsString());
                                nameList.append(nameValue);
                            }
                        };
                    }
                }
                delete value;
            }

#else
            QJsonParseError jsonError;
            QJsonDocument Json = QJsonDocument::fromJson(response, &jsonError);
            if (jsonError.error != QJsonParseError::NoError){
                 msgWidget->setText("failed to parse network reply");
                //std::cerr << qasc(jsonError.errorString()) << "\n";
            }

            QList<QVariant> list = Json.toVariant().toList();
            for(int i=0; i<list.count(); i++)  {
                QMap<QString, QVariant> map = list[i].toMap();
                nameList.append(map["name"].toString());
            }
#endif
            std::sort(nameList.begin(), nameList.end());

            for(int i=0; i<itemList.count(); i++)  {
                QStandardItem * item1 = itemList.at(i);
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
                if(item1 != (QStandardItem *) Q_NULLPTR) {
                    QVariant data = item1->data(0);
                    if(data.isValid()) data.clear();
                }
#else
                if(item1 != (QStandardItem *) Q_NULLPTR) item1->clearData();
#endif
            }
            itemList.clear();
            removeRows(0, rowCount());
            clear();

            for(int i=0; i<nameList.count(); i++)  {
                if(nameList[i].contains(filter)) {
                    QStandardItem *item = new QStandardItem(nameList[i]);
                    itemList.append(item);
                    appendRow(item);
                }
            }

            nameList.clear();
            list.clear();
        }
        reply->deleteLater();
        emit finished();
        m_reply = (QNetworkReply *) Q_NULLPTR;
    }

signals:
    void finished();

private:

    QList<QStandardItem*> itemList;
    QString filter;
    QLineEdit *msgWidget;
    QCheckBox *autoWidget;

    QNetworkRequest create_request(const QString & text) {
        //std::cerr << "create request\n";
        QString FindRecord_Srv = (QString)  qgetenv("CAQTDM_FINDRECORD_SRV");
        if (FindRecord_Srv.isEmpty()){
            FindRecord_Srv="http://iocinfo.psi.ch/api/v2/records?";// for PSI
        }
        QString FindRecord_facility = (QString)  qgetenv("CAQTDM_FINDRECORD_FACILITY");
        if (FindRecord_facility.isEmpty()){
                FindRecord_facility=".*";// for all
        }
        QString FindRecord_Limit = (QString)  qgetenv("CAQTDM_FINDRECORD_LIMIT");
        if (FindRecord_Limit.isEmpty()){
            FindRecord_Limit="200";// for PSI
        }

        QString FindRecord_Request= FindRecord_Srv + "pattern=" + text +".*&"+ "facility="+FindRecord_facility+"&limit="+FindRecord_Limit;
        QString FindRecord_Direct = (QString)  qgetenv("CAQTDM_FINDRECORD_DIRECT");
        if (!FindRecord_Direct.isEmpty()){
            FindRecord_Request=FindRecord_Direct;
        }

        QUrl suggestUrl(FindRecord_Request);
        //std::cerr << "start from " << qasc(suggestUrl.toString()) << "\n";
        return QNetworkRequest(suggestUrl);
    }

    QNetworkAccessManager manager;
    QNetworkReply *m_reply;
};

class QTCON_EXPORT NetworkCompleter: public QCompleter {

public:

    void setErrorWidget(QLineEdit *w) {
       m_model->setErrorWidget(w);
    }

    void setCompletionWidget(QCheckBox *w) {
        m_model->setCompletionWidget(w);
    }

    NetworkCompleter(QObject *parent=(QObject *) Q_NULLPTR): QCompleter(parent) {
        m_model = new NetworkModel();
        setModel(m_model);
        setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    }

    QStringList splitPath(const QString &path) const  {
        if(NetworkModel * m = qobject_cast<NetworkModel *>(model()))m->search(path);
        return QCompleter::splitPath(path);
    }

private:
    NetworkModel *m_model;

};
#endif
