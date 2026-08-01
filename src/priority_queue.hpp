#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct node {
		T value;
		node *left;
		node *right;
		int dist;
		int ref;

		explicit node(const T &v) : value(v), left(nullptr), right(nullptr), dist(1), ref(1) {}
	};

	node *root;
	size_t node_count;
	Compare cmp;

	static int distance(node *p) {
		return p ? p->dist : 0;
	}

	static void retain(node *p) {
		if (p != nullptr) ++p->ref;
	}

	static void release(node *p) {
		if (p == nullptr) return;
		if (--p->ref == 0) {
			release(p->left);
			release(p->right);
			delete p;
		}
	}

	static node *clone(node *p) {
		if (p == nullptr) return nullptr;
		node *copy = new node(p->value);
		try {
			copy->left = clone(p->left);
			copy->right = clone(p->right);
			copy->dist = p->dist;
		} catch (...) {
			release(copy->left);
			release(copy->right);
			delete copy;
			throw;
		}
		return copy;
	}

	node *merge_nodes(node *a, node *b) {
		if (a == nullptr) {
			retain(b);
			return b;
		}
		if (b == nullptr) {
			retain(a);
			return a;
		}
		if (cmp(a->value, b->value)) {
			node *tmp = a;
			a = b;
			b = tmp;
		}
		node *result = new node(a->value);
		try {
			result->left = a->left;
			retain(result->left);
			result->right = merge_nodes(a->right, b);
			if (distance(result->left) < distance(result->right)) {
				node *tmp = result->left;
				result->left = result->right;
				result->right = tmp;
			}
			result->dist = distance(result->right) + 1;
		} catch (...) {
			release(result->left);
			release(result->right);
			delete result;
			throw;
		}
		return result;
	}

	void clear() {
		release(root);
		root = nullptr;
		node_count = 0;
	}

public:
	/**
	 * @brief default constructor
	 */
	priority_queue() : root(nullptr), node_count(0), cmp(Compare()) {}

	/**
	 * @brief copy constructor
	 * @param other the priority_queue to be copied
	 */
	priority_queue(const priority_queue &other) : root(other.root), node_count(other.node_count), cmp(other.cmp) {
		retain(root);
	}

	/**
	 * @brief deconstructor
	 */
	~priority_queue() {
		clear();
	}

	/**
	 * @brief Assignment operator
	 * @param other the priority_queue to be assigned from
	 * @return a reference to this priority_queue after assignment
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		Compare new_cmp(other.cmp);
		node *new_root = other.root;
		retain(new_root);
		release(root);
		root = new_root;
		node_count = other.node_count;
		cmp = new_cmp;
		return *this;
	}

	/**
	 * @brief get the top element of the priority queue.
	 * @return a reference of the top element.
	 * @throws container_is_empty if empty() returns true
	 */
	const T & top() const {
		if (empty()) throw container_is_empty();
		return root->value;
	}

	/**
	 * @brief push new element to the priority queue.
	 * @param e the element to be pushed
	 */
	void push(const T &e) {
		node *single = new node(e);
		node *old_root = root;
		try {
			root = merge_nodes(root, single);
			release(old_root);
			release(single);
			++node_count;
		} catch (...) {
			delete single;
			throw runtime_error();
		}
	}

	/**
	 * @brief delete the top element from the priority queue.
	 * @throws container_is_empty if empty() returns true
	 */
	void pop() {
		if (empty()) throw container_is_empty();
		node *old_root = root;
		try {
			root = merge_nodes(root->left, root->right);
			release(old_root);
			--node_count;
		} catch (...) {
			throw runtime_error();
		}
	}

	/**
	 * @brief return the number of elements in the priority queue.
	 * @return the number of elements.
	 */
	size_t size() const {
		return node_count;
	}

	/**
	 * @brief check if the container is empty.
	 * @return true if it is empty, false otherwise.
	 */
	bool empty() const {
		return node_count == 0;
	}

	/**
	 * @brief merge another priority_queue into this one.
	 * The other priority_queue will be cleared after merging.
	 * The complexity is at most O(logn).
	 * @param other the priority_queue to be merged.
	 */
	void merge(priority_queue &other) {
		if (this == &other) return;
		try {
			node *new_root = merge_nodes(root, other.root);
			release(root);
			root = new_root;
			node_count += other.node_count;
			release(other.root);
			other.root = nullptr;
			other.node_count = 0;
		} catch (...) {
			throw runtime_error();
		}
	}
};

}

#endif
