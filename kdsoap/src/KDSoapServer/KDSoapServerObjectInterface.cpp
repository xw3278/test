#include "KDSoapServerObjectInterface.h"
#include "KDSoapServerSocket_p.h"
#include <QDebug>

class KDSoapServerObjectInterface::Private
{
public:
    Private() :
        m_delayedResponse(false),
        m_serverSocket(0)
    {
    }

    KDSoapHeaders m_requestHeaders;
    KDSoapHeaders m_responseHeaders;
    QString m_faultCode;
    QString m_faultString;
    QString m_faultActor;
    QString m_detail;
    QByteArray m_soapAction;
    bool m_delayedResponse;
    KDSoapServerSocket* m_serverSocket;
};

KDSoapServerObjectInterface::KDSoapServerObjectInterface()
    : d(new Private)
{
}

KDSoapServerObjectInterface::~KDSoapServerObjectInterface()
{
    delete d;
}

void KDSoapServerObjectInterface::processRequest(const KDSoapMessage &request, KDSoapMessage& response, const QByteArray& soapAction)
{
    const QString method = request.name();
    qDebug() << "Slot not found:" << method << "[soapAction =" << soapAction << "]" /* << "in" << metaObject()->className()*/;
    response.setFault(true);
    response.addArgument(QString::fromLatin1("faultcode"), QString::fromLatin1("Server.MethodNotFound"));
    response.addArgument(QString::fromLatin1("faultstring"), QString::fromLatin1("%1 not found").arg(method));
}

void KDSoapServerObjectInterface::setFault(const QString &faultCode, const QString &faultString, const QString &faultActor, const QString &detail)
{
    Q_ASSERT(!faultCode.isEmpty());
    d->m_faultCode = faultCode;
    d->m_faultString = faultString;
    d->m_faultActor = faultActor;
    d->m_detail = detail;
}

void KDSoapServerObjectInterface::storeFaultAttributes(KDSoapMessage& message) const
{
    message.addArgument(QString::fromLatin1("faultcode"), d->m_faultCode);
    message.addArgument(QString::fromLatin1("faultstring"), d->m_faultString);
    message.addArgument(QString::fromLatin1("faultactor"), d->m_faultActor);
    message.addArgument(QString::fromLatin1("detail"), d->m_detail);
}

bool KDSoapServerObjectInterface::hasFault() const
{
    return !d->m_faultCode.isEmpty();
}

KDSoapHeaders KDSoapServerObjectInterface::requestHeaders() const
{
    return d->m_requestHeaders;
}

void KDSoapServerObjectInterface::setRequestHeaders(const KDSoapHeaders &headers, const QByteArray& soapAction)
{
    d->m_requestHeaders = headers;
    d->m_soapAction = soapAction;
    // Prepare for a new request to be handled
    d->m_faultCode.clear();
    d->m_responseHeaders.clear();
}

void KDSoapServerObjectInterface::setResponseHeaders(const KDSoapHeaders &headers)
{
    d->m_responseHeaders = headers;
}

KDSoapHeaders KDSoapServerObjectInterface::responseHeaders() const
{
    return d->m_responseHeaders;
}

QByteArray KDSoapServerObjectInterface::soapAction() const
{
    return d->m_soapAction;
}

KDSoapDelayedResponseHandle KDSoapServerObjectInterface::prepareDelayedResponse()
{
    d->m_delayedResponse = true;
    return KDSoapDelayedResponseHandle(d->m_serverSocket);
}

bool KDSoapServerObjectInterface::isDelayedResponse() const
{
    return d->m_delayedResponse;
}

void KDSoapServerObjectInterface::setServerSocket(KDSoapServerSocket *serverSocket)
{
    d->m_serverSocket = serverSocket;
}

void KDSoapServerObjectInterface::sendDelayedResponse(const KDSoapDelayedResponseHandle& responseHandle, const KDSoapMessage &response)
{
    responseHandle.serverSocket()->sendDelayedReply(this, response);
}