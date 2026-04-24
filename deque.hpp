#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>

namespace sjtu { 

template<class T>
class deque {
private:
	static const size_t BLOCK_SIZE = 4096 / sizeof(T) > 1 ? 4096 / sizeof(T) : 1;
	
	struct Block {
		T* data;
		Block* prev;
		Block* next;
		
		Block() : data(new T[BLOCK_SIZE]), prev(nullptr), next(nullptr) {}
		~Block() { delete[] data; }
	};
	
	Block* first_block;
	Block* last_block;
	size_t block_count;
	size_t deque_size;

	// Helper function to find block and offset for a given position
	Block* get_block(size_t pos) const {
		size_t block_idx = pos / BLOCK_SIZE;
		Block* current = first_block;
		for (size_t i = 0; i < block_idx; ++i) {
			current = current->next;
		}
		return current;
	}

	// Helper function to create a new block
	Block* create_block(Block* prev_block = nullptr) {
		Block* new_block = new Block();
		new_block->prev = prev_block;
		if (prev_block) {
			prev_block->next = new_block;
		}
		return new_block;
	}

	// Helper function to delete a block
	void delete_block(Block* block) {
		if (block->prev) {
			block->prev->next = block->next;
		}
		if (block->next) {
			block->next->prev = block->prev;
		}
		delete block;
	}

public:
	class const_iterator;
	class iterator {
	private:
		deque* container;
		typename deque::Block* block;
		size_t offset;
		
		// Private constructor for internal use
		iterator(deque* deque_ptr, typename deque::Block* blk, size_t off) 
			: container(deque_ptr), block(blk), offset(off) {}
		
		friend class deque<T>;
		friend class const_iterator;
		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, the behaviour is **undefined**.
		 * as well as operator-
		 */
		iterator operator+(const int &n) const {
			size_t new_pos = offset + n;
			Block* current_block = block;
			
			// Handle moving forward across blocks
			while (new_pos >= BLOCK_SIZE && current_block->next) {
				new_pos -= BLOCK_SIZE;
				current_block = current_block->next;
			}
			
			// Handle moving backward across blocks
			while (new_pos >= BLOCK_SIZE) {
				new_pos -= BLOCK_SIZE;
				if (current_block->prev) {
					current_block = current_block->prev;
				}
			}
			
			return iterator(container, current_block, new_pos);
		}
		
		iterator operator-(const int &n) const {
			return *this + (-n);
		}
		
		// return the distance between two iterator,
		// if these two iterators points to different deques, throw invalid_iterator.
		int operator-(const iterator &rhs) const {
			if (container != rhs.container) {
				throw invalid_iterator();
			}
			
			// Find the position of this iterator
			size_t this_pos = 0;
			Block* current = container->first_block;
			while (current != block) {
				this_pos += BLOCK_SIZE;
				current = current->next;
			}
			this_pos += offset;
			
			// Find the position of rhs iterator
			size_t rhs_pos = 0;
			current = container->first_block;
			while (current != rhs.block) {
				rhs_pos += BLOCK_SIZE;
				current = current->next;
			}
			rhs_pos += rhs.offset;
			
			return static_cast<int>(this_pos - rhs_pos);
		}
		
		iterator operator+=(const int &n) {
			*this = *this + n;
			return *this;
		}
		
		iterator operator-=(const int &n) {
			*this = *this - n;
			return *this;
		}
		
		/**
		 * iter++
		 */
		iterator operator++(int) {
			iterator tmp = *this;
			++(*this);
			return tmp;
		}
		
		/**
		 * ++iter
		 */
		iterator& operator++() {
			++offset;
			if (offset >= BLOCK_SIZE && block->next) {
				block = block->next;
				offset = 0;
			}
			return *this;
		}
		
		/**
		 * iter--
		 */
		iterator operator--(int) {
			iterator tmp = *this;
			--(*this);
			return tmp;
		}
		
		/**
		 * --iter
		 */
		iterator& operator--() {
			if (offset == 0 && block->prev) {
				block = block->prev;
				offset = BLOCK_SIZE - 1;
			} else {
				--offset;
			}
			return *this;
		}
		
		/**
		 * *it
		 */
		T& operator*() const {
			return block->data[offset];
		}
		
