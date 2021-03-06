#include <Ar/Udp/UdpService.h>
#include <Ar/Middleware/ActiveThread.h>
#include <Ar/Middleware/Utils.h>
#include <Ar/Messages.h>
#include <Ar/Udp/UdpTrx.h>

namespace Ar { namespace Udp
{
    unsigned UdpService::nextRouteId = 0;

    UdpService::UdpService()
        : ActiveObject("UDP_SERVICE")
        , _at( Ar::Middleware::safeNew<Ar::Middleware::ActiveThread>() )

    {
        attachTo(_at);
    }

    UdpService::~UdpService()
    {
        for(auto route : _routes)
        {
            safeDel(route.second);
        }

        Ar::Middleware::safeDel(_at);
    }

    void UdpService::initialize()
    {
        registerForMessages();
    }

    void UdpService::run()
    {
        _at->start("UdpService");
    }

    void UdpService::sendUdpPacket(UdpPacketMessage *message)
    {
        log().debug("sendUdpPacket() received");
        _routes[message->routeId]->sendUdpPacket(message);
    }

    void UdpService::registerForMessages()
    {
        at()->registerReceiverForMessage(this, &UdpService::handleAddUdpTrxRouteMessage);
        at()->registerReceiverForMessage(this, &UdpService::sendUdpPacket);
    }

    void UdpService::handleAddUdpTrxRouteMessage(AddUdpTrxRouteMessage *message)
    {
        log().debug("handleAddUdpTrxRouteMessage() received");
        auto response = message->createResponse();

        auto id = addUdpTrxRoute(message);
        if(id!=0)
        {
            response->result = 1;
            response->routeId = id;
        }
        else
        {
            response->result = 0;
            response->routeId = 0;
        }
        at()->sendTo(message->from, response);
    }

    unsigned UdpService::addUdpTrxRoute(AddUdpTrxRouteMessage *message)
    {
        auto route = new Route(this, message->callbackAtName);
        auto routeInitializedId = route->initialize(message->config);
        if(! routeInitializedId)
        {
            delete route;
        }
        return routeInitializedId;
    }

    unsigned UdpService::add(Route *route)
    {
        auto id = ++nextRouteId;
        _routes[id] = route;

        return id;
    }

    UdpService::Route::Route(UdpService *service, const std::string &callbackAtName)
        : _service(service)
        , _routeId(0)
        , _callbackAtName(callbackAtName)
        , _trx(safeNew<UdpTrx>())
    {

    }

    UdpService::Route::~Route()
    {
        safeDel(_trx);
    }

    unsigned UdpService::Route::initialize(const UdpTrxConfig &config)
    {
        if(_trx->initialize(config, [&](auto data, unsigned length)
        {
            this->handleUdpPacket(data, length); /// @todo this lambda will be executed in UdpRx/async_receive_from thread, not in UdpSerivce::Route
                                                 /// @note here message to UdpService::Route needs to be sent
        }))
        {
            _routeId = _service->add(this);
            _trx->run();

            return _routeId;
        }

        return 0;
    }

    void UdpService::Route::handleUdpPacket(DataPtr &data, unsigned length)
    {
        log().debug("Route %u received data(%u)", _routeId, length);
        sendUdpPacketInternally(data, length);
    }

    void UdpService::Route::sendUdpPacketInternally(DataPtr &data, unsigned length)
    {
        auto message = safeNew<UdpPacketMessage>();
        message->routeId = _routeId;
        message->data = data;
        message->length = length;
        _service->at()->sendTo(_callbackAtName, message);
    }

    void UdpService::Route::sendUdpPacket(UdpPacketMessage *message)
    {
        log().debug("Route::sendUdpPacket() received");
        _trx->send(message->data, message->length);
    }
} }
