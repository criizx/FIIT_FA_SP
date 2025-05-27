#include "../include/allocator_buddies_system.h"

#include <not_implemented.h>

#include <cstddef>
#include <cstdint>

inline size_t allocator_buddies_system::pow_2(size_t exp) const noexcept { return 1ULL << exp; }

bool allocator_buddies_system::is_busy(void *block_ptr) const noexcept {
	return reinterpret_cast<block_metadata *>(block_ptr)->occupied;
}

byte allocator_buddies_system::degree_of_2(size_t value) const noexcept {
	size_t base = 1;
	byte power = 0;
	while (base < value) {
		base <<= 1;
		++power;
	}
	return power;
}

std::pmr::memory_resource *allocator_buddies_system::get_memory_resource() const {
	auto offset = sizeof(std::mutex) + sizeof(logger *);
	return *reinterpret_cast<std::pmr::memory_resource **>(reinterpret_cast<byte *>(_trusted_memory) + offset);
}

logger *allocator_buddies_system::get_logger() const {
	auto offset = sizeof(std::mutex);
	return *reinterpret_cast<logger **>(reinterpret_cast<byte *>(_trusted_memory) + offset);
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const {
	if (!_trusted_memory) {
		throw std::runtime_error("_trusted_memory is nullptr");
	}

	std::vector<allocator_test_utils::block_info> result;

	byte *cursor = reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size;
	byte *limit = cursor + get_allocator_pool_size();

	while (cursor < limit) {
		auto *meta = reinterpret_cast<block_metadata *>(cursor);
		result.emplace_back(pow_2(meta->size), meta->occupied);
		cursor += pow_2(meta->size);
	}

	return result;
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept {
	return get_blocks_info_inner();
}

allocator_with_fit_mode::fit_mode &allocator_buddies_system::get_fit_mode() const noexcept {
	auto offset = sizeof(std::mutex) + sizeof(logger *) + sizeof(std::pmr::memory_resource *);
	return *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<byte *>(_trusted_memory) + offset);
}

std::string allocator_buddies_system::get_typename() const { return "allocator_buddies_system"; }

void *allocator_buddies_system::get_block_twin(void *origin) noexcept {
	size_t relative_pos =
	    reinterpret_cast<byte *>(origin) - (reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size);
	size_t twin_offset = relative_pos ^ pow_2(degree_of_2(get_block_size(origin)));
	return reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size + twin_offset;
}

size_t allocator_buddies_system::get_block_size(void *block) const noexcept {
	return pow_2(reinterpret_cast<block_metadata *>(block)->size);
}

std::mutex &allocator_buddies_system::get_mutex() const noexcept {
	return *reinterpret_cast<std::mutex *>(_trusted_memory);
}

allocator_with_fit_mode::fit_mode &allocator_buddies_system::get_fit_mod() const noexcept {
	auto offset = sizeof(logger *) + sizeof(allocator_dbg_helper *);
	return *reinterpret_cast<fit_mode *>(reinterpret_cast<byte *>(_trusted_memory) + offset);
}

size_t allocator_buddies_system::get_allocator_pool_size() const noexcept {
	auto base_ptr = reinterpret_cast<byte *>(_trusted_memory);
	auto pool_size_byte =
	    *(base_ptr + sizeof(std::mutex) + sizeof(logger *) + sizeof(std::pmr::memory_resource *) + sizeof(fit_mode));
	return pow_2(static_cast<size_t>(pool_size_byte));
}

size_t allocator_buddies_system::get_size_memory() const noexcept { return get_allocator_pool_size(); }

void *allocator_buddies_system::find_first_suitable_block(size_t request_size) const noexcept {
	byte *start = reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size;
	byte *finish = start + get_size_memory();

	for (byte *block = start; block < finish; block += get_block_size(block)) {
		if (!is_busy(block) && (get_block_size(block) - occupied_block_metadata_size) >= request_size) {
			return block;
		}
	}
	return nullptr;
}

void *allocator_buddies_system::find_worst_suitable_block(size_t request_size) const noexcept {
	byte *start = reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size;
	byte *end = start + get_size_memory();

	void *largest_block = nullptr;
	size_t largest_size = 0;

	for (byte *current = start; current < end; current += get_block_size(current)) {
		size_t space = get_block_size(current) - occupied_block_metadata_size;
		if (!is_busy(current) && space >= request_size && space > largest_size) {
			largest_block = current;
			largest_size = space;
		}
	}
	return largest_block;
}

void *allocator_buddies_system::find_best_suitable_block(size_t request_size) const noexcept {
	byte *start = reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size;
	byte *end = start + get_size_memory();

	void *smallest_fit = nullptr;
	size_t best_fit_size = SIZE_MAX;

	for (byte *current = start; current < end; current += get_block_size(current)) {
		size_t usable = get_block_size(current) - occupied_block_metadata_size;
		if (!is_busy(current) && usable >= request_size && usable < best_fit_size) {
			best_fit_size = usable;
			smallest_fit = current;
		}
	}
	return smallest_fit;
}

void allocator_buddies_system::split_block(void *target, byte min_power) {
	auto *meta = reinterpret_cast<block_metadata *>(target);
	while (meta->size > min_power) {
		--meta->size;
		void *pair = get_block_twin(target);
		auto *pair_meta = reinterpret_cast<block_metadata *>(pair);
		pair_meta->occupied = false;
		pair_meta->size = meta->size;
	}
}

allocator_buddies_system::~allocator_buddies_system() {
	debug_with_guard("~allocator_buddies_system invoked");
	auto resource = get_memory_resource();

	if (!resource) {
		get_mutex().~mutex();
		::operator delete(_trusted_memory);
	} else {
		resource->deallocate(_trusted_memory, get_size_memory() + allocator_metadata_size, alignof(std::max_align_t));
	}
	debug_with_guard("~allocator_buddies_system complete");
}

allocator_buddies_system::allocator_buddies_system(allocator_buddies_system &&source) noexcept
    : smart_mem_resource(std::move(source)),
      allocator_test_utils(source),
      allocator_with_fit_mode(std::move(source)),
      logger_guardant(source),
      typename_holder(source),
      _trusted_memory(source._trusted_memory) {
	debug_with_guard("Move ctor invoked");
	source._trusted_memory = nullptr;
}

allocator_buddies_system &allocator_buddies_system::operator=(allocator_buddies_system &&source) noexcept {
	debug_with_guard("Move assignment start");
	if (this != &source) {
		smart_mem_resource::operator=(std::move(source));
		allocator_test_utils::operator=(source);
		allocator_with_fit_mode::operator=(std::move(source));
		logger_guardant::operator=(source);
		typename_holder::operator=(source);

		_trusted_memory = source._trusted_memory;
		source._trusted_memory = nullptr;
	}
	debug_with_guard("Move assignment complete");
	return *this;
}

allocator_buddies_system::allocator_buddies_system(size_t requested, std::pmr::memory_resource *upstream,
                                                   logger *log_inst, allocator_with_fit_mode::fit_mode mode) {
	byte power = degree_of_2(requested);
	if (power < min_k) {
		throw std::invalid_argument("Minimum size is " + std::to_string(min_k));
	}

	size_t allocation_size = pow_2(power) + allocator_metadata_size;

	if (!upstream) {
		_trusted_memory = ::operator new(allocation_size);
	} else {
		_trusted_memory = upstream->allocate(allocation_size, 1);
	}

	void *mem = _trusted_memory;
	new (mem) std::mutex;
	mem = static_cast<byte *>(mem) + sizeof(std::mutex);

	*reinterpret_cast<logger **>(mem) = log_inst;
	mem = static_cast<byte *>(mem) + sizeof(logger *);

	*reinterpret_cast<std::pmr::memory_resource **>(mem) = upstream;
	mem = static_cast<byte *>(mem) + sizeof(std::pmr::memory_resource *);

	*reinterpret_cast<fit_mode *>(mem) = mode;
	mem = static_cast<byte *>(mem) + sizeof(fit_mode);

	*reinterpret_cast<byte *>(mem) = power;
	mem = static_cast<byte *>(mem) + sizeof(byte);

	auto *meta = reinterpret_cast<block_metadata *>(mem);
	meta->occupied = false;
	meta->size = power;
}

void *allocator_buddies_system::do_allocate_sm(size_t size) {
	debug_with_guard("do_allocate_sm start");

	size_t total_size = size + occupied_block_metadata_size;
	byte power = degree_of_2(total_size);

	void *block = nullptr;
	auto policy = get_fit_mode();

	if (policy == fit_mode::first_fit)
		block = find_first_suitable_block(power);
	else if (policy == fit_mode::the_best_fit)
		block = find_best_suitable_block(power);
	else if (policy == fit_mode::the_worst_fit)
		block = find_worst_suitable_block(power);

	if (!block) throw std::bad_alloc();

	split_block(block, power);

	auto *meta = reinterpret_cast<block_metadata *>(block);
	meta->occupied = true;

	*reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + sizeof(block_metadata)) = _trusted_memory;
	debug_with_guard("do_allocate_sm end");
	return reinterpret_cast<byte *>(block) + occupied_block_metadata_size;
}