		/**
		 * it->field
		 */
		T* operator->() const noexcept {
			return &(block->data[offset]);
		}
		
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {
			return container == rhs.container && block == rhs.block && offset == rhs.offset;
		}
		
		bool operator==(const const_iterator &rhs) const {
			return container == rhs.container && block == rhs.block && offset == rhs.offset;
		}
		
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}
	};
	class const_iterator {
		// it should has similar member method as iterator.
		//  and it should be able to construct from an iterator.
		private:
			const deque* container;
			const Block* block;
			size_t offset;
			
			// Private constructor for internal use
			const_iterator(const deque* deque_ptr, const Block* blk, size_t off) 
				: container(deque_ptr), block(blk), offset(off) {}
			
			friend class deque<T>;
			friend class iterator;

		public:
			const_iterator() : container(nullptr), block(nullptr), offset(0) {}
			
			const_iterator(const iterator &other) 
				: container(other.container), block(other.block), offset(other.offset) {}
			
			const_iterator operator+(const int &n) const {
				size_t new_pos = offset + n;
				const Block* current_block = block;
				
				// Handle moving forward across blocks
				while (new_pos >= BLOCK_SIZE && current_block->next) {
					new_pos -= BLOCK_SIZE;
					current_block = current_block->next;
				}
				
				// Handle moving backward across blocks
				while (new_pos >= BLOCK_SIZE) {
					new_pos -= BLOCK_SIZE;
					if (current_block->prev) {
						current_block = current_block->prev;
					}
				}
				
				return const_iterator(container, current_block, new_pos);
			}
			
			const_iterator operator-(const int &n) const {
				return *this + (-n);
			}
			
			int operator-(const const_iterator &rhs) const {
				if (container != rhs.container) {
					throw invalid_iterator();
				}
				
				// Find the position of this iterator
				size_t this_pos = 0;
				const Block* current = container->first_block;
				while (current != block) {
					this_pos += BLOCK_SIZE;
					current = current->next;
				}
				this_pos += offset;
				
				// Find the position of rhs iterator
				size_t rhs_pos = 0;
				current = container->first_block;
				while (current != rhs.block) {
					rhs_pos += BLOCK_SIZE;
					current = current->next;
				}
				rhs_pos += rhs.offset;
				
				return static_cast<int>(this_pos - rhs_pos);
			}
			
			const_iterator operator+=(const int &n) {
				*this = *this + n;
				return *this;
			}
			
			const_iterator operator-=(const int &n) {
				*this = *this - n;
				return *this;
			}
			
			const_iterator operator++(int) {
				const_iterator tmp = *this;
				++(*this);
				return tmp;
			}
			
			const_iterator& operator++() {
				++offset;
				if (offset >= BLOCK_SIZE && block->next) {
					block = block->next;
					offset = 0;
				}
				return *this;
			}
			
			const_iterator operator--(int) {
				const_iterator tmp = *this;
				--(*this);
				return tmp;
			}
			
			const_iterator& operator--() {
				if (offset == 0 && block->prev) {
					block = block->prev;
					offset = BLOCK_SIZE - 1;
				} else {
					--offset;
				}
				return *this;
			}
			
			const T& operator*() const {
				return block->data[offset];
			}
			
			const T* operator->() const noexcept {
				return &(block->data[offset]);
			}
			
			bool operator==(const iterator &rhs) const {
				return container == rhs.container && block == rhs.block && offset == rhs.offset;
			}
			
			bool operator==(const const_iterator &rhs) const {
				return container == rhs.container && block == rhs.block && offset == rhs.offset;
			}
			
			bool operator!=(const iterator &rhs) const {
				return !(*this == rhs);
			}
			
			bool operator!=(const const_iterator &rhs) const {
				return !(*this == rhs);
			}
	};
	/**
	 * Constructors
	 */
	deque() : first_block(nullptr), last_block(nullptr), block_count(0), deque_size(0) {}
	
	deque(const deque &other) : first_block(nullptr), last_block(nullptr), block_count(0), deque_size(0) {
		// Copy elements from other deque
		for (const auto& element : other) {
			push_back(element);
		}
	}
	
	/**
	 * Deconstructor
	 */
	~deque() {
		clear();
	}
	
