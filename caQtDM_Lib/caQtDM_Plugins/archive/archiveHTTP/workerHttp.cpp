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
 *  Copyright (c) 2010 - 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#include <workerHttp.h>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <qwt.h>

#include "archiverCommon.h"
#include "httpretrieval.h"
#include "urlhandlerhttp.h"

WorkerHTTP::WorkerHTTP()
{
    qRegisterMetaType<indexes>("indexes");
    qRegisterMetaType<QVector<double> >("QVector<double>");
    m_httpRetrieval = (HttpRetrieval *) 0;
    m_receivedContinueAt = false;
    m_vecX.clear();
    m_vecY.clear();
}

WorkerHTTP::~WorkerHTTP()
{
}

void WorkerHTTP::workerFinish()
{
    deleteLater();
}

HttpRetrieval* WorkerHTTP::getArchive()
{
    return m_httpRetrieval;
}

void WorkerHTTP::getFromArchive(QWidget *w,
                    indexes indexNew,
                    QString index_name,
                    MessageWindow *messageWindow,
                    MutexKnobData *mutexKnobDataP,
                    QSharedPointer<HttpPerformanceData> httpPerformanceData)
{
    qDebug() << "Current Time is: " << QTime::currentTime();
    QMutexLocker locker(&m_globalMutex);
    Q_UNUSED(w);
    struct timeb now;
    bool isBinned;

    QString key = indexNew.pv;
    int nbVal = 0;

    ftime(&now);
    double endSeconds = (double) now.time + (double) now.millitm / (double) 1000;
    double startSeconds = endSeconds - indexNew.secondsPast;
    if (indexNew.nrOfBins != -1) {
        isBinned = true;
    } else {
        isBinned = false;
    }

    double timeDifference = indexNew.secondsPast;
    double nrOfBinsPerSecond = indexNew.nrOfBins / timeDifference;

    if (caCartesianPlot *w = qobject_cast<caCartesianPlot *>((QWidget *) indexNew.w)) {
        // Create a Buffer for the X axis of 5%
        double timeBuffer = timeDifference * 0.05;
        // Set the limits for the X axis of the cartesian plot
        double minXMSecs = 1000 * (startSeconds - timeBuffer);
        double maxXMSecs = 1000 * (endSeconds + timeBuffer);

        // Now, invoke the function to set the Scale unsing a lamda function,
        // we have to do this as the widget lives in another thread and this function is NOT thread safe.
        // This calls the function in the widgets thread itself, that being our main GUI thread.
        QMetaObject::invokeMethod(w, [minXMSecs, maxXMSecs, w]() {
            w->setScaleX(minXMSecs, maxXMSecs);
        });
    }

    UrlHandlerHttp *urlHandler = new UrlHandlerHttp();
    urlHandler->setUrl(index_name);
    urlHandler->setBackend(indexNew.backend);
    urlHandler->setChannelName(indexNew.pv);
    urlHandler->setBinned(isBinned);
    urlHandler->setBinCount(indexNew.nrOfBins);
    urlHandler->setEndTime(QDateTime::fromSecsSinceEpoch(endSeconds));

    // Now we collect all already stored data so we can reuse it and minimize network traffic.
    knobData kData = mutexKnobDataP->GetMutexKnobData(indexNew.indexX);
    mutexKnobDataP->DataLock(&kData);

    if (kData.edata.valueCount > 0) {
        // Update the time from which we have to request new data
        // Data is stored in miliseconds but we want seconds so convert it
        startSeconds = (reinterpret_cast<double*>(kData.edata.dataB)[kData.edata.valueCount - 1] / 1000);
    }
    mutexKnobDataP->DataUnlock(&kData);

    do {
        // If member is set, then this isn't the first iteration, so wait a bit before stressing the backend again
        if (m_receivedContinueAt) {
            QThread::sleep(1);
        }
        m_receivedContinueAt = false;

        // Set Bin Count according to initial density, so the density is consitent across multiple succeeding requests
        timeDifference = endSeconds - startSeconds;
        // The count cannot be 0 because then we would be requesting no data
        urlHandler->setBinCount(qMax(1, int(timeDifference * nrOfBinsPerSecond)));

        // Set Begin Time within loop to modify it if we get a continueAt
        urlHandler->setBeginTime(QDateTime::fromSecsSinceEpoch(startSeconds));

        // Make sure previous HttpRetrieval is deleted, however, if the previous HttpRetrieval contains an abort, dont proceed
        // This is done in case an abort was initiated after the previous HttpRetrieval returned and it therefore went unnoticed.
        bool previousHttpRetrievalAborted = false;
        if (m_httpRetrieval != Q_NULLPTR) {
            // save the pointer to a temporary variable so we don't have a memory leak
            HttpRetrieval *lastHttpRetrieval = m_httpRetrieval;
            if (m_httpRetrieval->isAborted()) {
                previousHttpRetrievalAborted = true;
            }
            // Assign a new HttpRetrieval to our publicly accessible member without deleting it so an abort would never be referencing a null pointer.
            m_httpRetrieval = new HttpRetrieval();
            // Because we saved the pointer in this temporary variable, we can now safely delete it without the risk of it being referenced.
            delete lastHttpRetrieval;
        } else {
            // Create our HttpRetrieval
            m_httpRetrieval = new HttpRetrieval();
        }

        bool readdata_ok;
        if (!previousHttpRetrievalAborted) {
            httpPerformanceData->addNewRequest(urlHandler);
            readdata_ok = m_httpRetrieval->requestUrl(urlHandler->assembleUrl(),
                                                           urlHandler->backend(),
                                                           endSeconds - startSeconds,
                                                           isBinned,
                                                           indexNew.timeAxis,
                                                           key);
            if (m_httpRetrieval->is_Redirected()) {
                QUrl url = QUrl(m_httpRetrieval->getRedirected_Url());
                // Messages in case of a redirect and set the widget to the correct location
                // with a reload of the panel this information get lost.
                // the url storage location is the dynamic property of the widget
                QString mess("ArchiveHTTP plugin -- redirect: ");
                mess.append(key);
                mess.append(" to ");
                mess.append(url.toString());
                messageWindow->postMsgEvent(QtDebugMsg, (char *) qasc(mess));
                indexNew.w->setProperty("archiverIndex", QVariant(url.toString()));
                delete m_httpRetrieval;
                m_httpRetrieval = new HttpRetrieval();
                urlHandler->setUrl(url);
                readdata_ok = m_httpRetrieval->requestUrl(urlHandler->assembleUrl(),
                                                          urlHandler->backend(),
                                                          indexNew.secondsPast,
                                                          isBinned,
                                                          indexNew.timeAxis,
                                                          key);
            }
        } else {
            readdata_ok = false;
        }

        if (readdata_ok) {
            httpPerformanceData->addNewResponse(m_httpRetrieval->requestSizeKB(), m_httpRetrieval->httpStatusCode(), m_httpRetrieval->hasContinueAt(), m_httpRetrieval->continueAt());
            if (m_httpRetrieval->getCount() > 0) {
                if (isBinned) {
                    m_vecX.clear();
                    m_vecY.clear();
                    m_vecMinY.clear();
                    m_vecMaxY.clear();
                    m_httpRetrieval->getBinnedDataAppended(m_vecX, m_vecY, m_vecMinY, m_vecMaxY);
                } else {
                    m_vecX.clear();
                    m_vecY.clear();
                    m_httpRetrieval->getDataAppended(m_vecX, m_vecY);
                }
            }
            if (m_httpRetrieval->hasContinueAt()) {
                // We don't care if it's only a couple of seconds, might as well be transmission delay
                if (endSeconds - m_httpRetrieval->continueAt().toSecsSinceEpoch() > 30) {
                    startSeconds = m_httpRetrieval->continueAt().toSecsSinceEpoch();
                    m_receivedContinueAt = true;
                }
            }

            nbVal = m_vecX.count();
            if (nbVal != m_vecY.count()) {
                // API is messing with us
                throw;
            }
        } else {
            httpPerformanceData->addNewResponse(m_httpRetrieval->requestSizeKB(), m_httpRetrieval->httpStatusCode(), m_httpRetrieval->hasContinueAt(), m_httpRetrieval->continueAt());
            if (messageWindow != (MessageWindow *) Q_NULLPTR) {
                QString mess("ArchiveHTTP plugin -- lastError: ");
                if (previousHttpRetrievalAborted) {
                    mess.append(" request was aborted ");
                } else {
                    mess.append(m_httpRetrieval->lastError());
                }
                mess.append(" for pv: ");
                mess.append(key);
                mess = QString(mess.toHtmlEscaped());
                messageWindow->postMsgEvent(QtFatalMsg, (char *) qasc(mess));
            }
            m_vecX.clear();
            m_vecY.clear();
            m_vecMinY.clear();
            m_vecMaxY.clear();
            nbVal = 0;
        }
        emit resultReady(indexNew, nbVal, m_vecX, m_vecY, m_vecMinY, m_vecMaxY, m_httpRetrieval->getBackend(), !m_receivedContinueAt);
    } while (m_receivedContinueAt);
    m_vecX.clear();
    m_vecY.clear();
    urlHandler->deleteLater();
    m_httpRetrieval->deleteLater();
}
