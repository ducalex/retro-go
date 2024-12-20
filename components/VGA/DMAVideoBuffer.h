
#include <hal/dma_types.h>
#include <rom/cache.h>
#include <esp_heap_caps.h>

extern int Cache_WriteBack_Addr(uint32_t addr, uint32_t size);

class DMAVideoBuffer
{
	protected:
	int	descriptorCount;
	dma_descriptor_t *descriptors;
	static const int MAX_BUFFERS = 2;	//need to malloc this
	static const int MAX_LINES = 1024;	//need to malloc this
	void *buffer[MAX_BUFFERS][MAX_LINES];

	public:
	int bufferCount;
	bool valid;
	int lines;
	int lineSize;
	bool psram;
	int clones;

	static const int MAX_DMA_BLOCK_SIZE = 4092;
	static const int ALIGNMENT_PSRAM = 64;
	static const int ALIGNMENT_SRAM = 4;

	void attachBuffer(int b = 0)
	{
		for(int i = 0; i < lines; i++)
			for(int j = 0; j < clones; j++)
				descriptors[i * clones + j].buffer = buffer[b][i];
	}

	uint8_t *getLineAddr8(int y ,int b = 0)
	{
		return (uint8_t*)buffer[b][y];
	}


	uint16_t *getLineAddr16(int y ,int b = 0)
	{
		return (uint16_t*)buffer[b][y];
	}

	DMAVideoBuffer(int lines, int lineSize, int clones = 1, bool ring = true, bool psram = true, int bufferCount = 1)
	{
		this->lineSize = lineSize;
		this->psram = psram;
		this->bufferCount = bufferCount;
		this->lines = lines;
		this->clones = clones;
		valid = false;
		descriptorCount = lines * clones; //assume we dont need more than 4095 bytes per line

		descriptors = (dma_descriptor_t *)heap_caps_aligned_calloc(ALIGNMENT_SRAM, 1, descriptorCount * sizeof(dma_descriptor_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
		if (!descriptors)
			return;
		for(int i = 0; i < bufferCount; i++)
			for(int j = 0; j < lines; j++)
				buffer[i][j] = 0;
		for(int i = 0; i < bufferCount; i++)
		{
			for(int y = 0; y < lines; y++)
			{
				if(psram)
					buffer[i][y] = heap_caps_aligned_calloc(ALIGNMENT_PSRAM, 1, lineSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
				else
					buffer[i][y] = heap_caps_aligned_calloc(ALIGNMENT_SRAM, 1, lineSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
				if (!buffer[i][y])
				{
					//TODO f this
					/*heap_caps_free(descriptors);
					descriptors = 0;
					for(int j = 0; j < i * lines + y; j++)
					{
						if(buffer[j])
							heap_caps_free(buffer[j]);
						buffer[j] = 0;
					}*/
					return;
				}
			}
		}
		for (int i = 0; i < descriptorCount; i++)
		{
			descriptors[i].dw0.owner = DMA_DESCRIPTOR_BUFFER_OWNER_CPU;
			descriptors[i].dw0.suc_eof = 0;
			descriptors[i].next = &descriptors[i + 1];
			descriptors[i].dw0.size = lineSize;
			descriptors[i].dw0.length = descriptors[i].dw0.size;
		}
		attachBuffer(0);
		if(ring)
			descriptors[descriptorCount - 1].next = descriptors;
		else
		{
			descriptors[descriptorCount - 1].dw0.suc_eof = 1;
			descriptors[descriptorCount - 1].next = 0;
		}
		valid = true;
	}

	~DMAVideoBuffer()
	{
		if(descriptors)
			heap_caps_free(descriptors);
		for(int i = 0; i < bufferCount; i++)
			for(int j = 0; j < lines; j++)
				if(buffer[i][j])
					heap_caps_free(buffer[i][j]);
	}

	dma_descriptor_t *getDescriptor(int i = 0) const
	{
		return &descriptors[0];
	}

	int getDescriptorCount() const
	{
		return descriptorCount;
	}

	int getLineSize() const
	{
		return lineSize;
	}

	void flush(int b = 0)
	{
		if(!psram) return;
		for(int y = 0; y < lines; y++)
			Cache_WriteBack_Addr((uint32_t)buffer[b][y], lineSize);
	}

	void flush(int b, int y)
	{
		if(!psram) return;
			Cache_WriteBack_Addr((uint32_t)buffer[b][y], lineSize);
	}

	/*void flush(int b, int start, int length)
	{
		if(!psram) return;
		Cache_WriteBack_Addr((uint32_t)start, length);
	}*/

	int getBufferCount() const
	{
		return bufferCount;
	}

	bool isValid() const
	{
		return valid;
	}
};