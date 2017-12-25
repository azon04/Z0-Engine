#include "../ZEngine.h"

#include "BufferManager.h"
#include "BufferLayout.h"

namespace ZE {

	BufferManager* BufferManager::m_instance = NULL;

	BufferManager::BufferManager()
	{
	}

	ZE::BufferManager* BufferManager::getInstance()
	{
		return m_instance;
	}

	void BufferManager::Init(GameContext* _gameContext)
	{
		Handle hBufferManager("Buffer Manager", sizeof(BufferManager));
		m_instance = new(hBufferManager) BufferManager();

		BufferLayoutManager::Init();
		m_instance->m_bufferLayoutManager = BufferLayoutManager::getInstance();

		// Create sample vertex Color buffer
		{
			Handle handle("Data Triangle", sizeof(float) * 18);
			handle.getObject();
			float* vertices_color = new(handle) float[18]{
				// Positions		// Colors			
				0.5, -0.5f, 0.0f,	1.0f, 0.0f, 0.0f, // Bottom Right
				-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // Bottom Left
				0.0f, 0.5f, 0.0f,	0.0f, 0.0f, 1.0f	// Top
			};

			Handle hBufferData("Buffer Data", sizeof(BufferData));
			BufferData* bufferData = new(hBufferData) BufferData(BufferType::VERTEX_BUFFER);
			bufferData->SetData(vertices_color, 18 * sizeof(float));
			bufferData->m_bufferLayout = BUFFER_LAYOUT_V3_C3;

			getInstance()->m_buffers.push_back(bufferData);
			getInstance()->createBufferArray(bufferData, nullptr, nullptr);
		}

		// Create Cube
		{
			Handle handle("Cube Data", sizeof(float) * 288);
			float* cube_vertices = new(handle) float[288]
			{
				// positions			// colors				// texture coords
				-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
				0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
				0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
				0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
				-0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 0.0f,		0.0f, 0.0f,

				-0.5f, -0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,
				0.5f, -0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
				0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
				0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
				-0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
				-0.5f, -0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,

				-0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f,
				-0.5f, 0.5f, -0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
				-0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,
				-0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f,

				0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f,
				0.5f, 0.5f, -0.5f,		1.0f, 1.0f, 0.0f,		1.0f, 1.0f,
				0.5f, -0.5f, -0.5f,		1.0f, 1.0f, 0.0f,		0.0f, 1.0f,
				0.5f, -0.5f, -0.5f,		1.0f, 1.0f, 0.0f,		0.0f, 1.0f,
				0.5f, -0.5f, 0.5f,		1.0f, 1.0f, 0.0f,		0.0f, 0.0f,
				0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f,

				-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 1.0f,		0.0f, 1.0f,
				0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f,
				0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 1.0f,		1.0f, 0.0f,
				0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 1.0f,		1.0f, 0.0f,
				-0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 1.0f,		0.0f, 0.0f,
				-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 1.0f,		0.0f, 1.0f,

				-0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 1.0f,		0.0f, 1.0f,
				0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 1.0f,		1.0f, 1.0f,
				0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f,
				0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f,
				-0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f,
				-0.5f, 0.5f, -0.5f,		0.0f, 1.0f, 1.0f,		0.0f, 1.0f
			};

			Handle hBufferData("CubeBuffer Data", sizeof(BufferData));
			BufferData* bufferData = new(hBufferData) BufferData(BufferType::VERTEX_BUFFER);
			bufferData->SetData(cube_vertices, 288 * sizeof(float));
			bufferData->m_bufferLayout = BUFFER_LAYOUT_V3_C3_TC2;

			getInstance()->m_buffers.push_back(bufferData);
			getInstance()->createBufferArray(bufferData, nullptr, nullptr);
		}

		// Create Basis vertices
		{
			Handle handle("Basis Data", sizeof(float) * 36);
			float* basis_data = new(handle) float[36]
			{
				// positions		// color
				0.0f, 0.0f, 0.0f,	0.0f, 0.0f, 1.0f,
				0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 1.0f,

				0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

				0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f
			};

			Handle hBufferData("BasisBufferData", sizeof(BufferData));
			BufferData* bufferData = new(hBufferData) BufferData(BufferType::VERTEX_BUFFER);
			bufferData->SetData(basis_data, 36 * sizeof(float));
			bufferData->m_bufferLayout = BUFFER_LAYOUT_V3_C3;

			getInstance()->m_buffers.push_back(bufferData);
			getInstance()->createBufferArray(bufferData, nullptr, nullptr);
		}
	}

	void BufferManager::Destroy()
	{

	}

	ZE::GPUBufferData* BufferManager::createGPUBufferFromBuffer(BufferData* _bufferData, bool _bStatic, bool _manualManage)
	{
		if (!_bufferData) return nullptr;
		// #OPENGL Specific
		Handle handle("GPU Buffer Data", sizeof(GPUBufferData));
		GPUBufferData* GPUBuffer = new(handle) GPUBufferData(_bStatic);
		GPUBuffer->FromBufferData(_bufferData);

		if (!_manualManage)
		{
			m_GPUBuffers.push_back(GPUBuffer);
		}

		return GPUBuffer;
	}

	ZE::GPUBufferArray* BufferManager::createBufferArray(BufferData* _vertexBuffer, BufferData* _indexBuffer, BufferData* _gpuBuffer)
	{
		// #OPENGL Specific
		GPUBufferData* vertexBufferGPU = createGPUBufferFromBuffer(_vertexBuffer);
		GPUBufferData* indexBufferGPU = createGPUBufferFromBuffer(_indexBuffer);
		GPUBufferData* computeGPUBuffer = createGPUBufferFromBuffer(_gpuBuffer);
	
		Handle handle("GPU Buffer Array", sizeof(GPUBufferArray));
		GPUBufferArray* bufferArray = new(handle) GPUBufferArray();
		bufferArray->SetupBufferArray(vertexBufferGPU, indexBufferGPU, computeGPUBuffer);

		m_GPUBufferArrays.push_back(bufferArray);
		return bufferArray;
	}

}