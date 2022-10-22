#ifndef __RERBUILDING_H__
#define __RERBUILDING_H__


#include "./RpcCommon.h"


/**
 * Black magic that builds a RpcExecutionRequest
 */
namespace Gicame::RerBuilding {

    template <class Arg, class... OtherArgs>
    inline void innerBuild(RpcExecutionRequest& rer, Arg arg, OtherArgs... otherArgs) {
        rer.params[++(rer.paramCount)] = RpcParamDescriptor(sizeof(Arg));
        innerBuild(rer, otherArgs);
    }

    template <class Arg>
    inline void innerBuild(RpcExecutionRequest& rer, Arg arg) {
        rer.params[++(rer.paramCount)] = RpcParamDescriptor(sizeof(Arg));
    }

    /**
    * Build a RpcExecutionRequest for the following functionId and the following parameters (args).
    * It uses innerBuild.
    */
    template <class... Args>
    inline RpcExecutionRequest build(const FunctionId functionId, Args... args) {
        RpcExecutionRequest rer(functionId, 0);
        innerBuild(rer);
        return rer;
    }

};


#endif
