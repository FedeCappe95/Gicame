#ifndef __RPCCOMMON_H__
#define __RPCCOMMON_H__


#include <vector>
#include <functional>
#include "../common.h"
#include "../interfaces/ISerializable.h"
#include "./RpcCommon.h"


namespace Gicame {

    /*
     * Prototypes
     */
    class RpcExecutionRequest;

    /**
     * An RpcFunction is identified by a natural number on 32 bits
     */
    typedef uint32_t FunctionId;

    /**
     * Type of RPC callbacks
     */
    using RpcFunction = std::function<uint64_t(RpcExecutionRequest*, const std::vector<std::vector<byte_t>>&)>;

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
    class RpcExecutionRequest : public ISerializable {

    public:
        static constexpr uint32_t MAGIC = 0xDEADBEEFu;
        static constexpr uint32_t MAGIC_FOR_EMPTY_REQUESTS = 0xFFFFFFFFu;
        static constexpr uint32_t MAX_PARAM_COUNT = 6u;

    public:
        const uint32_t magic;
        const FunctionId functionId;
        RpcParamDescriptor params[MAX_PARAM_COUNT];
        uint8_t paramCount;

    public:
        RpcExecutionRequest();
        RpcExecutionRequest(const FunctionId functionId, const uint8_t paramCount);
        bool checkIntegrity();
        std::vector<byte_t> serialize() const;

    public:
        static RpcExecutionRequest deserialize(const std::vector<byte_t>& serialized);

    };


    /*
     * Inline implementation
     */

    inline RpcExecutionRequest::RpcExecutionRequest() :
        magic(MAGIC_FOR_EMPTY_REQUESTS),
        functionId(0),
        paramCount(0)
    {}

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

    inline std::vector<byte_t> RpcExecutionRequest::serialize() const {
        std::vector<byte_t> serialized;
        serialized.reserve(sizeof(magic) + sizeof(functionId) + sizeof(params) + sizeof(paramCount));
        for (byte_t* ptr = (byte_t*)&magic; ptr < (byte_t*)&magic + sizeof(magic); ++ptr)
            serialized.emplace_back(*ptr);
        for (byte_t* ptr = (byte_t*)&functionId; ptr < (byte_t*)&functionId + sizeof(functionId); ++ptr)
            serialized.emplace_back(*ptr);
        for (uint32_t paramIndex = 0; paramIndex < MAX_PARAM_COUNT; ++paramIndex)
            for (byte_t* ptr = (byte_t*)&params[paramIndex]; ptr < (byte_t*)&params[paramIndex] + sizeof(params[paramIndex]); ++ptr)
                serialized.emplace_back(*ptr);
        for (byte_t* ptr = (byte_t*)&paramCount; ptr < (byte_t*)&paramCount + sizeof(paramCount); ++ptr)
            serialized.emplace_back(*ptr);
        return serialized;
    }

    inline RpcExecutionRequest RpcExecutionRequest::deserialize(const std::vector<byte_t>& serialized) {
        if (serialized.size() != sizeof(magic) + sizeof(functionId) + sizeof(params) + sizeof(paramCount))
            throw RUNTIME_ERROR("Invalid deserialization source: wrong size");

        RpcExecutionRequest rer;

        const byte_t* serPtr = serialized.data();

        for (byte_t* ptr = (byte_t*)&rer.magic; ptr < (byte_t*)&rer.magic + sizeof(magic); ++ptr, ++serPtr)
            *ptr = *serPtr;
        for (byte_t* ptr = (byte_t*)&rer.functionId; ptr < (byte_t*)&rer.functionId + sizeof(functionId); ++ptr, ++serPtr)
            *ptr = *serPtr;
        for (uint32_t paramIndex = 0; paramIndex < MAX_PARAM_COUNT; ++paramIndex)
            for (byte_t* ptr = (byte_t*)&rer.params[paramIndex]; ptr < (byte_t*)&rer.params[paramIndex] + sizeof(params[paramIndex]); ++ptr, ++serPtr)
                *ptr = *serPtr;
        for (byte_t* ptr = (byte_t*)&rer.paramCount; ptr < (byte_t*)&rer.paramCount + sizeof(paramCount); ++ptr, ++serPtr)
            *ptr = *serPtr;

        return rer;
    }

};


#endif
