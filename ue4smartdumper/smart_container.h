#pragma once
#include <iostream>
#include <stack>
#include <algorithm> // For std::max
#include <cassert>   // For assert

template <typename Key, typename Value>
class AVLTree {
private:
    struct Node {
        Key key;
        Value value;
        Node* left;
        Node* right;
        int height;

        Node(Key k, Value v) : key(k), value(v), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;

    int height(Node* n) const {
        return n ? n->height : 0;
    }

    int getBalance(Node* n) const {
        return n ? height(n->left) - height(n->right) : 0;
    }

    Node* rightRotate(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;

        return x;
    }

    Node* leftRotate(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;

        return y;
    }

    Node* insert(Node* node, Key key, Value value) {
        if (!node) return new Node(key, value);

        if (key < node->key)
            node->left = insert(node->left, key, value);
        else if (key > node->key)
            node->right = insert(node->right, key, value);
        else {
            node->value = value; // Update the value for existing key
            return node;
        }

        node->height = 1 + std::max(height(node->left), height(node->right));

        int balance = getBalance(node);

        if (balance > 1 && key < node->left->key)
            return rightRotate(node);

        if (balance < -1 && key > node->right->key)
            return leftRotate(node);

        if (balance > 1 && key > node->left->key) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (balance < -1 && key < node->right->key) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    Node* minValueNode(Node* node) const {
        Node* current = node;
        while (current->left)
            current = current->left;
        return current;
    }

    Node* deleteNode(Node* root, Key key) {
        if (!root)
            return root;

        if (key < root->key)
            root->left = deleteNode(root->left, key);
        else if (key > root->key)
            root->right = deleteNode(root->right, key);
        else {
            if ((!root->left) || (!root->right)) {
                Node* temp = root->left ? root->left : root->right;
                if (!temp) {
                    temp = root;
                    root = nullptr;
                }
                else
                    *root = *temp;
                delete temp;
            }
            else {
                Node* temp = minValueNode(root->right);
                root->key = temp->key;
                root->value = temp->value;
                root->right = deleteNode(root->right, temp->key);
            }
        }

        if (!root)
            return root;

        root->height = 1 + std::max(height(root->left), height(root->right));
        int balance = getBalance(root);

        if (balance > 1 && getBalance(root->left) >= 0)
            return rightRotate(root);

        if (balance > 1 && getBalance(root->left) < 0) {
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }

        if (balance < -1 && getBalance(root->right) <= 0)
            return leftRotate(root);

        if (balance < -1 && getBalance(root->right) > 0) {
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }

        return root;
    }

    Node* find(Node* node, Key key) const {
        if (!node || node->key == key)
            return node;
        if (key < node->key)
            return find(node->left, key);
        else
            return find(node->right, key);
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(Key key, Value value) {
        root = insert(root, key, value);
    }

    void remove(Key key) {
        root = deleteNode(root, key);
    }

    bool find(Key key, Value& value) const {
        Node* node = find(root, key);
        if (node) {
            value = node->value;
            return true;
        }
        return false;
    }

    class Iterator {
    private:
        std::stack<Node*> stack;

        void pushAll(Node* node) {
            while (node) {
                stack.push(node);
                node = node->left;
            }
        }

    public:
        Iterator(Node* root) {
            pushAll(root);
        }

        bool hasNext() const {
            return !stack.empty();
        }

        void next() {
            Node* node = stack.top();
            stack.pop();
            pushAll(node->right);
        }

        Node* current() const {
            return stack.top();
        }

        Key key() const {
            return stack.top()->key;
        }

        Value value() const {
            return stack.top()->value;
        }
    };

    Iterator begin() const {
        return Iterator(root);
    }
};

template <typename Key, typename Value>
class MyMap {
private:
    AVLTree<Key, Value> tree;

public:
    void insert(Key key, Value value) {
        tree.insert(key, value);
    }

    void erase(Key key) {
        tree.remove(key);
    }

    bool find(Key key, Value& value) const {
        return tree.find(key, value);
    }

    Value& operator[](const Key& key) {
        Value* value = new Value;
        if (!tree.find(key, *value)) {
            tree.insert(key, *value);
        }
        tree.find(key, *value);
        return *value;
    }

    class Iterator {
    private:
        typename AVLTree<Key, Value>::Iterator treeIterator;

    public:
        Iterator(typename AVLTree<Key, Value>::Iterator it) : treeIterator(it) {}

        bool operator!=(const Iterator& other) const {
            return treeIterator.hasNext() != other.treeIterator.hasNext();
        }

        Iterator& operator++() {
            treeIterator.next();
            return *this;
        }

        std::pair<Key, Value> operator*() const {
            return { treeIterator.key(), treeIterator.value() };
        }
    };

    Iterator begin() const {
        return Iterator(tree.begin());
    }

    Iterator end() const {
        return Iterator(typename AVLTree<Key, Value>::Iterator(nullptr));
    }
};