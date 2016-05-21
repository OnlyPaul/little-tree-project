#ifndef BpTree_H
#define BpTree_H

#include <iostream>
#include "Container.h"

class BpTreeNotImplementedException: public ContainerException {
public:
	virtual const char * what() const noexcept override { return "BpTree: Not implemented!"; }
};

template <typename E, size_t k=2>
class BpTree : public Container<E> {
	// nested BpNode class -> act as nodes and leaves in b+tree
	class BpNode {
	public:
		// instance declarations
		static const size_t order = 2*k;
		E key[order]; // values contained in BpNode
		BpNode* children[order+1]; // pointers point to child nodes - in leaf, these will be nullptr
		BpNode* parent; // show, who the parent of this BpNode is - parent = nullptr, if and only if parent is root
		BpNode* left; // points to next node in the same depth
		BpNode* right; // point to previous node in the same depth
		size_t n_key; // tell how many values are contained in this bucket
		bool is_leaf; // if the BpNode is leaf, isLeaf == 1

		// constructure&destructor
		BpNode(BpNode* parent_=nullptr) : parent(parent_) {
			is_leaf = true;
			n_key = 0;
			left = nullptr;
			right = nullptr;
		}
		~BpNode() {
			if(is_leaf)
				return;
			for(size_t i = 0; i < n_key+1; i++)
				delete children[i];
		}

		// methods declaration
		BpNode* search(const E& e); // search tree for a value, e, return node that 'may' contain the value. normally call from root
		BpNode* insert(E e); // more accurately, this is "insert to leaf", which will be manipulated by add()
		BpNode* split();
		BpNode* address_to_parent();
		bool member_(const E& e);
		size_t size_();
		std::ostream& print(std::ostream& o, int depth) const;
	};

	BpNode *root; // class contructor constructs a root to be the initial container for incoming keys
public:
	BpTree() : root(new BpNode(nullptr)) {}
	BpTree(std::initializer_list<E> el) : BpTree() { for (auto e: el) add(e); }
	~BpTree() { delete root; }

	using Container<E>::add;
	virtual void add(const E& e) override;
	virtual void add(const E e[], size_t s) override;

	using Container<E>::remove;
	virtual void remove(const E[], size_t) override { throw BpTreeNotImplementedException(); }

	virtual bool member(const E& e) const override { return root->member_(e); }
	virtual size_t size() const override { return root->size_(); }
	virtual bool empty() const override { throw BpTreeNotImplementedException(); }

	virtual size_t apply(std::function<void(const E&)>, Order = dontcare) const override { throw BpTreeNotImplementedException(); }

	virtual E min() const override { throw BpTreeNotImplementedException(); }
	virtual E max() const override { throw BpTreeNotImplementedException(); }

	virtual std::ostream& print( std::ostream& o ) const override { return root->print(o, 0); }
};

// BpNode implementation
template <typename E, size_t k>
typename BpTree<E,k>::BpNode* BpTree<E,k>::BpNode::search(const E& e) {
	if(is_leaf)
		return this;

	for(size_t i=0; i < n_key; i++) {
		if(e < key[i])
			return children[i]->search(e);
	}

	if(children[n_key+1] != nullptr)
		return children[n_key+1]->search(e);
	else return nullptr;
}

template <typename E, size_t k>
typename BpTree<E,k>::BpNode* BpTree<E,k>::BpNode::insert(E e) {
	// find exact position of value
	size_t i = 0;
	for (i=0; i < n_key; i++) {
		if (e == key[i])
			return nullptr; // same value insertion is not allowed
		else if (e < key[i])
			break;
	}

	// shift old keys, then insert e ti the position i
	for(size_t j=n_key; j > i; j--)
		key[j] = key[j-1];
	key[i] = e;
	n_key++;

	return this;
}

template <typename E, size_t k>
typename BpTree<E,k>::BpNode* BpTree<E,k>::BpNode::split() {
	size_t split_point = n_key/2;
	BpNode* new_node = new BpNode(parent);
	new_node->is_leaf = this->is_leaf;

	for(size_t i=split_point; i < n_key; i++)
		new_node->key[new_node->n_key++] = key[i];

	new_node->left = this;
	new_node->right = this->right;
	this->right->left = new_node;
	this->right = new_node;

	new_node->address_to_parent();

	return new_node;
}

template <typename E, size_t k>
typename BpTree<E,k>::BpNode* BpTree<E,k>::BpNode::address_to_parent() {
	if(parent==nullptr) // this is the old root level, let the root handling in add() do the job
		return nullptr;

	// if parent is not full, add smallest key to parent node
	if(parent->n_key < order)
		parent->insert(key[0]);
	else
		parent->split();

	return nullptr;
}

template <typename E, size_t k>
bool BpTree<E,k>::BpNode::member_(const E& e) {
	if(n_key==0)
		return false;

	// search to the possible leaf, then if there is the key with certain value, return true
	BpNode* node = search(e);
	for(size_t i = 0; i < node->n_key; i++) {
		if(e == node->key[i])
			return true;
	}

	return false;
}

template <typename E, size_t k>
size_t BpTree<E,k>::BpNode::size_() {
	// if it is leaf, return number of entries
	if(is_leaf)
		return n_key;

	// in case of inner nodes, recursive until their leaves and get the sum of entries
	size_t sum=0;
	for(size_t i=0; i < n_key+1; i++)
		sum += children[i]->size_();
	return sum;
}

template <typename E, size_t k>
std::ostream& BpTree<E,k>::BpNode::print(std::ostream& o, int depth) const {
	for (int i = 0; i < depth; i++)
		o << "  ";

	if (is_leaf)
		o << "leaf : " << this << std::endl;
	else
		o << "inner: " << this << std::endl;
	for (size_t i = 0; i < n_key; i++) {
		if (!is_leaf)
			children[i]->print(o, depth+1);
		for (int j = 0; j < depth; j++) {
			o << "  ";
		}
		o << "  " << this->key[i] << std::endl;
	}
	if (!is_leaf)
		children[n_key]->print(o, depth+1); // recursive call print for children
	return o;
}



// BpTree implementation (override: Container)
template <typename E, size_t k>
void BpTree<E,k>::add(const E& e) {
	// leaf insertion&leaf split
	BpNode* node = root->search(e); // perform search
	if(node->n_key < node->order) // if node is not full, insert val
		node->insert(e);
	else
		node->split();
	
	// split handling: if root is split, create new root
}

template <typename E, size_t k>
void BpTree<E,k>::add(const E e[], size_t s) {
	for(size_t i=0; i < s; i++)
		add(e[i]);
}

#endif //BpTree_H