	/**
	 * assignment operator
	 */
	deque &operator=(const deque &other) {
		if (this == &other) {
			return *this;
		}
		
		clear();
		
		// Copy elements from other deque
		for (const auto& element : other) {
			push_back(element);
		}
		
		return *this;
	}
	/**
	 * access specified element with bounds checking
	 * throw index_out_of_bound if out of bound.
	 */
	T & at(const size_t &pos) {
		if (pos >= deque_size) {
			throw index_out_of_bound();
		}
		Block* target_block = get_block(pos);
		return target_block->data[pos % BLOCK_SIZE];
	}
	
	const T & at(const size_t &pos) const {
		if (pos >= deque_size) {
			throw index_out_of_bound();
		}
		Block* target_block = get_block(pos);
		return target_block->data[pos % BLOCK_SIZE];
	}
	
	T & operator[](const size_t &pos) {
		Block* target_block = get_block(pos);
		return target_block->data[pos % BLOCK_SIZE];
	}
	
	const T & operator[](const size_t &pos) const {
		Block* target_block = get_block(pos);
		return target_block->data[pos % BLOCK_SIZE];
	}
	
	/**
	 * access the first element
	 * throw container_is_empty when the container is empty.
	 */
	const T & front() const {
		if (empty()) {
			throw container_is_empty();
		}
		return first_block->data[0];
	}
	
	/**
	 * access the last element
	 * throw container_is_empty when the container is empty.
	 */
	const T & back() const {
		if (empty()) {
			throw container_is_empty();
		}
		return last_block->data[(deque_size - 1) % BLOCK_SIZE];
	}
	
	/**
	 * returns an iterator to the beginning.
	 */
	iterator begin() {
		if (empty()) {
			return end();
		}
		return iterator(this, first_block, 0);
	}
	
	const_iterator cbegin() const {
		if (empty()) {
			return cend();
		}
		return const_iterator(this, first_block, 0);
	}
	
	/**
	 * returns an iterator to the end.
	 */
	iterator end() {
		if (empty()) {
			return iterator(this, nullptr, 0);
		}
		// Return an iterator pointing to one past the last element
		size_t last_offset = (deque_size - 1) % BLOCK_SIZE;
		if (last_offset == BLOCK_SIZE - 1) {
			// Last element is at the end of a block
			if (last_block->next) {
				return iterator(this, last_block->next, 0);
			} else {
				return iterator(this, nullptr, 0);
			}
		} else {
			return iterator(this, last_block, last_offset + 1);
		}
	}
	
	const_iterator cend() const {
		if (empty()) {
			return const_iterator(this, nullptr, 0);
		}
		// Return an iterator pointing to one past the last element
		size_t last_offset = (deque_size - 1) % BLOCK_SIZE;
		if (last_offset == BLOCK_SIZE - 1) {
			// Last element is at the end of a block
			if (last_block->next) {
				return const_iterator(this, last_block->next, 0);
			} else {
				return const_iterator(this, nullptr, 0);
			}
		} else {
			return const_iterator(this, last_block, last_offset + 1);
		}
	}
	
	/**
	 * checks whether the container is empty.
	 */
	bool empty() const {
		return deque_size == 0;
	}
	
	/**
	 * returns the number of elements
	 */
	size_t size() const {
		return deque_size;
	}
	
	/**
	 * clears the contents
	 */
	void clear() {
		// Delete all blocks
		Block* current = first_block;
		while (current) {
			Block* next = current->next;
			delete current;
			current = next;
		}
		
		first_block = nullptr;
		last_block = nullptr;
		block_count = 0;
		deque_size = 0;
	}
	/**
	 * inserts elements at the specified location in the container.
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value
	 *     throw if the iterator is invalid or it point to a wrong place.
	 */
	iterator insert(iterator pos, const T &value) {
		if (pos.container != this) {
			throw invalid_iterator();
		}
		
		// If inserting at the end
		if (pos == end()) {
			push_back(value);
			return iterator(this, last_block, (deque_size - 1) % BLOCK_SIZE);
		}
		
		// If inserting at the beginning
		if (pos == begin()) {
			push_front(value);
			return begin();
		}
		
		// Find the position to insert
		size_t insert_pos = 0;
		Block* current = first_block;
		while (current != pos.block) {
			insert_pos += BLOCK_SIZE;
			current = current->next;
		}
		insert_pos += pos.offset;
		
		// Insert at the beginning of the deque
		if (insert_pos == 0) {
			push_front(value);
			return begin();
		}
		
		// Insert at the end of the deque
		if (insert_pos == deque_size) {
			push_back(value);
			return iterator(this, last_block, (deque_size - 1) % BLOCK_SIZE);
		}
		
		// For other positions, we need to shift elements
		push_back(back());
		
		// Shift elements from the end to the insertion point
		for (size_t i = deque_size - 2; i > insert_pos; --i) {
			(*this)[i] = (*this)[i - 1];
		}
		
		// Insert the new value
		(*this)[insert_pos] = value;
		
		return iterator(this, get_block(insert_pos), insert_pos % BLOCK_SIZE);
	}
	
