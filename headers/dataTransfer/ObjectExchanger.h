#ifndef __GICAME__OBJECTEXCHANGER_H__
#define __GICAME__OBJECTEXCHANGER_H__


#include "../common.h"
#include "../interfaces/IReceiver.h"
#include "../interfaces/ISender.h"
#include "../interfaces/IDataExchanger.h"
#include "./ObjectSender.h"
#include "./ObjectReceiver.h"


namespace Gicame {

	/**
	 * An ObjectExchanger is both an ObjectSender and an ObjectReceiver. It implements a full-duplex
	 * communication.
	 */
	class ObjectExchanger :
		public IDataExchanger, public ObjectSender, public ObjectReceiver
	{

	private:
		IDataExchanger* dataExchanger;

	public:
		ObjectExchanger(IDataExchanger* dataExchanger);
		virtual bool isConnected() const override;

	};


	/*
	 * Inline implementation
	 */

	inline ObjectExchanger::ObjectExchanger(IDataExchanger* dataExchanger) :
		dataExchanger(dataExchanger),
		ObjectSender(dataExchanger),
		ObjectReceiver(dataExchanger)
	{}

	inline bool ObjectExchanger::isConnected() const {
		return dataExchanger->isConnected();
	}

};


#endif
