#include "LocateAI.hpp"
#include "Utils.hpp"
#include <deque>

namespace LocateScript {
	Vec2i AABBbox::getCenter() {
		return Vec2i((point1.x + point2.x) / 2, (point1.y + point2.y) / 2);
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
				int values[4];
				for (int& v : values) {
					v = tokens.front().value;
					tokens.pop_front();
				}
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
}