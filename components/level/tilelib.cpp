#include "tilelib.h"

#include <faio\faio.h>
#include <render\render.h>
#include <assert.h>

#include <iostream>

namespace Level
{
	template<typename T> struct TileData
	{
		T* data;
		size_t count;

		TileData(size_t size, FAIO::FAFile* file)
		{
			count = size;
			data = new T[size];

			FAIO::FAfread(data,sizeof(T),count,file);
		}

		~TileData()
		{
			delete data;
		}
	};

	struct CELEntry
	{
		uint8_t* data;
		size_t size;
	};

	struct CELFile
	{
		uint32_t count;
		CELEntry tiles[1];
	};

	void renderTile(CELEntry* cel, const Cel::Pal& Pal, size_t type, Cel::Colour dst[], int dx, int dy);

	TileLib::TileLib(const std::string& Library, const Cel::Pal& Palette, size_t MinEntrySize)
	{
		std::cout << "Loading tile library: " << Library << std::endl;
		FAIO::FAFile* min = FAIO::FAfopen(Library + ".min");
		size_t min_size = FAIO::FAsize(min);
		assert(min_size % (MinEntrySize * 2) == 0);
		TileData<uint16_t> min_data(min_size / (MinEntrySize * 2),min);
		FAIO::FAfclose(min);

		FAIO::FAFile* sol = FAIO::FAfopen(Library + ".sol");
		size_t sol_size = FAIO::FAsize(sol);
		TileData<uint8_t> sol_data(sol_size,sol);
		FAIO::FAfclose(sol);

		FAIO::FAFile* til = FAIO::FAfopen(Library + ".til");
		size_t til_size = FAIO::FAsize(til);
		assert(til_size % 8 == 0);
		TileData<uint16_t[4]> til_data(til_size / 8,til);
		FAIO::FAfclose(til);

		FAIO::FAFile* cel = FAIO::FAfopen(Library + ".cel");
		size_t cel_size = FAIO::FAsize(cel);
		assert(cel_size >= sizeof(uint32_t));
		uint32_t cel_count = FAIO::read32(cel);
		assert(cel_size >= sizeof(uint32_t) + cel_count * sizeof(uint32_t));
		CELFile* cel_data = (CELFile*)malloc(cel_size + cel_count * sizeof(CELEntry));
		cel_data->count = cel_count;
		uint32_t* cel_frames = reinterpret_cast<uint32_t*>(&cel_data->tiles[cel_count]);
		FAIO::FAfread(cel_frames,sizeof(uint8_t),cel_size - sizeof(uint32_t),cel);
		FAIO::FAfclose(cel);

		assert(cel_frames[cel_count] == cel_size);

		/*
		FAIO::FAFile* amp = FAIO::FAfopen(Library + ".amp");
		size_t amp_size = FAIO::FAsize(amp);
		assert(amp_size % 2 == 0);
		TileData<uint16_t> amp_data(amp_size / 2,amp);
		FAIO::FAfclose(amp);
		*/

		uint8_t* cel_base = reinterpret_cast<uint8_t*>(&cel_frames[-1]);
		for(size_t i = 0; i < cel_data->count; i++)
		{
			cel_data->tiles[i].data = cel_base + cel_frames[i];
			cel_data->tiles[i].size = cel_frames[i + 1] - cel_frames[i];
		}

		tiles.resize(til_data.count);

		std::cout << "Tile library size: " << til_data.count << std::endl;
		Cel::Colour* dst = new Cel::Colour[320 * 320];
		for(size_t i = 0; i < til_data.count; i++)
		{
			Tile& tile = tiles[i];

			const int dx[] = { 0, 32, -32, 0 };
			const int dy[] = { 0, 16, 16, 32 };

			size_t w = 0, h = 0;
			std::cout << "Tile: " << i << std::endl;
			for(size_t quad = 0; quad < 4; quad++)
			{
				size_t til_id = til_data.data[i][quad];
				tile.sol_data[quad] = sol_data.data[til_id];
				//tile.amp_data = amp_data.data[i];
				
				//need to tally up w & h
				for(size_t c = 0, min_offset = til_id * MinEntrySize; c < MinEntrySize; c++)
				{
					size_t cel_index = min_data.data[min_offset + c];
					if(cel_index != 0)
					{
						size_t cel_type = cel_index >> 12;
						cel_index = (cel_index & 0x0FFF) - 1;
						size_t x = (c & 1) ? dx[quad] + 32 : dx[quad];
						size_t y = dy[quad] - 32 * ((MinEntrySize - c - 1) >> 1);
						renderTile(&cel_data->tiles[cel_index],Palette,cel_type,dst,x,y);
					}
				}
			}

			w = 320;
			h = 320;
			tile.texture = Render::createTexture(dst,w,h);
		}

		delete [] dst;
		free(cel_data);
	}

	TileLib::~TileLib()
	{
	}

	const Tile& TileLib::getTile(size_t TileIndex) const
	{
		assert(TileIndex < tiles.size());
		return tiles[TileIndex];
	}

