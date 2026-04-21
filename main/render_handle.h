#ifndef RENDER_HANDLE_H_
#define RENDER_HANDLE_H_

#include <utility>


template <int (*Deleter)(int)>
class DxLibHandle
{
public:
	DxLibHandle() noexcept = default;

	explicit DxLibHandle(int handle) noexcept
		: m_handle(handle)
	{}

	~DxLibHandle() { Release(); }


	int  Get()   const noexcept { return m_handle; }
	bool Empty() const noexcept { return m_handle == -1; }
	explicit operator bool() const noexcept { return m_handle != -1; }


	void Reset() { Release(); }

	void Reset(int newHandle) noexcept
	{
		Release();
		m_handle = newHandle;
	}

	int Detach() noexcept
	{
		int h = m_handle;
		m_handle = -1;
		return h;
	}

	void Swap(DxLibHandle& other) noexcept { std::swap(m_handle, other.m_handle); }


	DxLibHandle(DxLibHandle&& other) noexcept
		: m_handle(other.Detach())
	{}

	DxLibHandle& operator=(DxLibHandle&& other) noexcept
	{
		if (this != &other)
		{
			Release();
			m_handle = other.Detach();
		}
		return *this;
	}


	DxLibHandle(const DxLibHandle&) = delete;
	DxLibHandle& operator=(const DxLibHandle&) = delete;

private:
	int m_handle = -1;

	void Release() noexcept
	{
		if (m_handle != -1 && Deleter != nullptr)
		{
			Deleter(m_handle);
			m_handle = -1;
		}
	}
};

#endif
