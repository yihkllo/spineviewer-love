#include "d3d11_overlay_drawer.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "d3d11_renderer.h"
#include "../runtime_shared/slot_mesh_data.h"

namespace
{
	SlColor ToD3D11OverlayColor(unsigned int color) noexcept
	{
		return SlColor(
			static_cast<float>((color >> 16) & 0xff) / 255.0f,
			static_cast<float>((color >> 8) & 0xff) / 255.0f,
			static_cast<float>(color & 0xff) / 255.0f,
			1.0f);
	}

	void DrawD3D11Line(sl_d3d11::D3D11Renderer& renderer, const SlVec2& a, const SlVec2& b, float thickness, const SlColor& color)
	{
		const float dx = b.x - a.x;
		const float dy = b.y - a.y;
		const float len = std::sqrt(dx * dx + dy * dy);
		if (len <= 0.001f)
			return;

		const float halfThickness = (thickness > 1.0f ? thickness : 1.0f) * 0.5f;
		const float nx = -dy / len * halfThickness;
		const float ny = dx / len * halfThickness;

		SlVertex2D vertices[4];
		vertices[0].pos = SlVec3(a.x + nx, a.y + ny, 0.0f);
		vertices[1].pos = SlVec3(b.x + nx, b.y + ny, 0.0f);
		vertices[2].pos = SlVec3(b.x - nx, b.y - ny, 0.0f);
		vertices[3].pos = SlVec3(a.x - nx, a.y - ny, 0.0f);
		for (SlVertex2D& vertex : vertices)
		{
			vertex.uv = SlVec2(0.0f, 0.0f);
			vertex.color = color;
		}
		const unsigned short indices[] = { 0, 1, 2, 2, 3, 0 };
		renderer.DrawTriangles(1, vertices, 4, indices, 6, SlBlendMode::Normal, false);
	}

	SlVec2 TransformOverlayPoint(float x, float y, const SlMatrix4& transform)
	{
		const SlVec3 point = SlVec3Transform(SlVec3(x, y, 0.0f), transform);
		return SlVec2(point.x, point.y);
	}

	struct MeshEdge
	{
		unsigned short a = 0;
		unsigned short b = 0;
		int hits = 0;
	};

	bool AppendMeshEdge(std::vector<MeshEdge>& edges, unsigned short a, unsigned short b)
	{
		if (a == b)
			return false;

		if (b < a)
			std::swap(a, b);

		for (MeshEdge& edge : edges)
		{
			if (edge.a == a && edge.b == b)
			{
				++edge.hits;
				return true;
			}
		}

		edges.push_back(MeshEdge{ a, b, 1 });
		return true;
	}

	int HullPointCount(const ReadSlotMeshData& meshData, int vertexCount) noexcept
	{
		if (meshData.hullLength <= 1)
			return 0;
		if (meshData.hullLength <= vertexCount)
			return meshData.hullLength;
		if ((meshData.hullLength % 2) == 0 && meshData.hullLength / 2 <= vertexCount)
			return meshData.hullLength / 2;
		return 0;
	}

	void DrawOrderedOutline(sl_d3d11::D3D11Renderer& renderer, const ReadSlotMeshData& meshData, const SlMatrix4& transform, float thickness, const SlColor& color)
	{
		const auto& vertices = meshData.worldVertices;
		const int vertexCount = static_cast<int>(vertices.size() / 2);
		int pointCount = HullPointCount(meshData, vertexCount);
		if (pointCount <= 0 && meshData.isRegion)
			pointCount = vertexCount;
		if (pointCount < 2)
			return;

		SlVec2 previous = TransformOverlayPoint(vertices[(pointCount - 1) * 2], vertices[(pointCount - 1) * 2 + 1], transform);
		for (int i = 0; i < pointCount; ++i)
		{
			const SlVec2 current = TransformOverlayPoint(vertices[i * 2], vertices[i * 2 + 1], transform);
			DrawD3D11Line(renderer, previous, current, thickness, color);
			previous = current;
		}
	}

