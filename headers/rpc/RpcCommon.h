#ifndef __RPCCOMMON_H__
#define __RPCCOMMON_H__


#include "../common.h"
#include <functional>


namespace Gicame {

    /**
     * An RpcFunction is identified by a natural number on 32 bits
     */
    typedef uint32_t FunctionId;

    /**
     * Type of RPC callbacks
     */
    //typedef std::function<uint64_t(RpcExecutionRequest*)> RpcFunction;
    #define RpcFunction std::function<uint64_t(RpcExecutionRequest*)>

    /**
     * Describe a paramenter of a RpcExecutionRequest
     */
    class RpcParamDescriptor {

    public:
        uint64_t size;

    public:
        constexpr RpcParamDescriptor() : size(0) {}
        constexpr RpcParamDescriptor(const uint64_t size) : size(size) {}
        constexpr RpcParamDescriptor(const RpcParamDescriptor& other) : size(other.size) {}
        constexpr void operator=(const RpcParamDescriptor& other) { size = other.size; }

    };

    /**
     * The RpcClient send an execution request to the RpcServer. The request is represented by an
     * instance of RpcExecutionRequest.
     */
    class RpcExecutionRequest {

    public:
        static constexpr uint32_t MAGIC = 0xDEADBEEFu;
        static constexpr uint32_t MAX_PARAM_COUNT = 6u;

    public:
        const uint32_t magic;
        const FunctionId functionId;
        RpcParamDescriptor params[MAX_PARAM_COUNT];
        const uint8_t paramCount;

    public:
        RpcExecutionRequest(const FunctionId functionId, const uint8_t paramCount);
        bool checkIntegrity();

    }__attribute__((packed));


    /*
     * Inline implementation
     */

    inline RpcExecutionRequest::RpcExecutionRequest(const FunctionId functionId, const uint8_t paramCount) :
        magic(MAGIC),
        functionId(functionId),
        paramCount(paramCount)
    {
        if (unlikely(paramCount > MAX_PARAM_COUNT)) {
            throw RUNTIME_ERROR("Invalid paramCount");
        }
        for (uint32_t paramIndex = 0; paramIndex < MAX_PARAM_COUNT; ++paramIndex) {
            params[paramIndex] = RpcParamDescriptor();
        }
    }

    inline bool RpcExecutionRequest::checkIntegrity() {
        return magic == MAGIC && paramCount < MAX_PARAM_COUNT;
    }

};


#endif