	//The following is taken from Ulmo's ViewTile (slightly modified for Freeablo)
	int decode_tile_0(CELEntry* cel, const Cel::Pal& Pal, Cel::Colour* dst) 
	{
		int i = 0;
		for (int y = 31; y >= 0; --y)
			for (int x = 0; x < 32; ++x)
				dst[x + 32 * y] = Pal[cel->data[i++]];

	   return 1;
	}

	int decode_tile_1(CELEntry* cel, const Cel::Pal& Pal, Cel::Colour* dst) 
	{
		int nb_pix, j;
		size_t i = 0;
		int x = 0, y = 31;
		while(y >= 0)
		{
			if(i >= cel->size)
			{
				std::cout << "too few data for cel (type 1) a !" << std::endl;
				return 0;
			}

			if(cel->data[i] >= 0x80)
			{
				nb_pix = 0x100 - cel->data[i];
				x += nb_pix;
				++i;
			}
			else 
			{
				if(i >= cel->size) 
				{
					std::cout << "too few data for cel (type 1) b !" << std::endl;
					return 0;
				}

				nb_pix = cel->data[i++];
				for(j = 0; j < nb_pix; ++j) 
				{
					if (i >= cel->size) 
					{
						std::cout << "too few data for cel (type 1) c !" << std::endl;
						return 0;
					}
				
					dst[x++ + 32 * y] = Pal[cel->data[i++]];
					if (x > 31)
					{
						x = 0;
						--y;
					}
				}
			}

			if (x > 31) 
			{
				x = 0;
				--y;
			}
		}

		return 1;
	}

	int decode_tile_2(CELEntry* cel, const Cel::Pal& Pal, Cel::Colour* dst) 
	{
		const int line_size[32] = { 0, 4, 4, 8, 8, 12, 12, 16, 16, 20, 20, 24, 24, 28, 28, 32, 32, 32, 28, 28, 24, 24, 20, 20, 16, 16, 12, 12, 8, 8, 4, 4 };

		int i = 0;
		for (int y = 31; y >= 0; y--) 
		{
			for (int x = 32 - line_size[y]; x < 32; x++) 
			{
				dst[x + 32 * y] = Pal[cel->data[i++]];
			}
		}

		return 1;
	}

	int decode_tile_3(CELEntry* cel, const Cel::Pal& Pal, Cel::Colour* dst) 
	{
		const int line_size[32] = { 0, 4, 4, 8, 8, 12, 12, 16, 16, 20, 20, 24, 24, 28, 28, 32, 32, 32, 28, 28, 24, 24, 20, 20, 16, 16, 12, 12, 8, 8, 4, 4 };

		int i = 0;
		for(int y = 31; y >= 0; --y)
		{
			for(int x = 0; x < line_size[y]; ++x)
			{
				dst[x + 32 * y] = Pal[cel->data[i++]];
			}
		}

		return 1;
	}

	int decode_tile_4(CELEntry* cel, const Cel::Pal& Pal, Cel::Colour* dst) 
	{
		const int line_size[32] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 , 28, 28, 24, 24, 20, 20, 16, 16, 12, 12, 8, 8, 4, 4 };

		int i = 0;
		for(int y = 31; y >= 0; --y) 
		{
			for(int x = 32 - line_size[y]; x < 32; ++x) 
			{
				dst[x + 32 * y] = Pal[cel->data[i++]];
			}
		}

		return 1;
	}

	int decode_tile_5(CELEntry* cel, const Cel::Pal& Pal, Cel::Colour* dst) 
	{
		const int line_size[32] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 , 28, 28, 24, 24, 20, 20, 16, 16, 12, 12, 8, 8, 4, 4 };

		int i = 0;
		for(int y = 31; y >= 0; --y) 
		{
			for(int x = 0; x < line_size[y]; ++x) 
			{
				dst[x + 32 * y] = Pal[cel->data[i++]];
			}
		}

		return 1;
	}

	void renderTile(CELEntry* cel, const Cel::Pal& Pal, size_t type, Cel::Colour dst[], int dx, int dy)
	{
		std::cout << "Decoding using method: " << type << "<" << dx << "," << dy << ">" << std::endl;
		assert(type < 6);

		Cel::Colour dst_tmp[32 * 32];
		for(size_t i = 0; i < 32 * 32; i++)
			dst_tmp[i].visible = false;

		int ok = 0;
		switch(type)
		{
			case 0: ok = decode_tile_0(cel,Pal,dst_tmp); break;
			case 1: ok = decode_tile_1(cel,Pal,dst_tmp); break;
			case 2: ok = decode_tile_2(cel,Pal,dst_tmp); break;
			case 3: ok = decode_tile_3(cel,Pal,dst_tmp); break;
			case 4: ok = decode_tile_4(cel,Pal,dst_tmp); break;
			case 5: ok = decode_tile_5(cel,Pal,dst_tmp); break;
		}

		assert(ok != 0);

		//Render::createTexture(dst_tmp,32,32);

		Cel::Colour* st = &dst[(dy * (320)) + dx];
		for(int y = 0; y < 32; y++)
		{
			for(int x = 0; x < 32; x++)
			{
				auto c = dst_tmp[y * 32 + x];
				if(c.visible)
					st[(y * (320)) + x] = c;
			}
		}
	}
}