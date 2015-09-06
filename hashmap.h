#pragma once

#include <cstdint>
#include <list>
#include <vector>
#include <functional>

template <typename K, typename V>
class hashmap {
public:
	hashmap(int capacity, double load_factor = 0.75);

	virtual ~hashmap();

	bool contain(const K &key);

	V& get(const K &key);

	void set(const K &key, const V &value);

	V remove(const K &key);

private:
	struct entry {
		K key;
		V value;
	};

	typedef std::vector<std::list<entry>> slots_t;

private:
	bool is_belongs_old(const K &key);

	void move_into_slots(slots_t &slots, entry &&e);

	entry& get_from_slots(slots_t &slots, const K &key);

	entry remove_from_slots(slots_t &slots, const K &key);

	void rehash_next_slot();

	void start_rehash();

	void finish_rehash();

private:
	slots_t current_slots;

	int total_entries;

	double load_factor;

	bool is_rehashing;

	slots_t old_slots;

	int remain_old_index;

	std::hash<K> hasher;
};

template<typename K, typename V>
hashmap<K, V>::hashmap(int capacity, double load_factor)
	:current_slots(capacity), total_entries(0), load_factor(load_factor), is_rehashing(false), remain_old_index(0) {}

template<typename K, typename V>
hashmap<K, V>::~hashmap() {}

template<typename K, typename V>
bool
hashmap<K, V>::contain(const K &key) {
	try {
		if (this->is_belongs_old(key)) {
			this->get_from_slots(this->old_slots, key);
		} else {
			this->get_from_slots(this->current_slots, key);
		}
		return true;
	} catch (std::logic_error) {
	}
	return false;
}

template<typename K, typename V>
V&
hashmap<K, V>::get(const K &key) {
	if (this->is_rehashing) {
		this->rehash_next_slot();
	}

	if (this->is_belongs_old(key)) {
		return this->get_from_slots(this->old_slots, key).value;
	} else {
		return this->get_from_slots(this->current_slots, key).value;
	}
}

template<typename K, typename V>
void
hashmap<K, V>::set(const K &key, const V &value) {
	if (this->is_rehashing) {
		this->rehash_next_slot();
	}

	if (this->is_belongs_old(key)) {
		this->move_into_slots(this->old_slots, { key, value });
	} else {
		this->move_into_slots(this->current_slots, { key, value });
	}

	if (!this->is_rehashing && this->total_entries >= this->current_slots.size() * this->load_factor) {
		this->start_rehash();
	}
}

template<typename K, typename V>
V
hashmap<K, V>::remove(const K &key) {
	if (this->is_rehashing) {
		this->rehash_next_slot();
	}

	if (this->is_belongs_old(key)) {
		return this->remove_from_slots(this->old_slots, key).value;
	} else {
		return this->remove_from_slots(this->current_slots, key).value;
	}
}

template<typename K, typename V>
bool
hashmap<K, V>::is_belongs_old(const K &key) {
	return this->is_rehashing && (int)(this->hasher(key) % this->old_slots.size()) >= this->remain_old_index;
}

template<typename K, typename V>
void
hashmap<K, V>::move_into_slots(slots_t &slots, entry &&e) {
	auto index = this->hasher(e.key) % slots.size();
	auto &slot = slots[index];
	for (auto &old : slot) {
		if (old.key == e.key) {
			old.value = std::move(e.value);
			return;
		}
	}
	slot.push_front(std::move(e));
	++this->total_entries;
}

template<typename K, typename V>
typename hashmap<K, V>::entry&
hashmap<K, V>::get_from_slots(slots_t &slots, const K &key) {
	auto index = this->hasher(key) % slots.size();
	auto &slot = slots[index];
	for (auto &e : slot) {
		if (key == e.key) {
			return e;
		}
	}
	throw std::logic_error("no entry for key");
}

template<typename K, typename V>
typename hashmap<K, V>::entry
hashmap<K, V>::remove_from_slots(slots_t &slots, const K &key) {
	auto index = this->hasher(key) % slots.size();
	auto &slot = slots[index];
	for (auto iter = slot.begin(); iter != slot.end(); ++iter) {
		if (key == iter->key) {
			auto old = std::move(*iter);
			slot.erase(iter);
			--this->total_entries;
			return old;
		}
	}
	throw std::logic_error("no entry for key");
}

template<typename K, typename V>
void hashmap<K, V>::
rehash_next_slot() {
	for (; this->remain_old_index < (int)this->old_slots.size(); ++this->remain_old_index) {
		if (!this->old_slots[this->remain_old_index].empty()) {
			break;
		}
	}

	if (this->remain_old_index < (int)this->old_slots.size()) {
		auto &slot = this->old_slots[this->remain_old_index];
		for (auto &e : slot) {
			this->move_into_slots(this->current_slots, std::move(e));
		}
		slot.clear();
		++this->remain_old_index;
	} else {
		this->finish_rehash();
	}

}

template<typename K, typename V>
void
hashmap<K, V>::start_rehash() {
	this->is_rehashing = true;
	this->old_slots = std::move(this->current_slots);
	this->remain_old_index = 0;

	this->current_slots.resize(this->total_entries * 2);
}

template<typename K, typename V>
void
hashmap<K, V>::finish_rehash() {
	this->is_rehashing = false;
	this->old_slots = slots_t();
	this->remain_old_index = 0;
}

