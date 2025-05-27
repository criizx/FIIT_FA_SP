#include "../include/allocator_global_heap.h"

#include <new>

allocator_global_heap::allocator_global_heap(logger *logger) : _logger(logger) {
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::allocator_global_heap - begin", logger::severity::debug);
	}
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::allocator_global_heap - end", logger::severity::debug);
	}
}

allocator_global_heap::~allocator_global_heap() {
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::~allocator_global_heap - begin", logger::severity::debug);
	}
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::~allocator_global_heap - end", logger::severity::debug);
	}
}

allocator_global_heap::allocator_global_heap(const allocator_global_heap &other) : _logger(other._logger) {
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::allocator_global_heap(copy) - begin", logger::severity::debug);
	}
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::allocator_global_heap(copy) - end", logger::severity::debug);
	}
}

allocator_global_heap &allocator_global_heap::operator=(const allocator_global_heap &other) {
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::operator=(copy) - begin", logger::severity::debug);
	}
	if (this != &other) {
		_logger = other._logger;
	}
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::operator=(copy) - end", logger::severity::debug);
	}
	return *this;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept : _logger(other._logger) {
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::allocator_global_heap(move) - begin", logger::severity::debug);
	}
	other._logger = nullptr;
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::allocator_global_heap(move) - end", logger::severity::debug);
	}
}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept {
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::operator=(move) - begin", logger::severity::debug);
	}
	if (this != &other) {
		_logger = other._logger;
		other._logger = nullptr;
	}
	if (auto log = get_logger()) {
		log->log("allocator_global_heap::operator=(move) - end", logger::severity::debug);
	}
	return *this;
}

[[nodiscard]] void *allocator_global_heap::do_allocate_sm(size_t size) {
	if (auto log = get_logger()) {
		log->log("do_allocate_sm - begin (size=" + std::to_string(size) + ")", logger::severity::trace);
	}

	if (size == 0) {
		if (auto log = get_logger()) {
			log->log("do_allocate_sm - zero-size, returning nullptr", logger::severity::debug);
		}
		return nullptr;
	}

	try {
		void *ptr = ::operator new(size);
		if (auto log = get_logger()) {
			log->log("do_allocate_sm - allocated " + std::to_string(size) + " bytes at " +
			             std::to_string(reinterpret_cast<uintptr_t>(ptr)),
			         logger::severity::trace);
		}
		return ptr;
	} catch (const std::bad_alloc &e) {
		if (auto log = get_logger()) {
			log->log("do_allocate_sm - bad_alloc: " + std::string(e.what()), logger::severity::error);
		}
		throw;
	}
}

void allocator_global_heap::do_deallocate_sm(void *ptr) {
	if (auto log = get_logger()) {
		log->log("do_deallocate_sm - begin (ptr=" + std::to_string(reinterpret_cast<uintptr_t>(ptr)) + ")",
		         logger::severity::trace);
	}

	if (!ptr) {
		if (auto log = get_logger()) {
			log->log("do_deallocate_sm - nullptr, nothing to do", logger::severity::debug);
		}
		return;
	}

	::operator delete(ptr);

	if (auto log = get_logger()) {
		log->log("do_deallocate_sm - deallocated ptr", logger::severity::trace);
	}
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
	if (auto log = get_logger()) {
		log->log("do_is_equal - comparing resources", logger::severity::trace);
	}
	return this == dynamic_cast<const allocator_global_heap *>(&other);
}

inline logger *allocator_global_heap::get_logger() const { return _logger; }

inline std::string allocator_global_heap::get_typename() const { return "allocator_global_heap"; }
