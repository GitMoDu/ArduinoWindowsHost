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


		public:
			ArduinoSerialPort(uint8_t portId,
				size_t lineCapacity = 1024)
				: PortId(portId)
				, LineCapacity(lineCapacity)
				, m_lines(lineCapacity)
			{
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

			// Clear stored lines and current assembling line.
			void clearBuffer()
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				std::fill(m_lines.begin(), m_lines.end(), std::string());
				m_head = 0;
				m_count = 0;
				m_currentLine.clear();
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
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.push_back(value);
			}

			void println(const char value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.push_back(value);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const char* value)
			{
				if (value == nullptr) return;
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(value);
			}

			void println(const char* value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				if (value != nullptr) m_currentLine.append(value);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const uint8_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<unsigned int>(value)));
			}

			void println(const uint8_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<unsigned int>(value)));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const int8_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));
			}

			void println(const int8_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const uint16_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
			}

			void println(const uint16_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const int16_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));
			}

			void println(const int16_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(static_cast<int>(value)));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const uint32_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
			}

			void println(const uint32_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			void print(const int32_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
			}

			void println(const int32_t value)
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				m_currentLine.append(std::to_string(value));
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
			}

			// finalize the current line (even if empty) and push it into the buffer.
			void println()
			{
				std::lock_guard<std::mutex> lk(m_mutex);
				pushLineLocked(std::move(m_currentLine));
				m_currentLine.clear();
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
		};
	}
}