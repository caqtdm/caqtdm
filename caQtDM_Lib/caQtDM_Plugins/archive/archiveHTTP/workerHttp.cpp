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

#include "archiverGeneral.h"
#include "httpretrieval.h"
#include "urlhandlerhttp.h"

WorkerHTTP::WorkerHTTP()
{
    qRegisterMetaType<indexes>("indexes");
    qRegisterMetaType<QVector<double> >("QVector<double>");
    m_httpRetrieval = Q_NULLPTR;
    m_receivedContinueAt = false;
    m_vecX.clear();
    m_vecY.clear();
    m_isActive = true;
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
    QMutexLocker locker(&m_globalMutex);
    Q_UNUSED(w);
    struct timeb now;
    bool isBinned;

    QString key = indexNew.pv;
    int nbVal = 0;

    ftime(&now);
    double endSeconds = (double) now.time + (double) now.millitm / (double) 1000;
    double startSeconds = endSeconds - indexNew.secondsPast;
    if (indexNew.nrOfBins > 0) {
        isBinned = true;
    } else {
        isBinned = false;
    }

    // Calculate the number of bins per second to be able to maintain the initial bin density if we receive a continueAt
    double timeDifference = indexNew.secondsPast;
    double nrOfBinsPerSecond = indexNew.nrOfBins / timeDifference;

    if (caCartesianPlot *w = qobject_cast<caCartesianPlot *>((QWidget *) indexNew.w)) {
        // Create a Buffer for the X axis of 5%
        double timeBuffer = timeDifference * 0.05;
        // Set the limits for the X axis of the cartesian plot
        double minXMSecs = 1000 * (startSeconds - timeBuffer);
        double maxXMSecs = 1000 * (endSeconds + timeBuffer);

        // Now, invoke the function to set the Scale unsing a lamda function,
        // We have to do this as the widget lives in another thread and this function is NOT thread safe.
        // This calls the function in the widgets thread itself, that being our main GUI thread.
        QMetaObject::invokeMethod(w, [minXMSecs, maxXMSecs, w]() {
            w->setScaleX(minXMSecs, maxXMSecs);
        });
    }

    // Initialize urlhandler with parameters
    UrlHandlerHttp *urlHandler = new UrlHandlerHttp();

    // Check whether the provided api name is only the hostname or if it is the whole url and set the according parameters.
    // It could be the whole url if we were redirected previously. We check this by looking if it starts with "http",
    // because then it cannot simply be the hostname.
    if (index_name.startsWith("http")) {
        // It's a full url, so parse it as such
        urlHandler->setUrl(QUrl(index_name));
    } else {
        // It's only the domain name
        urlHandler->setDomainName(index_name);
    }

    // Set other parameters
    urlHandler->setUsesHttps(false);
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
            if (m_retryAfter != 0) {
                // Wait as long as the server requested
                QThread::sleep(m_retryAfter);
            } else {
                // Just wait for a second
                QThread::sleep(1);
            }
        }
        m_retryAfter = 0;
        m_receivedContinueAt = false;

        // Set bin count according to initial density, so the density is consitent across multiple succeeding requests
        timeDifference = endSeconds - startSeconds;
        urlHandler->setBinCount(int(timeDifference * nrOfBinsPerSecond));
        // When the bin count is less than one, then not enough time has passed to make another request
        bool binCountLessThanOne = urlHandler->binCount() < 1;

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
        // If the previous retrieval aborted, don't even request.
        // If the bin Count is less than one and we have binned data, don't even request.
        if (!previousHttpRetrievalAborted && !(binCountLessThanOne && isBinned)) {
            httpPerformanceData->addNewRequest(urlHandler);
            readdata_ok = m_httpRetrieval->requestUrl(urlHandler->assembleUrl(),
                                                           urlHandler->backend(),
                                                           timeDifference,
                                                           isBinned,
                                                           indexNew.timeAxis,
                                                           key);
            if (m_httpRetrieval->is_Redirected()) {
                QUrl url = QUrl(m_httpRetrieval->getRedirected_Url());
                // Messages in case of a redirect and set the widget to the correct location
                // with a reload of the panel this information get lost.
                if (messageWindow != (MessageWindow *) Q_NULLPTR) {
                    QString mess("ArchiveHTTP plugin -- redirect: ");
                    mess.append(key);
                    mess.append(" to ");
                    mess.append(url.toString());
                    messageWindow->postMsgEvent(QtDebugMsg, (char *) qasc(mess));
                }

                // Set a dynamic property containing the new url so the next update doesnt run into the same error again
                indexNew.w->setProperty("archiverIndex", QVariant(url.toString()));
                // Update the url for the next request
                urlHandler->setUrl(url);
                // Make sure we do another request
                m_receivedContinueAt = true;
                readdata_ok = false;
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
                    // Get the data
                    m_httpRetrieval->getBinnedDataAppended(m_vecX, m_vecY, m_vecMinY, m_vecMaxY);
                    // Because we have binned data the latest point is faulty as it does not contain as much data as the others
                    // Due to this, there might be unproportional spikes, so remove the last point if there are more than one.
                    // Also only do this if there is no continueAt, because if there is the last received point isn't actually the latest one and therefore not affected by this problem.
                    if (!m_httpRetrieval->hasContinueAt() && m_httpRetrieval->getCount() > 1) {
                        m_vecX.removeLast();
                        m_vecY.removeLast();
                        m_vecMinY.removeLast();
                        m_vecMaxY.removeLast();
                    }
                } else {
                    m_vecX.clear();
                    m_vecY.clear();
                    // Get the data
                    m_httpRetrieval->getDataAppended(m_vecX, m_vecY);
                }
            }
            if (m_httpRetrieval->hasContinueAt()) {
                // We don't care if it's only a couple of seconds, might as well be transmission delay
                if (endSeconds - m_httpRetrieval->continueAt().toSecsSinceEpoch() > 30) {
                    startSeconds = m_httpRetrieval->continueAt().toSecsSinceEpoch();
                    m_receivedContinueAt = true;

                    if (m_httpRetrieval->retryAfter() != 0) {
                        // If the API told us how long to wait, follow that instruction
                        m_retryAfter = m_httpRetrieval->retryAfter();
                        if (m_retryAfter > 5) {
                            // We are not stalling for more than 5 seconds, and as long as we get a valid response, the server isn't fully at capacity, yet, so request again in 5 secs. :)
                            m_retryAfter = 5;
                        }
                    }
                }
            }

            nbVal = m_vecX.count();
            if (nbVal != m_vecY.count()) {
                // API is messing with us
                throw;
            }
        } else {
            httpPerformanceData->addNewResponse(m_httpRetrieval->requestSizeKB(), m_httpRetrieval->httpStatusCode(), m_httpRetrieval->hasContinueAt(), m_httpRetrieval->continueAt());
            // If we intentionally did not send out a request because the bin count was too low, don't generate an error
            // If the request was redirected, an error has already been displayed but the request is not aborted, so don't generate an error
            if (!(binCountLessThanOne && isBinned) && !m_httpRetrieval->is_Redirected()) {
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
            }

            m_vecX.clear();
            m_vecY.clear();
            m_vecMinY.clear();
            m_vecMaxY.clear();
            nbVal = 0;

            // If the server is temporarily at capacity, try again, but only if the request wasn't aborted.
            if (!previousHttpRetrievalAborted) {
                if (m_httpRetrieval->retryAfter() != 0) {
                    qDebug() << m_httpRetrieval->retryAfter();
                    // Set this to indicate we are trying again.
                    // A retry after is basically a continueAt, just that the server couldn't give ANY data, instead of not all.
                    m_receivedContinueAt = true;
                    m_retryAfter = m_httpRetrieval->retryAfter();
                    if (m_httpRetrieval->retryAfter() > 5) {
                        // Nope, we are not waiting this long... Just finish so it is requested again on the next update iteration, being at least 10 secs from now
                        m_receivedContinueAt = false;
                    }
                }
            }
        }
        // Check if the thread was marked as inactive to cancel everything if needed.
        if (!m_isActive) {
            // Set receivedContinueAt to false because we don't want any more updates.
            m_receivedContinueAt = false;
            // set data count to 0 to clarify this data is not needed.
            nbVal = 0;
        }
        qDebug() << "emitting in thread" << QThread::currentThread();
        emit resultReady(indexNew, nbVal, m_vecX, m_vecY, m_vecMinY, m_vecMaxY, m_httpRetrieval->getBackend(), !m_receivedContinueAt);
    } while (m_receivedContinueAt);
    m_vecX.clear();
    m_vecY.clear();
    urlHandler->deleteLater();
    m_httpRetrieval->deleteLater();
}

bool WorkerHTTP::isActive() const
{
    return m_isActive;
}

void WorkerHTTP::setIsActive(bool newIsActive)
{
    m_isActive = newIsActive;
}
