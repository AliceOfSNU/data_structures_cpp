//binary leftist tree

/*
* Properties
*	- it's a min heap, children have higher key
*	- heavier on the left side. dist(left) >= dist(right)
*	- dist(i) = 1 + dist(right(i))
*/

#include <optional>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

int deleteCount = 0;
int createCount = 0;

template <typename T>
class LeftistNode {
public:
	LeftistNode(const T& key) : Key(key) {
		createCount++;
	}
	LeftistNode(const LeftistNode<T>& node) = delete; //no copy constructor.
	~LeftistNode() {
		deleteCount++;
	}
	T Key;
	int Dist = 0;
	LeftistNode* Left = nullptr;
	LeftistNode* Right = nullptr;
};

template <typename T>
class LeftistTree {
public:
	LeftistTree(const LeftistNode<T>* rootNode) : root(rootNode) {}
	LeftistTree() : root(nullptr) {}

	std::optional<T> ExtractMin() {
		if (!root) return std::nullopt;
		const T minValue = root->Key;
		LeftistNode<T>* oldRoot = root;
		if (!root->Left) {
			root = root->Right;
		}
		else if (!root->Right) {
			root = root->Left;
		}
		else if (root->Left->Key <= root->Right->Key) {
			root = merge(root->Left, root->Right);
		}
		else {
			root = merge(root->Right, root->Left);
		}
		oldRoot->Right = nullptr;
		oldRoot->Left = nullptr;
		delete(oldRoot);
		return minValue;
	}

	std::optional<T> PeekMin() {
		if (!root) return std::nullopt;
		return root->Key;
	}

	void Insert(const T key) {
		LeftistNode<T>* newNode = new LeftistNode<T>(key);
		if (!root) {
			root = newNode;
		}
		else if (root->Key <= key) {
			root = merge(root, newNode);
		}
		else {
			root = merge(newNode, root);
		}
	}


private:
	LeftistNode<T>* merge(LeftistNode<T>* smaller, LeftistNode<T>* larger) {
		//assumes left tree is smaller, right tree is larger
		//also assumes both are not null.
		if (!smaller->Right) {
			smaller->Right = larger;
		}
		else if (smaller->Right->Key <= larger->Key) {
			smaller->Right = merge(smaller->Right, larger);
		}
		else {
			smaller->Right = merge(larger, smaller->Right);
		}

		//smaller->Right cannot be null from this point.

		//keep the left biased property
		if (!smaller->Left) {
			smaller->Left = smaller->Right;
			smaller->Right = nullptr;
		}
		else if (smaller->Left->Dist < smaller->Right->Dist) {
			LeftistNode<T>* tmp = smaller->Left;
			smaller->Left = smaller->Right;
			smaller->Right = tmp;
		}

		//now Right may be null.
		smaller->Dist = smaller->Right ? smaller->Right->Dist + 1 : 0;
		return smaller;
	}

	LeftistNode<T>* root;
};

void main() {
	LeftistTree<int> tree = LeftistTree<int>();
	std::vector<int> inserted = std::vector<int>();
	for (int i = 0; i < 20; ++i) {
		int randi = std::rand() % 1000;
		tree.Insert(randi);
		inserted.push_back(randi);
	}
	std::sort(inserted.begin(), inserted.end());
	for (int i = 0; i < inserted.size(); ++i) {
		std::optional<int> extracted = tree.ExtractMin();
		if (!extracted.has_value()) {
			std::cout << "missing value " << inserted[i] << std::endl;
			return;
		}

		if (inserted[i] != extracted.value()) {
			std::cout << "out of order expected: " << inserted[i] << ", was ; " << extracted.value() << std::endl;
		}
	}

}