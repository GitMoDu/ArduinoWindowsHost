#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>

namespace ArduinoWindowsHost
{
	namespace Hal
	{
		// Arduino flash string helper (F("...")) compatibility.
		// On actual Arduino you would read from PROGMEM; here we treat it as normal C string.
#if !defined(ARDUINO)
		struct __FlashStringHelper; // forward (host)
#endif

		class ArduinoSerialPort
		{
		private:
			mutable std::mutex m_mutex;

			uint8_t PortId;
			size_t LineCapacity;

			size_t m_head = 0;   // next write index
			size_t m_count = 0;  // number of valid entries

			// current (not-yet-terminated) line assembled from successive print() calls
			std::string m_currentLine;

			// circular buffer storage
			std::vector<std::string> m_lines;

		private:
			// --- RX circular buffer (fixed size, short) ---
			static constexpr size_t RxBufferCapacity = 256;
			char   m_rxBuffer[RxBufferCapacity]{};
			size_t m_rxHead = 0;   // next write position
			size_t m_rxTail = 0;   // next read position
			size_t m_rxCount = 0;  // number of bytes s

		private:
			std::atomic<bool> m_ready{ false }; // added

			volatile uint32_t TxId = 0;
			volatile uint32_t RxId = 0;
			volatile uint32_t LastRx = 0;
			volatile uint32_t LastTx = 0;

		public:
			ArduinoSerialPort(uint8_t portId,
				size_t lineCapacity = 1024)
				: PortId(portId)
				, LineCapacity(lineCapacity)
				, m_lines(lineCapacity)
			{
			}

			uint32_t ElapsedTx() const
			{
				return GetTimestamp() - LastTx;
			}

			uint32_t ElapsedRx() const
			{
				return GetTimestamp() - LastRx;
			}

			uint32_t GetRxId() const
			{
				return RxId;
			}

			uint32_t GetTxId() const
			{
				return TxId;
			}

			explicit operator bool() const noexcept
			{
				return m_ready.load(std::memory_order_acquire);
			}

			// Call when the virtual port becomes usable.
			void begin(uint32_t baudRate = 0)
			{
				m_ready.store(true, std::memory_order_release);
			}

			// Call to mark it unavailable.
			void end()
			{
				m_ready.store(false, std::memory_order_release);
			}


			// --- Arduino-style RX API additions ---

			// Number of bytes available to read.
			size_t available() const
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				return m_rxCount;
			}

			// Peek next byte without removing; returns -1 if none.
			int peek() const
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				if (m_rxCount == 0) return -1;
				return static_cast<unsigned char>(m_rxBuffer[m_rxTail]);
			}

