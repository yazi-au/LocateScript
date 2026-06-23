#include "LocateAI.hpp"
#include "Utils.hpp"
#include <deque>
#include <windows.h>

namespace LocateScript {
	Vec2i AABBbox::getCenter() {
		return Vec2i((point1.x + point2.x) / 2, (point1.y + point2.y) / 2);
	}

	void StreamBoxFactory::resize(uint32_t& x, uint32_t& y) {
		x = x * screenSize.x / 1000;
		y = y * screenSize.y / 1000;
	}

	void StreamBoxFactory::add(std::string str) {
		std::deque<Token> tokens;
		for (Token& temp : tokenBuffer) {
			tokens.push_back(temp);
		}
		tokenBuffer.clear();

		for (size_t i = 0; i < str.size(); i++) {
			char c = str.data()[i];
			switch (c) {
				case '<': {
					std::string s = std::string(buffer.begin(), buffer.end());
					if (!s.empty()) tokens.push_back(Token(TokenType::NAME, s));
					buffer.clear();
					break;
				}
				case '>': {
					std::string s = std::string(buffer.begin(), buffer.end());
					if (s == "ref" || s == "/ref" || s == "box" || s == "/box") {
						buffer.clear();
						break;
					}
					tokens.push_back(Token(TokenType::INTEGER, atoi(s.c_str())));
					buffer.clear();
					break;
				}
				default: {
					buffer.push_back(c);
					break;
				}
			}
		}

		while (!tokens.empty()) {
			Token& token = tokens.front();
			if (token.type == TokenType::NAME) {
				lastName = token.name;
				tokens.pop_front();
			} else {
				if (tokens.size() < 4) {
					break;
				}
				uint32_t values[4];
				for (uint32_t& v : values) {
					v = tokens.front().value;
					tokens.pop_front();
				}
				resize(values[0], values[1]);
				resize(values[2], values[3]);
				boxes.push_back(AABBbox(lastName, values[0], values[1], values[2], values[3]));
			}
		}

		for (Token& token : tokens) {
			tokenBuffer.push_back(token);
		}
	}

	std::vector<AABBbox>* StreamBoxFactory::getBoxes() {
		return &boxes;
	}

	StreamBoxFactory::StreamBoxFactory() {
		screenSize = Vec2i((uint32_t)GetSystemMetrics(SM_CXSCREEN), (uint32_t)GetSystemMetrics(SM_CYSCREEN));
	}
}