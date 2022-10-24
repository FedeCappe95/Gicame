#ifndef __RERBUILDING_H__
#define __RERBUILDING_H__


#include "./RpcCommon.h"


/**
 * Black magic that builds a RpcExecutionRequest
 */
namespace Gicame::RerBuilding {

    template <class Arg>
    inline void innerBuild(RpcExecutionRequest& rer) {
        // Do nothing
    }

    template <class Arg>
    inline void innerBuild(RpcExecutionRequest& rer, [[maybe_unused]] Arg arg) {
        rer.params[(rer.paramCount)++] = RpcParamDescriptor(sizeof(Arg));
    }

    template <class Arg, class... OtherArgs>
    inline void innerBuild(RpcExecutionRequest& rer, [[maybe_unused]] Arg arg, OtherArgs... otherArgs) {
        rer.params[(rer.paramCount)++] = RpcParamDescriptor(sizeof(Arg));
        innerBuild(rer, otherArgs...);
    }

    /**
    * Build a RpcExecutionRequest for the following functionId and the following parameters (args).
    * It uses innerBuild.
    */
    template <class... Args>
    inline RpcExecutionRequest build(const FunctionId functionId, Args... args) {
        RpcExecutionRequest rer(functionId, 0);
        innerBuild(rer, args...);
        return rer;
    }

    /**
    * Build a RpcExecutionRequest for the following functionId and the following parameters (args).
    * It uses innerBuild.
    */
    template <class... Args>
    inline RpcExecutionRequest build(const FunctionId functionId) {
        RpcExecutionRequest rer(functionId, 0);
        return rer;
    }

};


#endif
