#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
#include <sstream>
#include <cassert>

#include "hashmap.h"

std::string
to_string(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

int main() {
	hashmap<std::string, std::string> dict(10);

	std::unordered_map<std::string, std::string> bak;

	std::default_random_engine engine(10);

	const int key_space = 50;

	for (int i = 0; i < 1000; ++i) {
		auto key_int = engine() % key_space;
		auto key = to_string(key_int);
		auto value = to_string(engine());

		if (bak.find(key) != bak.end()) {
			assert(dict.contain(key));
			assert(dict.get(key) == bak.find(key)->second);
		} else {
			assert(!dict.contain(key));

			try {
				dict.get(key);
				assert(false);
			} catch (std::logic_error) {
			}

			try {
				dict.remove(key);
				assert(false);
			} catch (std::logic_error) {
			}
		}

		switch (engine() % 2) {
		case 0:
		{
			dict.set(key, value);

			bak.insert({ key, value }).first->second = value;

			assert(dict.contain(key));
			assert(dict.get(key) == value);
		}
		break;
		case 1:
		{
			if (bak.find(key) != bak.end()) {
				auto ret = dict.remove(key);

				assert(ret == bak.find(key)->second);
				assert(!dict.contain(key));

				try {
					dict.get(key);
					assert(false);
				} catch (std::logic_error) {
				}

				try {
					dict.remove(key);
					assert(false);
				} catch (std::logic_error) {
				}

				bak.erase(bak.find(key));
			}
		}
		break;
		}

	}

}
