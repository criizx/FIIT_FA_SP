#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H

#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <pp_allocator.h>
#include <typename_holder.h>

#include <cmath>
#include <mutex>

using byte = uint8_t;

namespace __detail {
constexpr size_t nearest_greater_k_of_2(size_t size) noexcept {
	int ones_counter = 0, index = -1;

	constexpr const size_t o = 1;

	for (int i = sizeof(size_t) * 8 - 1; i >= 0; --i) {
		if (size & (o << i)) {
			if (ones_counter == 0) index = i;
			++ones_counter;
		}
	}

	return ones_counter <= 1 ? index : index + 1;
}
}  // namespace __detail

class allocator_buddies_system final : public smart_mem_resource,
                                       public allocator_test_utils,
                                       public allocator_with_fit_mode,
                                       private logger_guardant,
                                       private typename_holder {
   private:
	struct block_metadata {
		bool occupied : 1;
		byte size : 7;
	};

	void *_trusted_memory;

	/**
	 * TODO: You must improve it for alignment support
	 */

	static constexpr const size_t allocator_metadata_size = sizeof(logger *) + sizeof(allocator_dbg_helper *) +
	                                                        sizeof(fit_mode) + sizeof(unsigned char) +
	                                                        sizeof(std::mutex);

	static constexpr const size_t occupied_block_metadata_size = sizeof(block_metadata) + sizeof(void *);

	static constexpr const size_t free_block_metadata_size = sizeof(block_metadata);

	static constexpr const size_t min_k = __detail::nearest_greater_k_of_2(occupied_block_metadata_size);

   public:
	explicit allocator_buddies_system(
	    size_t space_size_power_of_two, std::pmr::memory_resource *parent_allocator = nullptr, logger *logger = nullptr,
	    allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

	allocator_buddies_system(allocator_buddies_system const &other);

	allocator_buddies_system &operator=(allocator_buddies_system const &other);

	allocator_buddies_system(allocator_buddies_system &&other) noexcept;

	allocator_buddies_system &operator=(allocator_buddies_system &&other) noexcept;

	~allocator_buddies_system() override;

   public:
	[[nodiscard]] void *do_allocate_sm(size_t size) override;

	void do_deallocate_sm(void *at) override;

	bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override;

	inline void set_fit_mode(allocator_with_fit_mode::fit_mode mode) override;

	inline size_t pow_2(size_t size) const noexcept;
	byte degree_of_2(size_t size) const noexcept;  // HMMMMMMMMMMMMMMMMMMMMM

	std::pmr::memory_resource *get_memory_resource() const;
	fit_mode &get_fit_mod() const noexcept;
	std::mutex &get_mutex() const noexcept;
	size_t get_allocator_pool_size() const noexcept;
	size_t get_block_size(void *current_block) const noexcept;
	void *get_block_twin(void *current_block) noexcept;
	fit_mode &get_fit_mode() const noexcept;

   private:
	inline logger *get_logger() const override;

	inline std::string get_typename() const override;

	size_t get_size_memory() const noexcept;

	std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;
	std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

	void split_block(void *block, byte required_power);

	bool is_busy(void *current_block) const noexcept;

	void *find_first_suitable_block(size_t block_size_to_find) const noexcept;
	void *find_worst_suitable_block(size_t block_size_to_find) const noexcept;
	void *find_best_suitable_block(size_t block_size_to_find) const noexcept;

	/** TODO: `	`````Highly recommended for helper functions to return references */

	class buddy_iterator {
		void *_block;

	   public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = void *;
		using reference = void *&;
		using pointer = void **;
		using difference_type = ptrdiff_t;

		bool operator==(const buddy_iterator &) const noexcept;

		bool operator!=(const buddy_iterator &) const noexcept;

		buddy_iterator &operator++() & noexcept;

		buddy_iterator operator++(int n);

		size_t size() const noexcept;

		bool occupied() const noexcept;

		void *operator*() const noexcept;

		buddy_iterator();

		buddy_iterator(void *start);
	};

	friend class buddy_iterator;

	buddy_iterator begin() const noexcept;

	buddy_iterator end() const noexcept;
};

#endif  // MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
