#ifndef __OBJECTEXCHANGER_H__
#define __OBJECTEXCHANGER_H__


#include "../common.h"
#include "../interfaces/IReceiver.h"
#include "../interfaces/ISender.h"
#include "../interfaces/IDataExchanger.h"
#include "ObjectSender.h"
#include "ObjectReceiver.h"


namespace Gicame {

	/**
	 * An ObjectExchanger is both an ObjectSender and an ObjectReceiver. It implements a full-duplex
	 * communication.
	 */
	class ObjectExchanger :
		public IDataExchanger, public ObjectSender, public ObjectReceiver
	{

	public:
		ObjectExchanger(IDataExchanger* dataExchanger);

	};


	/*
	 * Inline implementation
	 */

	inline ObjectExchanger::ObjectExchanger(IDataExchanger* dataExchanger) :
		ObjectSender(dataExchanger),
		ObjectReceiver(dataExchanger)
	{}

};


#endif