	bool DrawTriangleBoundary(sl_d3d11::D3D11Renderer& renderer, const ReadSlotMeshData& meshData, const SlMatrix4& transform, float thickness, const SlColor& color)
	{
		const auto& vertices = meshData.worldVertices;
		const auto& indices = meshData.triangles;
		const int vertexCount = static_cast<int>(vertices.size() / 2);
		if (vertexCount < 2 || indices.size() < 3)
			return false;

		std::vector<MeshEdge> edges;
		edges.reserve(indices.size());
		for (std::size_t i = 0; i + 2 < indices.size(); i += 3)
		{
			const unsigned short i0 = indices[i + 0];
			const unsigned short i1 = indices[i + 1];
			const unsigned short i2 = indices[i + 2];
			if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount)
				continue;

			AppendMeshEdge(edges, i0, i1);
			AppendMeshEdge(edges, i1, i2);
			AppendMeshEdge(edges, i2, i0);
		}

		bool drewAny = false;
		for (const MeshEdge& edge : edges)
		{
			if (edge.hits != 1)
				continue;

			const SlVec2 a = TransformOverlayPoint(vertices[edge.a * 2], vertices[edge.a * 2 + 1], transform);
			const SlVec2 b = TransformOverlayPoint(vertices[edge.b * 2], vertices[edge.b * 2 + 1], transform);
			DrawD3D11Line(renderer, a, b, thickness, color);
			drewAny = true;
		}
		return drewAny;
	}

	struct LocalBounds
	{
		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;
	};

	bool FindVertexBounds(const ReadSlotMeshData& meshData, LocalBounds& bounds) noexcept
	{
		if (meshData.worldVertices.size() < 2)
			return false;

		bounds.minX = meshData.worldVertices[0];
		bounds.maxX = bounds.minX;
		bounds.minY = meshData.worldVertices[1];
		bounds.maxY = bounds.minY;
		for (std::size_t i = 2; i + 1 < meshData.worldVertices.size(); i += 2)
		{
			const float x = meshData.worldVertices[i];
			const float y = meshData.worldVertices[i + 1];
			if (x < bounds.minX) bounds.minX = x;
			if (x > bounds.maxX) bounds.maxX = x;
			if (y < bounds.minY) bounds.minY = y;
			if (y > bounds.maxY) bounds.maxY = y;
		}

		return std::isfinite(bounds.minX) && std::isfinite(bounds.minY) &&
			std::isfinite(bounds.maxX) && std::isfinite(bounds.maxY) &&
			bounds.maxX > bounds.minX && bounds.maxY > bounds.minY;
	}

	void DrawTransformedEdgeDots(sl_d3d11::D3D11Renderer& renderer, const std::vector<SlVec2>& points, const SlMatrix4& transform, float thickness, const SlColor& color)
	{
		const float half = (std::max)(1.0f, thickness) * 0.5f;
		constexpr std::size_t kMaxDotsPerBatch = 12000;
		std::vector<SlVertex2D> vertices;
		std::vector<unsigned short> indices;
		vertices.reserve(kMaxDotsPerBatch * 4);
		indices.reserve(kMaxDotsPerBatch * 6);

		auto flush = [&]()
		{
			if (!vertices.empty() && !indices.empty())
			{
				renderer.DrawTriangles(1, vertices.data(), static_cast<int>(vertices.size()), indices.data(), static_cast<int>(indices.size()), SlBlendMode::Normal, false);
				vertices.clear();
				indices.clear();
			}
		};

		for (const SlVec2& point : points)
		{
			if (vertices.size() / 4 >= kMaxDotsPerBatch)
				flush();

			const unsigned short base = static_cast<unsigned short>(vertices.size());
			SlVertex2D quad[4];
			const SlVec2 p0 = TransformOverlayPoint(point.x - half, point.y - half, transform);
			const SlVec2 p1 = TransformOverlayPoint(point.x + half, point.y - half, transform);
			const SlVec2 p2 = TransformOverlayPoint(point.x + half, point.y + half, transform);
			const SlVec2 p3 = TransformOverlayPoint(point.x - half, point.y + half, transform);
			quad[0].pos = SlVec3(p0.x, p0.y, 0.0f);
			quad[1].pos = SlVec3(p1.x, p1.y, 0.0f);
			quad[2].pos = SlVec3(p2.x, p2.y, 0.0f);
			quad[3].pos = SlVec3(p3.x, p3.y, 0.0f);
			for (SlVertex2D& vertex : quad)
			{
				vertex.uv = SlVec2(0.0f, 0.0f);
				vertex.color = color;
			}
			vertices.insert(vertices.end(), quad, quad + 4);
			const unsigned short quadIndices[6] = { base, static_cast<unsigned short>(base + 1), static_cast<unsigned short>(base + 2),
				static_cast<unsigned short>(base + 2), static_cast<unsigned short>(base + 3), base };
			indices.insert(indices.end(), quadIndices, quadIndices + 6);
		}

		flush();
	}

	bool DrawAlphaContour(sl_d3d11::D3D11Renderer& renderer, const ReadSlotMeshData& meshData, const SlMatrix4& transform,
		float thickness, const SlColor& color, int viewportWidth, int viewportHeight)
	{
		if (meshData.textureHandle <= 0 || meshData.worldVertices.size() < 4 || meshData.uvs.size() < 4 || meshData.triangles.size() < 3)
			return false;

		LocalBounds bounds{};
		if (!FindVertexBounds(meshData, bounds))
			return false;

		constexpr int kPadding = 3;
		const int targetWidth = static_cast<int>(std::ceil(bounds.maxX - bounds.minX)) + kPadding * 2 + 1;
		const int targetHeight = static_cast<int>(std::ceil(bounds.maxY - bounds.minY)) + kPadding * 2 + 1;
		if (targetWidth <= 0 || targetHeight <= 0 || targetWidth > 4096 || targetHeight > 4096)
			return false;

		const int vertexCount = static_cast<int>(meshData.worldVertices.size() / 2);
		const int uvCount = static_cast<int>(meshData.uvs.size() / 2);
		if (vertexCount <= 0 || uvCount < vertexCount)
			return false;

		std::vector<SlVertex2D> maskVertices(static_cast<std::size_t>(vertexCount));
		for (int i = 0; i < vertexCount; ++i)
		{
			SlVertex2D& vertex = maskVertices[static_cast<std::size_t>(i)];
			vertex.pos = SlVec3(meshData.worldVertices[i * 2] - bounds.minX + static_cast<float>(kPadding),
				meshData.worldVertices[i * 2 + 1] - bounds.minY + static_cast<float>(kPadding), 0.0f);
			vertex.uv = SlVec2(meshData.uvs[i * 2], meshData.uvs[i * 2 + 1]);
			vertex.color = SlColor(1.0f, 1.0f, 1.0f, 1.0f);
		}

		std::vector<unsigned char> pixels;
		int stride = 0;
		if (!renderer.RasterizeToPixels(static_cast<SlTextureId>(meshData.textureHandle), maskVertices.data(), vertexCount,
			meshData.triangles.data(), static_cast<int>(meshData.triangles.size()), targetWidth, targetHeight, false, pixels, stride))
		{
			return false;
		}

		renderer.BeginFrame(viewportWidth, viewportHeight);

		constexpr int kAlphaThreshold = 16;
		auto alphaAt = [&](int x, int y) -> int
		{
			if (x < 0 || y < 0 || x >= targetWidth || y >= targetHeight)
				return 0;
			return pixels[static_cast<std::size_t>(y) * stride + static_cast<std::size_t>(x) * 4 + 3];
		};

		std::vector<SlVec2> edgePoints;
		edgePoints.reserve(static_cast<std::size_t>(targetWidth + targetHeight) * 4);
		for (int y = 0; y < targetHeight; ++y)
		{
			for (int x = 0; x < targetWidth; ++x)
			{
				if (alphaAt(x, y) <= kAlphaThreshold)
					continue;
				if (alphaAt(x - 1, y) > kAlphaThreshold &&
					alphaAt(x + 1, y) > kAlphaThreshold &&
					alphaAt(x, y - 1) > kAlphaThreshold &&
					alphaAt(x, y + 1) > kAlphaThreshold)
				{
					continue;
				}

				const float localX = static_cast<float>(x - kPadding) + bounds.minX;
				const float localY = static_cast<float>(y - kPadding) + bounds.minY;
				edgePoints.push_back(SlVec2(localX, localY));
			}
		}

		if (edgePoints.empty())
			return false;

		DrawTransformedEdgeDots(renderer, edgePoints, transform, thickness, color);
		return true;
	}
}