			// Read next byte; returns -1 if none.
			int read()
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				if (m_rxCount == 0) return -1;
				char c = m_rxBuffer[m_rxTail];
				m_rxTail = (m_rxTail + 1) % RxBufferCapacity;
				--m_rxCount;
				return static_cast<unsigned char>(c);
			}

			// Clear RX buffer only.
			void flushRx()
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_rxHead = m_rxTail = m_rxCount = 0;
			}

			// Arduino's modern flush() waits for outgoing data. We have no
			// async TX queue, so we repurpose to clear RX (legacy behavior).
			void flush()
			{
				flushRx();
			}

			// Return buffered lines in chronological order (oldest first).
			std::vector<std::string> getBufferedLines() const
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				std::vector<std::string> out;
				out.reserve(m_count);
				size_t start = (m_head + LineCapacity - m_count) % LineCapacity;
				for (size_t i = 0; i < m_count; ++i)
				{
					out.push_back(m_lines[(start + i) % LineCapacity]);
				}
				return out;
			}

			// Clear stored output lines and current assembling line.
			void flusthTx()
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				std::fill(m_lines.begin(), m_lines.end(), std::string());
				m_head = 0;
				m_count = 0;
				m_currentLine.clear();

				TxId++;
			}

			// Number of stored lines currently in buffer.
			size_t size() const
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				return m_count;
			}

			// --- Print API (Arduino-like) ---
			// Behavior:
			// - print(...) appends to the current line buffer.
			// - println(...) appends (if any) and then finalizes the line, pushing it into the circular buffer.

			void print(const char value)
			{
				OnTx();

				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.push_back(value);

			}

			void println(const char value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.push_back(value);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();

			}

			void print(const char* value)
			{
				if (value == nullptr) return;
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(value);

			}

			void println(const char* value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				if (value != nullptr) m_currentLine.append(value);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const uint8_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<unsigned int>(value)));
			}

			void println(const uint8_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<unsigned int>(value)));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const int8_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));
			}

			void println(const int8_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const uint16_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
			}

			void println(const uint16_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();

			}

			void print(const int16_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));

			}

			void println(const int16_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();

			}

			void print(const uint32_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
			}

			void println(const uint32_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const int32_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
			}

			void println(const int32_t value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void println()
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			// std::string
			void print(const std::string& value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(value);
			}

			void println(const std::string& value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(value);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			// C-string with explicit length (may contain nulls)
			void print(const char* value, size_t length)
			{
				if (!value || length == 0) return;
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(value, length);
			}

			void println(const char* value, size_t length)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				if (value && length)
					m_currentLine.append(value, length);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const __FlashStringHelper* value)
			{
				if (!value) return;
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(reinterpret_cast<const char*>(value));
			}

			void println(const __FlashStringHelper* value)
			{
				OnTx();
				std::lock_guard<std::mutex> lk(m_mutex);
				if (value)
					m_currentLine.append(reinterpret_cast<const char*>(value));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

		public:
			void Rx(const char value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				if (m_rxCount < RxBufferCapacity)
				{
					m_rxBuffer[m_rxHead] = value;
					m_rxHead = (m_rxHead + 1) % RxBufferCapacity;
					++m_rxCount;
					OnRx();
				}
			}

			// Feed a null-terminated C string. Stops at first '\0'.
			size_t Rx(const char* cstr)
			{
				if (!cstr) return 0;
				return Rx(cstr, std::char_traits<char>::length(cstr));
			}

			// Feed a raw char buffer (may contain nulls). Returns number of bytes accepted.
			size_t Rx(const char* data, size_t length)
			{
				if (!data || length == 0) return 0;
				std::lock_guard<std::mutex> lk(m_mutex);
				size_t accepted = 0;
				while (accepted < length && m_rxCount < RxBufferCapacity)
				{
					m_rxBuffer[m_rxHead] = data[accepted];
					m_rxHead = (m_rxHead + 1) % RxBufferCapacity;
					++m_rxCount;
					++accepted;
				}
				if (accepted)
				{
					OnRx(); // single state change for the whole batch
				}
				return accepted;
			}

			// Feed a raw unsigned byte buffer.
			size_t Rx(const uint8_t* data, size_t length)
			{
				return Rx(reinterpret_cast<const char*>(data), length);
			}

			// Feed a std::string (all characters).
			size_t Rx(const std::string& s)
			{
				return Rx(s.data(), s.size());
			}

			// Convenience: feed a line (appends '\n' if not present).
			size_t RxLine(const std::string& s)
			{
				if (s.empty())
					return Rx("\n", 1);
				if (s.back() == '\n')
					return Rx(s.data(), s.size());
				size_t a = Rx(s.data(), s.size());
				a += Rx("\n", 1);
				return a;
			}

		private:
			// push a complete line into the circular buffer (oldest overwritten when full)
			void pushLineLocked(std::string&& line)
			{
				// assumes m_mutex is held
				m_lines[m_head] = std::move(line);
				m_head = (m_head + 1) % LineCapacity;
				if (m_count < LineCapacity) ++m_count;
			}

			// Iterate over buffered lines (oldest first).
			template <typename F>
			void for_each_buffered_line(F&& f) const
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				size_t start = (m_head + LineCapacity - m_count) % LineCapacity;
				for (size_t i = 0; i < m_count; ++i)
				{
					f(m_lines[(start + i) % LineCapacity]);
				}
			}

		private:
			void OnTx()
			{
				LastTx = GetTimestamp();
				TxId++;
			}

			void OnRx()
			{
				LastRx = GetTimestamp();
				RxId++;
			}

			static uint32_t GetTimestamp()
			{
				using namespace std::chrono;

				return static_cast<uint32_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
			}
		};
	}
}