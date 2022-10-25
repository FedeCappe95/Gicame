#ifndef __RERBUILDING_H__
#define __RERBUILDING_H__


#include <tuple>
#include "./RpcCommon.h"
#include "../dataSerialization/BinarySerializer.h"


/**
 * Black magic that builds a RpcExecutionRequest
 */
namespace Gicame::RerBuilding {

    template<class Arg, class... OtherArgs>
    inline void paramSerialization(const BinarySerializer& serializer, std::vector<byte_t>& dump, Arg arg) {
        const std::vector<byte_t> serializedParam = serializer.serialize(arg);
        dump.insert(dump.end(), serializedParam.begin(), serializedParam.end());
    }

    template<class Arg, class... OtherArgs>
    inline void paramSerialization(const BinarySerializer& serializer, std::vector<byte_t>& dump, Arg arg, OtherArgs... args) {
        const std::vector<byte_t> serializedParam = serializer.serialize(arg);
        dump.insert(dump.end(), serializedParam.begin(), serializedParam.end());
        paramSerialization(serializer, dump, args...);
    }

    /**
    * Build a RpcExecutionRequest for the following functionId and the following parameters (args).
    * It uses innerBuild.
    */
    template <class... Args>
    inline std::tuple<RpcExecutionRequest, std::vector<byte_t>> build(const BinarySerializer& serializer, const FunctionId functionId, Args... args) {
        constexpr size_t paramCount = sizeof...(args);
        const size_t paramSizes[] = { serializer.serializedSize(args)... };

        if (paramCount > RpcExecutionRequest::MAX_PARAM_COUNT)
            throw RUNTIME_ERROR("Too many parameters");

        RpcExecutionRequest rer(functionId, paramCount);
        size_t totalSize = 0;
        for (size_t paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
            rer.params[paramIndex].size = paramSizes[paramIndex];
            totalSize += paramSizes[paramIndex];
        }

        std::vector<byte_t> paramsDump;
        paramsDump.reserve(totalSize);
        paramSerialization(serializer, paramsDump, args...);

        return std::make_tuple(rer, paramsDump);
    }

    /**
    * Build a RpcExecutionRequest for the following functionId and the following parameters (args).
    * It uses innerBuild.
    */
    template <class... Args>
    inline std::tuple<RpcExecutionRequest, std::vector<byte_t>> build([[maybe_unused]] const BinarySerializer& serializer, const FunctionId functionId) {
        RpcExecutionRequest rer(functionId, 0);
        std::vector<byte_t> paramsDump;
        return std::make_tuple(rer, paramsDump);
    }

};


#endif
