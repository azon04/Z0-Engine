#ifndef __ZE_STRING__
#define __ZE_STRING__

#include "Memory/Handle.h"
#include "Memory/MemoryHelper.h"

#include "StringFunc.h"

namespace ZE
{
	// Class wrapper for char*
	class String
	{

	public:
		String() 
		{ 
			m_handle = Handle();
		}

		String(int reservedSize)
		{
			m_handle = Handle(reservedSize);
			m_handle.getObject<char>()[0] = '\0';
		}

		String(const String& other)
		{
			Handle otherHandle = other.getHandle();
			char* otherText = otherHandle.getObject<char>();
			
			size_t size = StringFunc::Length(otherText) + 1;

			if (m_handle.getCapacity() < size)
			{
				if (m_handle.isValid())
				{
					m_handle.release();
				}
				m_handle = Handle(size);
			}

			char* text = m_handle.getObject<char>();

			StringFunc::WriteTo(text, otherText, size);
		}

		String& operator=(const String& other)
		{
			Handle otherHandle = other.getHandle();
			char* otherText = otherHandle.getObject<char>();

			size_t size = StringFunc::Length(otherText) + 1;

			if (m_handle.getCapacity() < size)
			{
				if (m_handle.isValid())
				{
					m_handle.release();
				}
				m_handle = Handle(size);
			}

			char* text = m_handle.getObject<char>();

			StringFunc::WriteTo(text, otherText, size);

			return *this;
		}

		char& operator[](const Int32 index)
		{
			return m_handle.getObject<char>()[index];
		}

		char& operator[](const Int32 index) const
		{
			return ((char*)m_handle.getObjectConst())[index];
		}

		String(const char* text)
		{
			size_t size = StringFunc::Length(text) + 1;
			m_handle = Handle(size);

			char* cText = m_handle.getObject<char>();
			
			StringFunc::WriteTo(cText, text, size);
		}

		~String()
		{
			if (m_handle.isValid())
			{
				m_handle.release();
			}
		}

		String& operator=(const char* text)
		{
			size_t size = StringFunc::Length(text) + 1;

			if (m_handle.getCapacity() < size)
			{
				if (m_handle.isValid())
				{
					m_handle.release();
				}
				m_handle = Handle(size);
			}

			char* cText = m_handle.getObject<char>();

			StringFunc::WriteTo(cText, text, size);

			return *this;
		}

		String& operator+=(const char* text)
		{
			size_t size = StringFunc::Length(text) + 1;
			size_t currentLength = length();

			Handle newHandle;

			if (m_handle.getCapacity() < size + currentLength + 1)
			{
				newHandle = Handle(size+currentLength+1);
				MemoryHelper::Copy(m_handle.getObject(), newHandle.getObject(), currentLength);
				if (m_handle.isValid())
				{
					m_handle.release();
				}
				m_handle = newHandle;
			}

			char* cText = m_handle.getObject<char>() + currentLength;

			StringFunc::WriteTo(cText, text, size);

			return *this;
		}

		bool operator==(const char* text) const
		{
			return StringFunc::Compare(const_str(), text) == 0;
		}

		bool operator==(const String& text) const 
		{
			return StringFunc::Compare(const_str(), text.const_str()) == 0;
		}

		bool operator!=(const String& text) const
		{
			return StringFunc::Compare(const_str(), text.const_str()) != 0;
		}

		char* const_str() const { return (char*)(m_handle.isValid() ? m_handle.getObjectConst() : nullptr); }
		char* c_str() { return m_handle.isValid() ? m_handle.getObject<char>() : nullptr; }

		Int32 length() const { return StringFunc::Length(const_str()); }
		Handle getHandle() const { return m_handle; }

		Handle m_handle;
	};
};
#endif
