#ifndef __GICAME__RPCSERVER_H__
#define __GICAME__RPCSERVER_H__


#include <unordered_map>
#include <functional>
#include <vector>
#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "../dataTransfer/BinaryInstanceExchanger.h"
#include "./RpcCommon.h"
#include "../dataSerialization/BinarySerializer.h"


namespace Gicame {

    class GICAME_API RpcServer {

        MOVABLE_BUT_NOT_COPYABLE;

    public:  // Public types
        enum class InvalidRequestReason {
            RER_INTEGRITY_CHECK_FAILED, INVALID_FUNCTION_ID, BAD_PARAMETERS
        };
        using InvalidRequestEventHandler = std::function<bool(const RpcExecutionRequest&, const InvalidRequestReason)>;

    private:  // Private types
        /**
         * Describe an RPC Function stored in the RpcServer
         */
        struct RpcFunctionDescriptor {
            RpcFunction rpcFunction;
            FunctionId functionId;
            inline RpcFunctionDescriptor(const RpcFunction rpcFunction, const FunctionId functionId) :
                rpcFunction(rpcFunction), functionId(functionId) {}
        };

    private:  // Private data
        IDataExchanger* dataExchanger;
        BinarySerializer serializer;
#ifdef MSVC
#pragma warning(push)
#pragma warning(disable:4251)
#endif
        std::unordered_map<FunctionId, RpcFunctionDescriptor> funStore;
        InvalidRequestEventHandler invalidRequestEventHandler;
#ifdef MSVC
#pragma warning(pop)
#endif

    public:  // Public methods
        RpcServer(IDataExchanger* dataExchanger, const BinarySerializer& serializer = BinarySerializer());

        /**
         * Add or replace an RpcFunction to the RpcServer
         */
        void registerRpcFunction(RpcFunction rpcFunction, const FunctionId functionId);

        /**
         * Add or replace a function to the RpcServer
         */
        void registerFunction(std::function<void()> function, const FunctionId functionId);

        /**
         * Add or replace a function to the RpcServer
         */
        void registerFunction(std::function<uint64_t()> function, const FunctionId functionId);

        /**
         * Add or replace a function to the RpcServer
         */
        template<class... Args>
        void registerFunction(std::function<uint64_t(Args...)> function, const FunctionId functionId);

        /**
         * Add or replace a function to the RpcServer
         */
        template<class... Args>
        void registerFunction(std::function<void(Args...)> function, const FunctionId functionId);

        /**
         * Process a single execution request
         */
        bool oneShot();

        /**
         * Process execution requests in loop
         */
        void run();

        /**
         * Invalid execution request handler
         */
        void setInvalidRequestEventHandler(InvalidRequestEventHandler& handler);

    };


    /*
     * Inline implementation
     */

    template <class... Args>
    inline void RpcServer::registerFunction(std::function<uint64_t(Args...)> function, const FunctionId functionId) {
        registerRpcFunction(
            [=](RpcExecutionRequest*, const std::vector<byte_t>& params) {
                return function(serializer.deserialize<Args>(params.data(), params.size())...);  // FIX using iterators
            },
            functionId
        );
    }

    template <class... Args>
    inline void RpcServer::registerFunction(std::function<void(Args...)> function, const FunctionId functionId) {
        registerRpcFunction(
            [=](RpcExecutionRequest*, const std::vector<byte_t>& params) {
                function(serializer.deserialize<Args>(params.data(), params.size())...);  // FIX using iterators
                return 0;
            },
            functionId
        );
    }

};


#endif
