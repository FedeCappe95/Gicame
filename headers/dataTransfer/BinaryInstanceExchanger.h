#ifndef __BINARYINSTANCERECEIVER_H__
#define __BINARYINSTANCERECEIVER_H__


#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "BinaryInstanceSender.h"
#include "BinaryInstanceReceiver.h"


namespace Gicame {

	/**
	 * A BinaryInstanceExchanger is both a BinaryInstanceSender and a BinaryInstanceReceiver.
	 * It implements a full-duplex communication.
	 */
	class BinaryInstanceExchanger : public BinaryInstanceSender, public BinaryInstanceReceiver {

	public:
		BinaryInstanceExchanger(IDataExchanger* dataExchanger);

	};


	/*
	 * Inline implementation
	 */

	inline BinaryInstanceExchanger::BinaryInstanceExchanger(IDataExchanger* dataExchanger) :
		BinaryInstanceSender(dataExchanger),
		BinaryInstanceReceiver(dataExchanger)
	{}

};


#endif
