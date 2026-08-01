#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct node {
		T value;
		node *left;
		node *right;
		int dist;

		explicit node(const T &v) : value(v), left(nullptr), right(nullptr), dist(1) {}
	};

	node *root;
	size_t node_count;
	Compare cmp;

	static int distance(node *p) {
		return p ? p->dist : 0;
	}

	static void delete_tree(node *p) {
		if (p == nullptr) return;
		delete_tree(p->left);
		delete_tree(p->right);
		delete p;
	}

	static node *clone_tree(node *p) {
		if (p == nullptr) return nullptr;
		node *copy = new node(p->value);
		try {
			copy->left = clone_tree(p->left);
			copy->right = clone_tree(p->right);
			copy->dist = p->dist;
		} catch (...) {
			delete_tree(copy->left);
			delete_tree(copy->right);
			delete copy;
			throw;
		}
		return copy;
	}

	node *merge_nodes(node *a, node *b) {
		if (a == nullptr) return b;
		if (b == nullptr) return a;
		if (cmp(a->value, b->value)) {
			node *tmp = a;
			a = b;
			b = tmp;
		}
		a->right = merge_nodes(a->right, b);
		if (distance(a->left) < distance(a->right)) {
			node *tmp = a->left;
			a->left = a->right;
			a->right = tmp;
		}
		a->dist = distance(a->right) + 1;
		return a;
	}

	void clear() {
		delete_tree(root);
		root = nullptr;
		node_count = 0;
	}

public:
	priority_queue() : root(nullptr), node_count(0), cmp(Compare()) {}

	priority_queue(const priority_queue &other) : root(nullptr), node_count(other.node_count), cmp(other.cmp) {
		root = clone_tree(other.root);
	}

	~priority_queue() {
		clear();
	}

	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		priority_queue temp(other);
		Compare new_cmp(temp.cmp);
		clear();
		root = temp.root;
		node_count = temp.node_count;
		cmp = new_cmp;
		temp.root = nullptr;
		temp.node_count = 0;
		return *this;
	}

	const T & top() const {
		if (empty()) throw container_is_empty();
		return root->value;
	}

	void push(const T &e) {
		node *single = new node(e);
		try {
			root = merge_nodes(root, single);
			++node_count;
		} catch (...) {
			delete single;
			throw runtime_error();
		}
	}

	void pop() {
		if (empty()) throw container_is_empty();
		node *old_root = root;
		try {
			root = merge_nodes(root->left, root->right);
			delete old_root;
			--node_count;
		} catch (...) {
			throw runtime_error();
		}
	}

	size_t size() const {
		return node_count;
	}

	bool empty() const {
		return node_count == 0;
	}

	void merge(priority_queue &other) {
		if (this == &other) return;
		try {
			root = merge_nodes(root, other.root);
			node_count += other.node_count;
			other.root = nullptr;
			other.node_count = 0;
		} catch (...) {
			throw runtime_error();
		}
	}
};

}

#endif