void allocator_buddies_system::do_deallocate_sm(void *ptr) {
	std::lock_guard lock(get_mutex());
	debug_with_guard("do_deallocate_sm");

	void *block = reinterpret_cast<byte *>(ptr) - occupied_block_metadata_size;

	if (*reinterpret_cast<void **>(reinterpret_cast<byte *>(block) + sizeof(block_metadata)) != _trusted_memory) {
		throw std::runtime_error("Mismatched allocator pointer");
	}

	reinterpret_cast<block_metadata *>(block)->occupied = false;

	void *twin = get_block_twin(block);

	while (get_block_size(block) < get_size_memory() && get_block_size(block) == get_block_size(twin) &&
	       !is_busy(twin)) {
		block = std::min(block, twin);
		auto *meta = reinterpret_cast<block_metadata *>(block);
		++meta->size;
		twin = get_block_twin(block);
	}
	debug_with_guard("do_deallocate_sm complete");
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
	auto *casted = dynamic_cast<const allocator_buddies_system *>(&other);
	return casted && (_trusted_memory == casted->_trusted_memory);
}

void allocator_buddies_system::set_fit_mode(fit_mode mode) { get_fit_mod() = mode; }

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept {
	return buddy_iterator(reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size);
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept {
	return buddy_iterator(reinterpret_cast<byte *>(_trusted_memory) + allocator_metadata_size + get_size_memory());
}

bool allocator_buddies_system::buddy_iterator::operator==(const buddy_iterator &rhs) const noexcept {
	return _block == rhs._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const buddy_iterator &rhs) const noexcept {
	return !(*this == rhs);
}

allocator_buddies_system::buddy_iterator &allocator_buddies_system::buddy_iterator::operator++() noexcept {
	_block = reinterpret_cast<byte *>(_block) + (1 << reinterpret_cast<block_metadata *>(_block)->size);
	return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int) {
	auto copy = *this;
	++(*this);
	return copy;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept {
	return reinterpret_cast<block_metadata *>(_block)->size;
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept {
	return reinterpret_cast<block_metadata *>(_block)->occupied;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept { return _block; }

allocator_buddies_system::buddy_iterator::buddy_iterator(void *ptr) : _block(ptr) {}
allocator_buddies_system::buddy_iterator::buddy_iterator() : _block(nullptr) {}