namespace sl_d3d11 {

void D3D11OverlayDrawer::SetTarget(D3D11Renderer* renderer, int width, int height) noexcept
{
	m_renderer = renderer;
	m_width = width;
	m_height = height;
}

void D3D11OverlayDrawer::SetStateRefreshCallback(OverlayStateRefreshFn callback) noexcept
{
	m_refreshCallback = callback;
}

void D3D11OverlayDrawer::DrawBox(const SlOverlayBox& box)
{
	if (!BeginOverlay())
		return;

	const SlColor color = ToD3D11OverlayColor(box.color);
	const float x1 = box.rect.x;
	const float y1 = box.rect.y;
	const float x2 = box.rect.x + box.rect.z;
	const float y2 = box.rect.y + box.rect.w;
	const SlVec2 p0 = TransformOverlayPoint(x1, y1, box.transform);
	const SlVec2 p1 = TransformOverlayPoint(x2, y1, box.transform);
	const SlVec2 p2 = TransformOverlayPoint(x2, y2, box.transform);
	const SlVec2 p3 = TransformOverlayPoint(x1, y2, box.transform);
	DrawD3D11Line(*m_renderer, p0, p1, box.thickness, color);
	DrawD3D11Line(*m_renderer, p1, p2, box.thickness, color);
	DrawD3D11Line(*m_renderer, p2, p3, box.thickness, color);
	DrawD3D11Line(*m_renderer, p3, p0, box.thickness, color);

	EndOverlay();
}

void D3D11OverlayDrawer::DrawReadSlotMeshOutline(const SlReadSlotMeshOutlineRequest& request)
{
	if (request.meshData == nullptr || request.meshData->worldVertices.size() < 4)
		return;
	if (!BeginOverlay())
		return;

	const auto& vertices = request.meshData->worldVertices;
	const int pointCount = static_cast<int>(vertices.size() / 2);
	if (pointCount < 2)
	{
		EndOverlay();
		return;
	}

	const SlColor color = ToD3D11OverlayColor(request.color);
	if (!DrawAlphaContour(*m_renderer, *request.meshData, request.transform, request.thickness, color, m_width, m_height) &&
		!DrawTriangleBoundary(*m_renderer, *request.meshData, request.transform, request.thickness, color))
	{
		DrawOrderedOutline(*m_renderer, *request.meshData, request.transform, request.thickness, color);
	}

	EndOverlay();
}

bool D3D11OverlayDrawer::BeginOverlay()
{
	if (m_renderer == nullptr || m_width <= 0 || m_height <= 0)
		return false;
	if (m_refreshCallback)
		m_refreshCallback();
	m_renderer->BeginFrame(m_width, m_height);
	return true;
}

void D3D11OverlayDrawer::EndOverlay()
{
	m_renderer->EndFrame();
	if (m_refreshCallback)
		m_refreshCallback();
}

D3D11OverlayDrawer& GetOverlayDrawer()
{
	static D3D11OverlayDrawer drawer;
	return drawer;
}

}
