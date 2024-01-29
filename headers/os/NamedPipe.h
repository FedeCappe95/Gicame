#ifndef __GICAME__NAMEDPIPE_H__
#define __GICAME__NAMEDPIPE_H__


#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include <string>


namespace Gicame::Os {

	class NamedPipe : public Gicame::IDataExchanger {

		MOVABLE_BUT_NOT_COPYABLE;

	private:
		enum class State { DISCONNECTED, CREATED, CONNECTED };

	private:
#ifdef WINDOWS
		void* handle;
#else
		int fd;
#endif
		const std::string fullName;
		State state;

	public:
		GICAME_API NamedPipe(const std::string& name);
		GICAME_API ~NamedPipe();
		GICAME_API bool create(const size_t bufferSize);
		GICAME_API bool open();
		GICAME_API virtual size_t send(const void* buffer, const size_t size) override final;
		GICAME_API virtual bool isSenderConnected() const override final;
		GICAME_API virtual size_t receive(void* buffer, const size_t size) override final;
		GICAME_API virtual bool isReceiverConnected() const override final;
		GICAME_API void close();

	};

};


#endif
