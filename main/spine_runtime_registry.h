#ifndef SPINELOVE_SPINE_RUNTIME_REGISTRY_H_
#define SPINELOVE_SPINE_RUNTIME_REGISTRY_H_

#include <array>
#include <memory>

#include "runtime_shared/spine_player_api.h"

#include "render_d3d11/d3d11_renderer.h"

class SlRuntimeHub
{
public:
	SlRuntimeHub();
	~SlRuntimeHub();

	bool RebuildRuntimePool();
	bool RuntimePoolReady() const noexcept;

	enum class RuntimeLane : uint8_t
	{
		Unknown = static_cast<uint8_t>(-1U),
		Runtime21 = 0,
		Runtime34,
		Runtime35,
		Runtime36,
		Runtime37,
		Runtime38,
		Runtime40,
		Runtime41,
		Runtime42,
		End
	};

	RuntimeLane LaneForVersionText(const char* version) const noexcept;
	bool ActivateLane(RuntimeLane slot) noexcept;
	RuntimeLane CurrentLane() const noexcept;

	SlPlaybackRuntime* RuntimeForLane(RuntimeLane slot) const;
	bool LaneIsReady(RuntimeLane slot) const;
	SlPlaybackRuntime* CurrentRuntime() const;
	bool RenderCurrentRuntimeD3D11(sl_d3d11::D3D11Renderer& renderer);

private:
	static constexpr size_t RuntimeLaneCount = 9;
	static_assert(RuntimeLaneCount == static_cast<uint8_t>(RuntimeLane::End), "Runtime lane table size is out of sync.");

	std::array<std::unique_ptr<SlPlaybackRuntime>, RuntimeLaneCount> m_runtimeSlots;
	RuntimeLane m_currentLane = RuntimeLane::Runtime38;
	bool m_runtimePoolReady = true;
};

#endif
