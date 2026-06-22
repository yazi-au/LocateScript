#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace LocateScript {
	class Vec2i {
	public:
		uint32_t x, y;

		Vec2i(uint32_t x, uint32_t y) : x(x), y(y) {}
	};

	class AABBbox {
	public:
		std::string name;
		Vec2i point1, point2;

		AABBbox(std::string name, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) : name(name), point1(x1, y1), point2(x2, y2) {}
		Vec2i getCenter();
	};

	enum class TokenType {
		NAME,
		INTEGER
	};

	class Token {
	public:
		TokenType type;
		uint32_t value;
		std::string name;
		Token(TokenType type, uint32_t value) : type(type), value(value) {}
		Token(TokenType type, std::string name) : type(type), name(name) {}
		Token(TokenType type) : type(type) {}
	};

	class StreamBoxFactory {
		std::vector<AABBbox> boxes;
		std::vector<Token> tokenBuffer;
		std::vector<char> buffer;
		std::string lastName;
	public:
		void add(std::string str);
		std::vector<AABBbox>* getBoxes();
	};
}