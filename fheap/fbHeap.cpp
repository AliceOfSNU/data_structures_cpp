#include <iostream>
#include <optional>
#include <vector>
#include <algorithm>

template <typename T>
class fbNode {
public:
	fbNode();
	fbNode(const T& item) : key(item), degree(0), marked(false), right(nullptr), child(nullptr) {}

	T key;
	size_t degree;
	bool marked;

	std::shared_ptr<fbNode<T>> right;
	std::shared_ptr<fbNode<T>> child;

	//link nodes in both ways.. using weak_ptr to prevent circular reference
	std::weak_ptr<fbNode<T>> left;
	std::weak_ptr<fbNode<T>> parent;
};


template <typename T>
class fbHeap {
public:
	fbHeap() : min_node(nullptr), size(0) {}
	~fbHeap() {
		while (!is_empty()) {
			extract_min();
		}
		// NOTE: Be aware of memory leak or memory error.
	}

	bool is_empty() const { return !size; }

	void insert(const T& item) {
		//create a fbNode
		std::shared_ptr<fbNode<T>> new_node = std::make_shared<fbNode<T>>(item);

		//place it at the beginning of the root list.
		//  [min_node] <-> [new_node*] <-> [right] <-> ...
		if (min_node == nullptr) {
			min_node = new_node;
		}
		else {
			if (min_node->right) {
				new_node->right = min_node->right;
			}
			else {
				new_node->right = min_node;
			}
			new_node->right->left = new_node;

			min_node->right = new_node;
			new_node->left = min_node;
		}
		size++;

		//min node should be kept updated.
		if (new_node->key < min_node->key) min_node = new_node;
	}

	std::optional<T> extract_min() {
		if (min_node == nullptr) return std::nullopt; //nothing to do.
		std::shared_ptr<fbNode<T>> deleteNode(min_node); //copy it.
		std::shared_ptr<fbNode<T>> left = min_node->left.lock();
		std::shared_ptr<fbNode<T>> right = min_node->right;
		std::shared_ptr<fbNode<T>> child = min_node->child;

		if (!child) {
			//[left] <-> [min_node(delete*)] <-> [right]
			
			if (right) {
				right->left = (left != right) ? left : std::weak_ptr<fbNode<T>>();
			}
			if (left) {
				left->right = (right != left) ?  right : nullptr;
			}
		}
		else {
			//wrap around to get the other end.
			std::shared_ptr<fbNode<T>> rightmostChild = child->left.lock(); //may be null
			if (!rightmostChild) rightmostChild = child;
			for (auto node = child; node; node = node->right) {
				node->parent.reset();
				if (node == rightmostChild) break;
			}

			if (left) {
				left->right = child; // not null
				child->left = std::weak_ptr<fbNode<T>>(left);
			}

			if (right) {
				rightmostChild->right = right;
				right->left = std::weak_ptr<fbNode<T>>(rightmostChild);
			}
		}

		//update min_node
		if (right) {
			min_node = right;
		}
		else {
			min_node = min_node->child; //may be null, but ok.
		}

		size--;
		consolidate();
		return deleteNode->key;
	}

	void consolidate() {
		if (!min_node) return;

		float phi = (1 + sqrt(5)) / 2.0;
		int len = int(log(size) / log(phi)) + 10;
		std::unique_ptr<std::shared_ptr<fbNode<T>>[]> arr(new std::shared_ptr<fbNode<T>>[len]);

		const std::shared_ptr<fbNode<T>> until = min_node->left.lock();
		if (!until) return; //trivial. nothing to do.

		std::shared_ptr<fbNode<T>> node = min_node;
		while (node) {
			std::shared_ptr<fbNode<T>> other = arr[node->degree];
			std::shared_ptr<fbNode<T>> next = node == until ? nullptr : node->right; //cache next step, since "node" might change due to merge op.
			std::shared_ptr<fbNode<T>> merged = node;
			while (other) {
				arr[other->degree] = nullptr; //clear the bin
				merged = merged->key <= other->key ? merge(merged, other) : merge(other, merged);
				if (!merged) break;
				other = arr[merged->degree];
			}
			arr[merged->degree] = merged;
			//moving to next node.
			node = next;
		}

		//link the result array as the root array.
		node = nullptr;
		std::shared_ptr<fbNode<T>> prev, start(nullptr);
		for (int i = 0; i < len; ++i) {
			if (!arr[i]) continue;
			node = arr[i];
			if (!start) start = node;
			if (!min_node || node->key <= min_node->key) min_node = node; //update minimum
			if (prev) {
				prev->right = node; //node is not null
				node->left = std::weak_ptr(prev); //prev is not null
			} 
			prev = node;
		}
		if (node) {
			//start is not null
			node->right = (start != node) ? start : nullptr;
			start->left = std::weak_ptr((start != node) ? node : nullptr);
		}

	}

	std::shared_ptr<fbNode<T>> merge(std::shared_ptr<fbNode<T>>& smaller, std::shared_ptr<fbNode<T>>& larger) {
		std::shared_ptr<fbNode<T>> child = smaller->child;
		if (!child) {
			//no child
			smaller->child = larger;
			larger->right.reset();
			larger->left.reset();
		}
		else {
			//insert to the right of smaller->child
			larger->right = child->right ? child->right : child;
			if (larger->right) larger->right->left = std::weak_ptr(larger);

			child->right = larger;
			larger->left = std::weak_ptr(child);
		}
		smaller->degree++;
		larger->parent = smaller;

		return smaller;
	}

	std::optional<T> get_min() const {
		if (!min_node)
			return std::nullopt;
		else
			return min_node->key;
	}



	size_t size;
	std::shared_ptr<fbNode<T>> min_node;

private:
};

void main() {
	fbHeap<int> heap = fbHeap<int>();
	std::vector<int> inserted = std::vector<int>();
	for (int i = 0; i < 200; ++i) {
		int randi = std::rand() % 1000;
		heap.insert(randi);
		inserted.push_back(randi);
	}
	std::sort(inserted.begin(), inserted.end());
	for (int i = 0; i < inserted.size(); ++i) {
		std::optional<int> extracted = heap.extract_min();
		if (!extracted.has_value()) {
			std::cout << "missing value " << inserted[i] << std::endl;
			return;
		}

		if (inserted[i] != extracted.value()) {
			std::cout << "out of order expected: " << inserted[i] << ", was ; " << extracted.value() << std::endl;
		}
		std::cout << inserted[i] << "==" << extracted.value() << std::endl;
	}
}