	/**
	 * removes specified element at pos.
	 * removes the element at pos.
	 * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
	 * throw if the container is empty, the iterator is invalid or it points to a wrong place.
	 */
	iterator erase(iterator pos) {
		if (empty() || pos.container != this) {
			throw invalid_iterator();
		}
		
		// Find the position to erase
		size_t erase_pos = 0;
		Block* current = first_block;
		while (current != pos.block) {
			erase_pos += BLOCK_SIZE;
			current = current->next;
		}
		erase_pos += pos.offset;
		
		// If erasing the first element
		if (erase_pos == 0) {
			pop_front();
			return begin();
		}
		
		// If erasing the last element
		if (erase_pos == deque_size - 1) {
			pop_back();
			return end();
		}
		
		// For other positions, shift elements
		for (size_t i = erase_pos; i < deque_size - 1; ++i) {
			(*this)[i] = (*this)[i + 1];
		}
		
		pop_back();
		
		return iterator(this, get_block(erase_pos), erase_pos % BLOCK_SIZE);
	}
	
	/**
	 * adds an element to the end
	 */
	void push_back(const T &value) {
		if (empty()) {
			first_block = create_block();
			last_block = first_block;
			block_count = 1;
			deque_size = 1;
			first_block->data[0] = value;
			return;
		}
		
		size_t last_offset = (deque_size - 1) % BLOCK_SIZE;
		
		// If the last block is full, create a new block
		if (last_offset == BLOCK_SIZE - 1) {
			Block* new_block = create_block(last_block);
			last_block = new_block;
			block_count++;
			deque_size++;
			last_block->data[0] = value;
		} else {
			// Add to the current last block
			deque_size++;
			last_block->data[last_offset + 1] = value;
		}
	}
	
	/**
	 * removes the last element
	 *     throw when the container is empty.
	 */
	void pop_back() {
		if (empty()) {
			throw container_is_empty();
		}
		
		size_t last_offset = (deque_size - 1) % BLOCK_SIZE;
		
		// If this was the last element
		if (deque_size == 1) {
			delete first_block;
			first_block = nullptr;
			last_block = nullptr;
			block_count = 0;
			deque_size = 0;
			return;
		}
		
		// If the last block becomes empty after removal
		if (last_offset == 0) {
			Block* prev_block = last_block->prev;
			delete_block(last_block);
			last_block = prev_block;
			block_count--;
		}
		
		deque_size--;
	}
	
	/**
	 * inserts an element to the beginning.
	 */
	void push_front(const T &value) {
		if (empty()) {
			push_back(value);
			return;
		}
		
		// If the first block has space at the beginning
		if (first_block->prev == nullptr) {
			// Create a new block before the first block
			Block* new_block = new Block();
			new_block->next = first_block;
			first_block->prev = new_block;
			first_block = new_block;
			block_count++;
		}
		
		// Shift all elements one position to the right
		push_back(back());
		
		for (size_t i = deque_size - 2; i > 0; --i) {
			(*this)[i] = (*this)[i - 1];
		}
		
		(*this)[0] = value;
	}
	
	/**
	 * removes the first element.
	 *     throw when the container is empty.
	 */
	void pop_front() {
		if (empty()) {
			throw container_is_empty();
		}
		
		// If this was the last element
		if (deque_size == 1) {
			pop_back();
			return;
		}
		
		// Shift all elements one position to the left
		for (size_t i = 0; i < deque_size - 1; ++i) {
			(*this)[i] = (*this)[i + 1];
		}
		
		pop_back();
	}
};

}

#endif
