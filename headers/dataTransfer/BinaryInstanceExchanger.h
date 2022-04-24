#ifndef __BINARYINSTANCERECEIVER_H__
#define __BINARYINSTANCERECEIVER_H__


#include "../common.h"
#include "../interfaces/IDataExhanger.h"
#include "BinaryInstanceSender.h"
#include "BinaryInstanceReceiver.h"


namespace Gicame {

	/**
	 * A BinaryInstanceExhanger is both a BinaryInstanceSender and a BinaryInstanceReceiver.
	 * It implements a full-duplex communication.
	 */
	class BinaryInstanceExhanger : public BinaryInstanceSender, public BinaryInstanceReceiver {

	public:
		BinaryInstanceExhanger(IDataExhanger* dataExchanger);

	};


	/*
	 * Inline implementation
	 */

	inline BinaryInstanceExhanger::BinaryInstanceExhanger(IDataExhanger* dataExchanger) :
		BinaryInstanceSender(dataExchanger),
		BinaryInstanceReceiver(dataExchanger)
	{}

};


#endif
