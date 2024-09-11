#ifndef __GICAME__SERIAL_H__
#define __GICAME__SERIAL_H__


#include <stdint.h>
#include <stdexcept>
#include "../common.h"
#include "../utils/MovableButNotCopyable.h"
#include "../interfaces/IDataExchanger.h"


namespace Gicame::Device {

	class GICAME_API Serial : public Gicame::IDataExchanger {

		MOVABLE_BUT_NOT_COPYABLE;

	public:
		static constexpr uint32_t P_NOPARITY = 0;
		static constexpr uint32_t P_ODDPARITY = 1;
		static constexpr uint32_t P_EVENPARITY = 2;
		static constexpr uint32_t P_MARKPARITY = 3;
		static constexpr uint32_t P_SPACEPARITY = 4;
		static constexpr uint32_t SB_ONESTOPBIT = 0;
		static constexpr uint32_t SB_ONE5STOPBITS = 1;
		static constexpr uint32_t SB_TWOSTOPBITS = 2;

	public:
		struct SerialPort {
			std::string name;
			uint32_t index;
			inline SerialPort(const std::string& name, const uint32_t index) : name(name), index(index) {}
		};

	private:
		void* handle;
		const uint32_t comIndex;
		const uint32_t boud;
		const uint32_t byteSize;
		const uint32_t parity;
		const uint32_t stopBitsMode;

	public:
		constexpr Serial(
			const uint32_t comIndex, const uint32_t boud, const uint32_t byteSize, const uint32_t parity, const uint32_t stopBitsMode
		) :
			handle(NULL), comIndex(comIndex), boud(boud), byteSize(byteSize), parity(parity), stopBitsMode(stopBitsMode) {}
		/**
		 * Open a serial communication
		 * @return true if the communication can be established
		 */
		bool open();
		size_t send(const uint8_t* what, const size_t size);
		uint8_t* receive(uint8_t* outBuffer, const size_t size);
		/**
		 * Close the serial communication is previously open
		 */
		void close();

	public:
		static std::vector<SerialPort> enumerateSerialPorts();

	};

};


#endif
