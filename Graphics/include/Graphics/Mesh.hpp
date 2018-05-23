#pragma once
#include <Graphics/VertexFormat.hpp>

namespace Graphics
{
	/* The type that tells how a mesh is drawn */
	enum class PrimitiveType
	{
		TriangleList = 0,
		TriangleStrip,
		TriangleFan,
		LineList,
		LineStrip,
		PointList,
	};

	/*
		Simple mesh object
	*/
	class MeshRes
	{
	public:
		MeshRes();
		~MeshRes();

		// Sets the vertex point data for this mesh
		// must be set before drawing
		// the vertex type must inherit from VertexFormat to automatically detect the correct format
		template <typename T>
		void SetData(const Vector<T>& verts)
		{
			SetData(verts.data(), verts.size(), T::GetDescriptors());
		}

		// Sets how the point data is interpreted and drawn
		// must be set before drawing
		void SetPrimitiveType(PrimitiveType pt);
		PrimitiveType get_primitive_type() const;

		// Draws the mesh
		void Draw() const;

		// Draws the mesh after if has already been drawn once, reuse of bound objects
		void Redraw() const;

	private:
		uint32 m_buffer = 0;
		uint32 m_vao = 0;
		PrimitiveType m_type;
		uint32 m_glType;
		size_t m_vertexCount;
		bool m_bDynamic = true;

		void SetData(const void* pData, size_t vertexCount, const VertexFormatList& desc);
	};

	typedef shared_ptr<MeshRes> Mesh;
}
