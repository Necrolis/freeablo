#pragma once

#include <string>
#include <vector>
#include <stdint.h>

#include <cel\pal.h>

namespace Level
{
	enum IsoQuadrants
	{
		Top,
		Right,
		Left,
		Bottom,

		Count
	};

	enum TileTypeFlags
	{
		DoorLeft	= 0x01,
		DoorRight	= 0x02,
		ArchLeft	= 0x04,
		ArchRight	= 0x08,
		GrillLeft	= 0x10,
		GrillRight	= 0x20,
		Stairs		= 0x40,
		Roof		= 0x80
	};

	struct TileLibRecord
	{
		std::string path;
		size_t min_size;

		//we can't include the palette paths here because the are 6 differing palette types for each level
	};

	static const TileLibRecord TileLibraries[] =
	{
		{"levels\\towndata", 16},
		{"levels\\l1data", 10},
		{"levels\\l2data", 10},
		{"levels\\l3data", 10},
		{"levels\\l4data", 16},
		{"nlevels\\l5data", 10},
		{"nlevels\\l6data", 10},
		{"nlevels\\towndata", 16}
	};

	class Tile
	{
		friend class TileLib;

		private:
			void* texture;
			union
			{
				struct
				{
					uint8_t type;
					uint8_t orientation;
				};

				uint16_t amp_data;
			};

			uint8_t sol_data[IsoQuadrants::Count];

		public:
			Tile()
			{
			}

			~Tile()
			{
			}
	};

	class TileLib
	{
		private:
			std::vector<Tile> tiles;

		public:
			TileLib(const std::string& Library, const Cel::Pal& Palette, size_t MinEntrySize);
			~TileLib();

			const Tile& getTile(size_t TileIndex) const;

			const Tile& TileLib::operator[](size_t TileIndex) const
			{
				return getTile(TileIndex);
			}
	};
}